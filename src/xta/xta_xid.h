/*
 * Copyright (c) 2009-2017, Christian Ferrari <tiian@users.sourceforge.net>
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
#ifndef XTA_XID_H
# define XTA_XID_H



/* LIXA includes */
#include <xa.h>
#include "lixa_trace.h"



/* save old LIXA_TRACE_MODULE and set a new value */
#ifdef LIXA_TRACE_MODULE
# define LIXA_TRACE_MODULE_SAVE LIXA_TRACE_MODULE
# undef LIXA_TRACE_MODULE
#else
# undef LIXA_TRACE_MODULE_SAVE
#endif /* LIXA_TRACE_MODULE */
#define LIXA_TRACE_MODULE      LIXA_TRACE_MOD_XTA



/**
 * XTA Transaction data type
 */
typedef struct {
    /**
     * Transaction ID using the standard XA format
     */
    XID          xa_xid;
} xta_xid_t;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



    /**
     * Create a new Transaction Identifier object and generate a new unique XID
     * @param[in] branch_qualifier that must be assigned to the XID
     *            (in ASCII HEX format)
     * @return a new transaction identifier object or NULL in the event of an
     *         error occurred
     */
    xta_xid_t *xta_xid_new(const char *branch_qualifier);



    /**
     * Create a new Transaction Identifier object and set XID as passed by the
     * caller
     * @param[in] xid_string a serialized XID, see @ref xta_xid_to_string
     * @return a new transaction identifier object or NULL in the event of an
     *         error occurred
     */
    xta_xid_t *xta_xid_new_from_string(const char *xid_string);

    

    /**
     * Delete a Transaction Identifier object
     * @param[in] this : xid object to delete
     */
    void xta_xid_delete(xta_xid_t *this);



    /**
     * Retrieve the transaction ID in the XA standard format
     * @param[in] this : xid object
     * @return a reference to the XA representation of the transaction id
     */
    const XID *xta_xid_get_xa_xid(xta_xid_t *this);



    /**
     * Convert the transaction ID to an ASCII string
     * @param[in] this xid object
     * @return a string that must be released using free() function by the
     *         caller
     */
    char *xta_xid_to_string(const xta_xid_t *this);



    /**
     * Reset a Transaction identifier object
     * @param[in,out] this xid object to delete
     */
    void xta_xid_reset(xta_xid_t *this);


    
#ifdef __cplusplus
}
#endif /* __cplusplus */



/* restore old value of LIXA_TRACE_MODULE */
#ifdef LIXA_TRACE_MODULE_SAVE
# undef LIXA_TRACE_MODULE
# define LIXA_TRACE_MODULE LIXA_TRACE_MODULE_SAVE
# undef LIXA_TRACE_MODULE_SAVE
#endif /* LIXA_TRACE_MODULE_SAVE */



#endif /* XTA_XID_H */
