/*
 * Copyright (c) 2009-2018, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of LIXA.
 *
 * LIXA is free software: you can redistribute this file and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * LIXA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LIXA.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "config.h"



/* system includes */
#ifdef HAVE_GLIB_H
# include <glib.h>
#endif
/* LIXA includes */
#include "lixa_errors.h"
#include "lixa_trace.h"
#include "client_status.h"
/* XTA includes */
#include "xta_transaction_manager.h"



/* set module trace flag */
#ifdef LIXA_TRACE_MODULE
# undef LIXA_TRACE_MODULE
#endif /* LIXA_TRACE_MODULE */
#define LIXA_TRACE_MODULE   LIXA_TRACE_MOD_XTA



xta_transaction_manager_t *xta_transaction_manager_new(void)
{
    enum Exception { G_TRY_MALLOC_ERROR
                     , CLIENT_CONFIG_ERROR
                     , NONE } excp;
    int ret_cod = LIXA_RC_INTERNAL_ERROR;
    xta_transaction_manager_t *this = NULL;
    
    LIXA_TRACE(("xta_transaction_manager_new\n"));
    TRY {
        /* allocate the object */
        if (NULL == (this = (xta_transaction_manager_t *)
                     g_try_malloc0(sizeof(xta_transaction_manager_t))))
            THROW(G_TRY_MALLOC_ERROR);
        /* initialize the mutex */
        g_mutex_init(&this->mutex);
        /* configure the global config for LIXA client (if necessary) */
        if (LIXA_RC_OK != (ret_cod = client_config(&global_ccc, TRUE)))
            THROW(CLIENT_CONFIG_ERROR);
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_TRY_MALLOC_ERROR:
                ret_cod = LIXA_RC_G_TRY_MALLOC_ERROR;
                break;
            case CLIENT_CONFIG_ERROR:
                break;
            case NONE:
                ret_cod = LIXA_RC_OK;
                break;
            default:
                ret_cod = LIXA_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    LIXA_TRACE(("xta_transaction_manager_new/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return this;
}



void xta_transaction_manager_delete(xta_transaction_manager_t *this)
{
    enum Exception { CLIENT_UNCONFIG_ERROR
                     , NONE } excp;
    int ret_cod = LIXA_RC_INTERNAL_ERROR;
    
    LIXA_TRACE(("xta_transaction_manager_delete\n"));
    TRY {
        /* configure the global config for LIXA client (if necessary) */
        if (LIXA_RC_OK != (ret_cod = client_unconfig(&global_ccc, TRUE)))
            THROW(CLIENT_UNCONFIG_ERROR);
        /* destroy transaction objects if any */
        if (NULL != this->transactions) {
            g_hash_table_destroy(this->transactions);
            this->transactions = NULL;
        } /* if (NULL != this->transactions) */
        
        /* clear the synchronization mutex */
        g_mutex_clear(&this->mutex);
        
        /* destroy the object itself */
        g_free(this);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case CLIENT_UNCONFIG_ERROR:
                break;
            case NONE:
                ret_cod = LIXA_RC_OK;
                break;
            default:
                ret_cod = LIXA_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    LIXA_TRACE(("xta_transaction_manager_delete/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return;
}



xta_transaction_manager_config_t *
xta_transaction_manager_get_config(void) {
    return &global_ccc;
}



xta_transaction_t *
xta_transaction_manager_create_transaction(xta_transaction_manager_t *this)
{
    enum Exception { NULL_OBJECT1
                     , G_HASH_TABLE_NEW_ERROR
                     , NULL_OBJECT2
                     , INTERNAL_ERROR
                     , NONE } excp;
    int ret_cod = LIXA_RC_INTERNAL_ERROR;
    xta_transaction_t *tx = NULL;
    
    LIXA_TRACE(("xta_transaction_manager_create_transaction\n"));
    TRY {
        GThread *self = g_thread_self();
        
        /* check object reference */
        if (NULL == this)
            THROW(NULL_OBJECT1);
        
        LIXA_TRACE(("xta_transaction_manager_create_transaction: "
                    "locking mutex...\n"));
        g_mutex_lock(&this->mutex);

        /* check if the hash table as been already created */
        if (NULL == this->transactions) {
            if (NULL == (this->transactions = g_hash_table_new_full(
                             NULL, NULL, NULL,
                             (GDestroyNotify)xta_transaction_delete)))
                THROW(G_HASH_TABLE_NEW_ERROR);
        } /* if (NULL == this->transactions) */
        
        /* look for transaction in hash table */
        if (NULL == (tx = g_hash_table_lookup(this->transactions, self))) {
            LIXA_TRACE(("xta_transaction_manager_create_transaction: there is "
                        "not a started transactions for this thread, "
                        "starting a new one...\n"));
            /* allocate a new transaction object */
            if (NULL == (tx = xta_transaction_new()))
                THROW(NULL_OBJECT2);
            
            /* insert the transaction object in the hash map */
#if GLIB_MAJOR_VERSION == 2
# if GLIB_MINOR_VERSION >= 40
            if (!g_hash_table_insert(this->transactions, self, tx)) {
                THROW(INTERNAL_ERROR);
            }
# else
            g_hash_table_insert(this->transactions, self, tx);
# endif
#endif
        } /* if (NULL == (tx = g_hash_table_lookup */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NULL_OBJECT1:
                ret_cod = LIXA_RC_NULL_OBJECT;
                break;
            case G_HASH_TABLE_NEW_ERROR:
                ret_cod = LIXA_RC_G_HASH_TABLE_NEW_ERROR;
                break;
            case NULL_OBJECT2:
                ret_cod = LIXA_RC_NULL_OBJECT;
                break;
            case INTERNAL_ERROR:
                ret_cod = LIXA_RC_INTERNAL_ERROR;
                break;
            case NONE:
                ret_cod = LIXA_RC_OK;
                break;
            default:
                ret_cod = LIXA_RC_INTERNAL_ERROR;
        } /* switch (excp) */
        /* recoverying memory in the event of error */
        if (NULL_OBJECT2 < excp && NONE > excp)
            xta_transaction_delete(tx);
        /* clear mutex */
        if (NULL_OBJECT1 < excp) {
            g_mutex_unlock(&this->mutex);
            LIXA_TRACE(("xta_transaction_manager_create_transaction: "
                        "mutex unlocked\n"));
        }
    } /* TRY-CATCH */
    LIXA_TRACE(("xta_transaction_manager_create_transaction/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return tx;
}
