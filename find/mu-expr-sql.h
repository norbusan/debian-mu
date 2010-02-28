/* 
** Copyright (C) 2008 Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>
**
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 3, or (at your option) any
** later version.
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

#ifndef __MU_EXPR_SQL_H__
#define __MU_EXPR_SQL_H__

#include <glib.h>
#include "mu-expr.h"

/** 
 * generate the corresponding SQL statements for a list of Mu search
 * expressions. See the mu-find(1) manpage for details on the syntax.
 * the function attempts to produce human-readable SQL, that conform
 * with (a)SQLite, and (b)the database scheme as specified in
 * index/storage.sql
 * 
 * @param exprs a list of search expressions.
 * @param xapian_hash hash with xapian values; see mu_query_xapian_run
 * @param sortfields the fields to sort the results by in the char-per-field
 * notation
 * @param ascending sort ascending if TRUE, decending otherwise
 * 
 * @return a newly allocated string; free with g_free when it is no
 * longer needed.
 */
char* mu_expr_sql_generate (GSList *exprs, GHashTable *xapian_hash,
			    const char* sortfields, gboolean ascending);



/** 
 * generate the corresponding SQL statements for a list of Mu search
 * expressions similar to mu_expr_sql_generate; difference is that the sql
 * sql this function generates will create a temporary view with the results
 * 
 * @param exprs a list of search expressions.
 * @param xapian_hash hash with xapian values; see mu_query_xapian_run
 * @param tmpview temp view name for storing the result of the query, or NULL
 * 
 * @return a newly allocated string; free with g_free when it is no
 * longer needed.
 */
char* mu_expr_sql_generate_sql_view_create (MuExprList *exprs, 
					    GHashTable *xapian_hash,
					    const char* tmpview);

/** 
 * generate the SQL to read the temporary view created with
 * mu_expr_sql_generate_sql_view_create
 * 
 * @param tmpview temp view name to query
 * @param sortfields the fields to sort the results by in the char-per-field
 * notation
 * @param ascending sort ascending if TRUE, decending otherwise
 * 
 * @return a newly allocated string; free with g_free when it is no
 * longer needed.
 */
char* mu_expr_sql_generate_sql_view_read (const char* tmpview, 
					  const char* sortfields,
					  gboolean ascending);

#endif /*__MU_EXPR_SQL_H__*/
