/* 
** Copyright (C) 2008 Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**  
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**  
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software Foundation,
** Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  
**  
*/

#ifndef __MU_QUERY_H__
#define __MU_QUERY_H__

#include <glib.h>
#include "mu-msg-sqlite.h"
#include "mu-query-sqlite.h"
#include "mu-query-xapian.h"

G_BEGIN_DECLS

struct _MuQueryMgr;
typedef struct _MuQueryMgr MuQueryMgr;


/** 
 * create a new MuQueryMgr instance. 
 * 
 * @param spath path to the sqlite db to search
 * @param cpath path to the xapian db to search
 * @param path to the bookmarksfile, or NULL
 * @param err receives error information
 * 
 * @return a new MuQueryMgr instance, or NULL in case of error.
 * when done with the instance, use mu_query_mgr_destroy
 */
MuQueryMgr *mu_query_mgr_new     (const char *spath, 
				  const char* xpath,
				  const char* bmpath,
				  GError **err);


/** 
 * destroy the MuQueryMgr instance
 * 
 * @param mgr a MuQueryMgr instance, or NULL
 */
void        mu_query_mgr_destroy (MuQueryMgr *mgr);


/** 
 * generate the sql query for a list of search expression strings 
 * 
 * @param mgr a valid MuQueryMgr instance
 * @param exprs_strs a list of search expr strings (eg. "s:foo", "f:bar")
 * @param tmptable temp table name for storing the result of the query, or NULL 
 * @param sortfields one-letter descriptions of fields to sort on
 * @param whether to sort ascending (TRUE), or descending (FALSE)
 * @param err errorptr to receive error information
 * 
 * @return a newly allocated string with the search query, or NULL in
 * case of error. free the string with g_free when done with it.
 */
char*       mu_query_mgr_get_sql (MuQueryMgr *mgr, GSList *expr_strs,
				  const char* tmptable,
				  const char* sortfields, gboolean ascending,
				  GError **err);

/** 
 * run a search query for a list of search expression strings 
 * 
 * @param mgr a valid MuQueryMgr instance
 * @param exprs_strs a list of search expr strings (eg. "s:foo", "f:bar")
 * @param tmptable temp table name for storing the result of the query, or NULL 
 * @param sortfields one-letter descriptions of fields to sort on
 * @param ascending whether to sort ascending (TRUE), or descending (FALSE)
 * @param err errorptr to receive error information
 * 
 * @return a new MuMsgSQLite object with the search query, or NULL in
 * case of error. to get the first result row, do mu_msg_sqlite_next
 * and get subsequent rows with the next call to mu_msg_sqlite_next
 * until that call returns FALSE
 */
MuMsgSQLite*   mu_query_mgr_run     (MuQueryMgr *mgr, GSList *expr_strs,
				     const char* sortfields, gboolean ascending,
				     GError **err);


/** 
 * similar to mu_query_mgr_run, but instead puts it results in a temporary
 * view
 * 
 * @param mgr a valid MuQueryMgr instance
 * @param exprs_strs a list of search expr strings (eg. "s:foo", "f:bar")
 * @param tmpview temp view name for storing the result of the query, or NULL 
 * @param err errorptr to receive error information
 * 
 * @return TRUE if this succeeded, FALSE otherwise
 */
gboolean       mu_query_mgr_create_tmpview (MuQueryMgr *mgr, GSList *expr_strs,
					    const char* tmpview,
					    GError **err);


/** 
 * get search results from an existing view; use in combination with
 * mu_query_mgr_create_tmpview
 * 
 * @param mgr a valid MuQueryMgr instance
 * @param tmpview temp view name for storing the result of the query, or NULL 
 * @param sortfields one-letter descriptions of fields to sort on
 * @param ascending whether to sort ascending (TRUE), or descending (FALSE)
 * @param err errorptr to receive error information
 * 
 * @return TRUE if this succeeded, FALSE otherwise
 */
MuMsgSQLite*    mu_query_mgr_read_tmpview (MuQueryMgr *mgr,
					   const char* tmpview, 
					   const char* sortfields, 
					   gboolean ascending, GError **err);


/** 
 * get statistics about a table with messages; usually, this table will
 * be a temp view/table created as a subset of the message table. 
 *
 * @param query MuQuerySQLite instance
 * @param tmpview the name of the table/view to get stats for
 * @param stats ptr to a stats struct that will receive the result
 * 
 * @return TRUE if succeeded, FALSE otherwise
 */
gboolean        mu_query_mgr_get_tmpview_stats (MuQueryMgr *mgr, 
						const char* tmpview,
						MuQueryStats *stats);

/** 
 * preprocess the the list of (pseudo-)MuExpressions; ie. change barewords into
 * xapian expressions, and resolve bookmarks ('B:mybookmark')
 * 
 * @param mgr a valid MuQueryMgr instance
 * @param expr_strs a list of expressions strings
 * @param dofree should we free the strings when we replace them?
 * @param err errorptr to receive error information
 * 
 * @return TRUE if preprocessing succeeded, FALSE otherwise
 */
gboolean   mu_query_mgr_preprocess (MuQueryMgr *mgr, GSList *expr_strs, 
				    gboolean dofree, GError **err);

G_END_DECLS

#endif /*__MU_QUERY_H__*/
