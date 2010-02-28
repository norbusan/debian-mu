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

#include <stdlib.h>
#include <xapian.h>
#include <glib/gstdio.h>
#include <string.h>
#include <string>

#include "index/mu-storage-xapian.h" /* for MU_SQLITE_MESSAGE_ID */

#include "mu-expr-val.h"
#include "mu-query-mgr.h" /* for MuContentMatch */
#include "mu-query-xapian.h"


struct _MuQueryXapian {
	Xapian::Database *_db;
};

MuQueryXapian*
mu_query_xapian_new (const char* path, GError **err)
{
	MuQueryXapian *query;
	
	g_return_val_if_fail (path, NULL);
	if (!g_file_test (path, G_FILE_TEST_IS_DIR) ||
	    g_access(path, R_OK) != 0) { 
		g_set_error (err, 0, MU_EXPR_ERROR_XAPIAN_INIT,
			     "'%s' is not a readable xapian dir", path);
		return NULL;
	}

	query = (MuQueryXapian*)malloc(sizeof(MuQueryXapian));
	if (!query)
		return NULL;

	try {
		query->_db = new Xapian::Database(path);
		return query;
	
	} catch (const Xapian::Error &ex) {
		delete query->_db;
		g_set_error (err, 0, MU_EXPR_ERROR_XAPIAN_INIT,
			     "%s: caught xapian exception '%s' (%s)", 
			     __FUNCTION__, ex.get_msg().c_str(), 
			     ex.get_error_string());
	} catch (...) {
		delete query->_db;
		g_set_error (err, 0, MU_EXPR_ERROR_XAPIAN_INIT,
			     "%s: caught exception", __FUNCTION__);
	}
	
	free (query);
	return NULL;
}


void
mu_query_xapian_destroy (MuQueryXapian *query)
{
	if (!query)
		return;

	try {
		delete query->_db;
		free (query);

	} catch (const Xapian::Error &err) {
		free (query);
		g_warning ("%s: caught xapian exception '%s' (%s)", 
			   __FUNCTION__, err.get_msg().c_str(), 
			   err.get_error_string());
	} catch (...) {
		free (query);
		g_warning ("%s: caught exception", __FUNCTION__);
	}
}


static void
mu_content_match_destroy (MuContentMatch *match)
{
	if (!match)
		return;

	g_slice_free (MuContentMatch, match);
}


static MuContentMatch*
mu_content_match_new (void)
{
	return g_slice_new (MuContentMatch);
}


GHashTable*
mu_query_xapian_run  (MuQueryXapian *query, const char* searchexpr, 
		      size_t minscore, size_t maxnum)
{
	GSList *lst;
	size_t count;
	GHashTable *hash = NULL;
	
	g_return_val_if_fail (query, NULL);
	g_return_val_if_fail (0 <= minscore && minscore <= 100, NULL);
	
	if (!query)
		return NULL; /* not strictly an error */
	
	try {
		Xapian::MSet matches;
		Xapian::Enquire enq(*query->_db);
		Xapian::QueryParser qparser;
		qparser.set_database(*query->_db);
		Xapian::Query q (qparser.parse_query (searchexpr));
		enq.set_query  (q);
		enq.set_cutoff (minscore);
		matches = enq.get_mset(0, maxnum);

		hash = g_hash_table_new_full(g_str_hash,
					     g_str_equal,
					     g_free,
					     (GDestroyNotify)mu_content_match_destroy);

		for (Xapian::MSet::const_iterator ci = matches.begin();
		     ci != matches.end(); ++ci) {
			const std::string 
				id (ci.get_document().get_value
				    (MU_XAPIAN_SQLITE_MESSAGE_ID));
			if (!id.empty()) {
				MuContentMatch *match = 
					mu_content_match_new ();
				match->score = (size_t)ci.get_percent();
				g_hash_table_insert (hash, g_strdup(id.c_str()),
						     match);
			}
		}		

		return hash;

	} catch (const Xapian::Error &err) {
		g_warning ("%s: caught xapian exception '%s' (%s)", 
			   __FUNCTION__, err.get_msg().c_str(), 
			   err.get_error_string());
	} catch (...) {
		g_warning ("%s: caught exception", __FUNCTION__);
	}

	if (hash)
		g_hash_table_destroy (hash);

	return NULL;

}



GHashTable*
mu_query_xapian_run_list (MuQueryXapian *query, MuExprList* exprs, 
			  size_t minscore, size_t maxnum)
{
	std::string str;

	g_return_val_if_fail (query, NULL);
	if (!exprs)
		return NULL;
	
	str.reserve(255); /* just a guess */
	while (exprs) {
		const char* data;
		data = mu_expr_val_str(mu_expr_get_val((MuExpr*)exprs->data));
		if (data) {
			if (!str.empty())
				str += " AND ";
			str += data;
		}
		exprs = g_slist_next (exprs);
	}

	return mu_query_xapian_run  (query, str.c_str(), minscore, maxnum);
}
