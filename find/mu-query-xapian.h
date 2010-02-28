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

#ifndef __MU_QUERY_XAPIAN_H__
#define __MU_QUERY_XAPIAN_H__

#include <glib.h>
#include "mu-expr.h"

G_BEGIN_DECLS

/*
 * MuQueryXapian. Usually, it is not used directly, but through
 * MuQueryMgr
 */
struct _MuQueryXapian;
typedef struct _MuQueryXapian MuQueryXapian;

/** 
 * create a new MuQueryXapian instance. 
 * 
 * @param path path to the xapian db to search
 * @param err receives error information (if there is any)
 *
 * @return a new MuQuerySqlite instance, or NULL in case of error.
 * when the instance is no longer needed, use mu_query_xapian_destroy
 * to free it
 */
MuQueryXapian  *mu_query_xapian_new      (const char* path, GError **err);

/** 
 * destroy the MuQueryXapian instance
 * 
 * @param mgr a MuQueryXapian instance, or NULL
 */
void            mu_query_xapian_destroy  (MuQueryXapian *query);

/*
 * struct returned as part of a GHashTable in mu_query_xapian_run
 */
struct _MuContentMatch {
	size_t         score; /* [0..100] % */
};
typedef struct _MuContentMatch MuContentMatch;

/** 
 * run a Xapian query; for the syntax, please refer to the mu-find
 * manpage, or http://xapian.org/docs/queryparser.html
 * 
 * @param query a valid MuQueryXapian instance
 * @param expr the search expression
 * @minscore the minimum score [0..100] to be considered a match
 * @maxnum the maximum number of documents to return
 *
 * @return a GHashTable of [id => MuContentMatch], with matching messages,
 * where id is the string representation of the id in the SQLite database
 * (ie. "1" or "2", or ..) and MuContentMatch has information about the
 * score [0..100] of the match
 */
GHashTable*     mu_query_xapian_run      (MuQueryXapian *query, 
					  const char* expr, 
					  size_t minscore, 
					  size_t maxnum);


/** 
 * run a Xapian query for a list of strings; the strings will be 
 * concatenated with " AND " between them.  * for the syntax of the
 * individual strings, please refer to the mu-find
 * manpage, or http://xapian.org/docs/queryparser.html
 * 
 * @param query a valid MuQueryXapian instance
 * @param expr the search expression
 * @minscore the minimum score [0..100] to be considered a match
 * @maxnum the maximum number of documents to return
 *
 * @return a GHashTable of [id => MuContentMatch], with matching messages,
 * where id is the string representation of the id in the SQLite database
 * (ie. "1" or "2", or ..) and MuContentMatch has information about the
 * score [0..100] of the match
 */
GHashTable*     mu_query_xapian_run_list (MuQueryXapian *query,
					  MuExprList* exprs,
					  size_t minscore,
					  size_t maxnum);

G_END_DECLS

#endif /*__MU_QUERY_XAPIAN_H__*/
