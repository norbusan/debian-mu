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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /*HAVE_CONFIG_H*/

#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glib/gstdio.h>

#include "mu-msg-sqlite.h"
#include "mu-msg-sqlite-priv.h"
#include "mu-expr-helpers.h"
#include "mu-expr-bookmarks.h"

#include "mu-expr.h"
#include "mu-expr-sql.h"

#include "mu-query-mgr.h"
#include "mu-query-sqlite.h"

#ifdef MU_HAVE_XAPIAN
#include "mu-query-xapian.h"
struct _MuQueryMgr {
	MuQuerySQLite     *_qsqlite;
	MuQueryXapian     *_qxapian;
	MuExprBookmarks   *_bookmarks;
};
#else 
struct _MuQueryMgr {
	MuQuerySQLite     *_qsqlite;
	MuExprBookmarks   *_bookmarks;
};
#endif /*!MU_HAVE_XAPIAN */

MuQueryMgr *
mu_query_mgr_new (const char *spath, const char* xpath, const char* bmpath,
		  GError **err)
{
	MuQueryMgr *mgr;	

	g_return_val_if_fail (spath, NULL);
#ifdef MU_HAVE_XAPIAN
	g_return_val_if_fail (xpath, NULL);
#endif /*MU_HAVE_XAPIAN*/

	mgr = (MuQueryMgr*)malloc(sizeof(MuQueryMgr));
	if (!mgr)
		return NULL;
	memset (mgr, 0, sizeof(MuQueryMgr));

	/* bookmarks*/
	if (bmpath && g_access(bmpath, R_OK) == 0) { /* not required */
		mgr->_bookmarks = mu_expr_bookmarks_new (bmpath, err);
		if (!mgr->_bookmarks) {
			mu_query_mgr_destroy (mgr);
			return NULL;
		}
	}

	/* sqlite */
	mgr->_qsqlite = mu_query_sqlite_new (spath, err);
	if (!mgr->_qsqlite) {
		mu_query_mgr_destroy (mgr);
		return NULL;
	}
	
#ifdef MU_HAVE_XAPIAN
	/* xapian */
	mgr->_qxapian = mu_query_xapian_new (xpath, err);
	if (!mgr->_qxapian) {
		g_warning ("%s: failed to open xapian query for '%s'",
			   __FUNCTION__, xpath);
		mu_query_mgr_destroy (mgr);
		return NULL;
	}
#endif /*MU_HAVE_XAPIAN*/	
	
	return mgr;
}

void
mu_query_mgr_destroy (MuQueryMgr *mgr)
{
	if (mgr) {
		mu_expr_bookmarks_destroy (mgr->_bookmarks);
		mu_query_sqlite_destroy (mgr->_qsqlite);
#ifdef MU_HAVE_XAPIAN
		mu_query_xapian_destroy (mgr->_qxapian);
#endif /*MU_HAVE_XAPIAN */
		free (mgr);
	}	
}


static GHashTable*
get_xapian_hash (MuQueryMgr *mgr, MuExprList *exprs, GError **err)
{
	GHashTable *xapian_hash;
	MuExprList *xapian_exprs = NULL, *cursor;
	
	cursor = exprs;
	while (cursor) {
		MuExpr *expr = cursor->data;
		if (mu_expr_type(expr) == MU_EXPR_TYPE_NONE) {
			g_set_error (err, 0, 0, "invalid xapian expression type");
			mu_expr_list_destroy (xapian_exprs, TRUE);
			return NULL;
		} else if (mu_expr_type(expr) == MU_EXPR_TYPE_XAPIAN)
			xapian_exprs = g_slist_append (xapian_exprs, expr);
		cursor = cursor->next;
	}
	
#ifdef MU_HAVE_XAPIAN
	/* now, get the list of matching docs from xapian;
	 *  we'll use there id's in the sqlite statement below */
	xapian_hash = mu_query_xapian_run_list (mgr->_qxapian, xapian_exprs, 
						50, 1000);
#else 
	xapian_hash = NULL;	
#endif /*MU_HAVE_XAPIAN*/
	
	mu_expr_list_destroy (xapian_exprs, TRUE);
	
	return xapian_hash;
}


static MuExprList*
get_sqlite_exprs (MuQueryMgr *mgr, MuExprList *exprs, GError **err)
{
	MuExprList *sqlite_exprs, *cursor;
		
	/* we split the expressions in ones to be handled
	 * by sqlite and xapian */
	sqlite_exprs = NULL;
	cursor = exprs;
	while (cursor) {
		MuExpr *expr = cursor->data;
		if (mu_expr_type(expr) == MU_EXPR_TYPE_NONE) {
			g_set_error (err, 0, 0, "invalid sqlite expression type");
			mu_expr_list_destroy (sqlite_exprs, TRUE);
			return NULL;
		} else if (mu_expr_type(expr) != MU_EXPR_TYPE_XAPIAN)
			sqlite_exprs = g_slist_append (sqlite_exprs, expr);
		
		cursor = cursor->next;
	}

	return sqlite_exprs;
}


char*
mu_query_mgr_get_sql  (MuQueryMgr *mgr, GSList *expr_strs,
		       const char* tmpview,
		       const char* sortfields, 
		       gboolean ascending, GError **err)
{
	MuExprList *exprs, *sqlite_exprs;
	GHashTable *xapian_hash;
	char *sql;
	
	g_return_val_if_fail (mgr, NULL);
	g_return_val_if_fail (expr_strs, NULL);
		
	exprs = mu_expr_list_new (expr_strs, err);
	if (!exprs)
		return NULL;

	xapian_hash = get_xapian_hash (mgr, exprs, err);
	
	sqlite_exprs = get_sqlite_exprs (mgr, exprs, err);
	if (err&&*err) 
		return NULL;

	if (tmpview)
		sql =  mu_expr_sql_generate_sql_view_create (sqlite_exprs,
						     xapian_hash,
						     tmpview);
	else
		sql = mu_expr_sql_generate (sqlite_exprs, xapian_hash,
					    sortfields, ascending);

	mu_expr_list_destroy (exprs, FALSE);

	if (xapian_hash)
		g_hash_table_destroy (xapian_hash);

	return sql;
}



MuMsgSQLite*
mu_query_mgr_run (MuQueryMgr *mgr, GSList *expr_strs,
		  const char* sortfields, gboolean ascending,
		  GError **err)
{
	char *sql;
	MuMsgSQLite *row;

	g_return_val_if_fail (mgr, NULL);
	g_return_val_if_fail (expr_strs, NULL);
		
	sql = mu_query_mgr_get_sql (mgr, expr_strs, NULL,
				    sortfields, ascending,
				    err);
	if (!sql) {
		g_warning ("%s: failed to generate sql", __FUNCTION__);
		return NULL;
	}
	
	row =  mu_query_sqlite_run (mgr->_qsqlite, sql, err); 
	g_free (sql);
	
	return row;
}

gboolean
mu_query_mgr_create_tmpview (MuQueryMgr *mgr, GSList *expr_strs,
			     const char* tmpview,
			     GError **err)
{
	char* sql;
	gboolean result;

	g_return_val_if_fail (tmpview, FALSE);
	
	sql = mu_query_mgr_get_sql (mgr, expr_strs, tmpview,
				    NULL, FALSE, err);
	if (!sql) {
		g_warning ("%s: failed to generate sql", __FUNCTION__);
		return FALSE;
	}
	
	/* first we run the 'create view .... as .... ' query */
	result = mu_query_sqlite_exec (mgr->_qsqlite, sql, err);
	g_free (sql);
	if (!result)
		return FALSE;

	return TRUE;
}


MuMsgSQLite*
mu_query_mgr_read_tmpview (MuQueryMgr *mgr,
			   const char* tmpview, const char* sortfields, 
			   gboolean ascending, GError **err)
{
	char* sql;
	MuMsgSQLite *row;

	g_return_val_if_fail (tmpview, NULL);

	/* now, we run the 'select * from view ... query */
	sql = mu_expr_sql_generate_sql_view_read (tmpview, sortfields,
						  ascending);
	if (!sql) {
		g_warning ("%s: failed to generate sql", __FUNCTION__);
		return NULL;
	}

	row =  mu_query_sqlite_run (mgr->_qsqlite, sql, err); 
	g_free (sql);
	
	return row;
}



gboolean
mu_query_mgr_preprocess (MuQueryMgr *mgr, GSList *expr_strs, gboolean dofree,
			 GError **err)
{
	const char* colon;
	GSList *cursor;
	
	g_return_val_if_fail (mgr, FALSE);
	g_return_val_if_fail (expr_strs, FALSE);
	
	cursor = expr_strs;
	while (cursor) {
		char *expr = cursor->data;
		
		/* bare words become xapian queries */
		if (!(colon = strchr (expr, ':'))) {
#ifdef MU_HAVE_XAPIAN
			cursor->data = g_strdup_printf ("x:%s", expr);
#else
			cursor->data = g_strdup_printf ("s:%s", expr);
#endif /*!MU_HAVE_XAPIAN*/
			if (dofree)
				g_free(expr);

			expr = cursor->data;
		}
		
		/* insert bookmarks here */
		if (expr[0] == 'B' && expr[1] == ':') {
			char* bookmark = NULL;
			if (mgr->_bookmarks)
				bookmark = mu_expr_bookmarks_resolve (mgr->_bookmarks, 
								      &expr[2]);
			if (!bookmark) {
				g_set_error (err, 0,
					     MU_EXPR_ERROR_BOOKMARK_NOT_FOUND,
					     "could not find bookmark for '%s'",
					     &expr[2]);
				return FALSE;
			}  else {
				cursor->data = bookmark;
				if (dofree)
					g_free(expr);
			}
		}
		cursor = cursor->next;
	}
	
	return TRUE;
}



gboolean
mu_query_mgr_get_tmpview_stats (MuQueryMgr *mgr, const char* tmpview,
				MuQueryStats *stats)
{
	g_return_val_if_fail (mgr, FALSE);
	g_return_val_if_fail (tmpview, FALSE);
	g_return_val_if_fail (stats, FALSE);

	return mu_query_sqlite_get_stats (mgr->_qsqlite, tmpview, stats);
}
