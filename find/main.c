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

#include <glib.h>
#include <glib/gstdio.h>

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>

#include "mu/mu.h"

#include "app/mu-app.h"
#include "app/mu-util.h"
#include "path/mu-path.h"

#include "mu/mu-msg-str.h"
#include "mu/mu-msg-flags.h"

#include "mu-expr.h"
#include "mu-expr-helpers.h"
#include "mu-expr-sql.h"
#include "mu-query-mgr.h"

struct _FindOptions {
	char*       _linkdir; /* must be freed */
 	const char* _format;
	const char* _output;
	const char* _sortfields;

	/* after parsing _sortdir, _sort_ascending will have the value */
	const char* _sortdir; /* 'a'=>ascending, 'd'=>descending */
	gboolean    _sort_ascending;
};


typedef struct _FindOptions FindOptions;

static FindOptions OPTIONS;

static GOptionEntry OPTION_ENTRIES [] = {
	{"output", 'o', 0, G_OPTION_ARG_STRING, &OPTIONS._output, 
	 "desired output: links|text|sql|aggregate or l|t|s|a"},
	{"linkdir", 'l', 0, G_OPTION_ARG_FILENAME, &OPTIONS._linkdir, 
	 "target directory for symlinks (default: ./mu-results)"},
	{"format", 'f', 0, G_OPTION_ARG_FILENAME, &OPTIONS._format, 
	 "output format; see mu-find(1) for details"},
	{"sortfields", 'S', 0, G_OPTION_ARG_STRING, &OPTIONS._sortfields,
	 "fields to sort results by (default 'd'; see mu-find(1))"},
	{"sortdir", 'D', 0, G_OPTION_ARG_STRING, &OPTIONS._sortdir,
	 "sort direction ('a'=ascending (default if sortfields set),"
	 " 'd'=descending)"},
	{NULL}
};


static MuMsgSQLite*
get_rows (MuQueryMgr *mgr, GSList *expr_strs, FindOptions *opt)
{
	GError *err = NULL;
	MuMsgSQLite *row;
	
	row = mu_query_mgr_run (mgr, expr_strs, 
				opt->_sortfields, 
				opt->_sort_ascending,
				&err);
	if (!row) {
		g_printerr ("error: %s\n", err ? err->message: 
			    "couldn't get rows");
		if (err)
			g_error_free (err);
	}
	return row;
}

static gboolean
print_rows (MuQueryMgr *mgr, GSList *expr_strs, FindOptions *opt)
{
	GError *err = NULL;
	MuMsgSQLite *row;
	
	row = get_rows (mgr, expr_strs, opt);
	if (!row)
		return FALSE;
	
	while (mu_msg_sqlite_next (row, &err)) {
		char *rowstr = mu_msg_sqlite_to_string(row, opt->_format);
		g_printf ("%s\n", rowstr);
		g_free (rowstr);
	}
	
	if (err)  {
		g_printerr ("error: %s\n", err->message);
		if (err)
			g_error_free (err);
	}

	mu_msg_sqlite_destroy (row);	
	return err == NULL;
}


static char*
get_output_dir (void)
{
	int i = 0;
	for (i = 0; i != 256; ++i)  {
		char *cand = g_strdup_printf ("mu-found-%02x", i);
		if (access (cand, F_OK) != 0)
			return cand;
		else
			g_free (cand);
	}
	
	g_printerr ("can't find a writable target directory for symlinks\n");
	return NULL;
}


static gboolean
generate_symlinks (MuQueryMgr *mgr, GSList *expr_strs, const char* dir, 
		   FindOptions *opt)
{
	GError *err = NULL;
	MuMsgSQLite *row;
	char* mdir;

	row = get_rows (mgr, expr_strs, opt);
	if (!row)
		return FALSE;
	
	mdir = dir ? g_strdup(dir): get_output_dir ();
	if (!mu_path_make_maildir(mdir, 0700)) {
		g_free (mdir);
		g_warning ("%s: failed to create maildir: %s",
			   __FUNCTION__, strerror(errno));
		mu_msg_sqlite_destroy (row);
		return FALSE;
	}
	
	while (mu_msg_sqlite_next (row, &err)) {
		const char *path = mu_msg_sqlite_get_path (row);
		if (g_access(path, F_OK) != 0) {
			g_printerr ("error: database seems outdated; cannot find "
				    "'%s';\n"
				    "       run 'mu-index' to update your database\n", 
				    path);
			continue;
		}
		/* this might fail, mu_path_symlink_message logs an error
		 * we just continue with the next file */
		mu_path_symlink_message (path, mdir);
	}	

	if (err)  {
		g_printerr ("error: %s\n", err->message);
		g_error_free (err);
	}

	g_free (mdir);	
	mu_msg_sqlite_destroy (row);
	
	return err == NULL;
}

static gboolean
print_sql (MuQueryMgr *mgr, GSList *expr_strs, FindOptions *opt)
{
	char *sql;
	GError *err = NULL;
	
	sql = mu_query_mgr_get_sql (mgr, expr_strs, NULL,
				    opt->_sortfields, 
				    opt->_sort_ascending, &err);
	if (!sql) {
		if (err) {
			g_printerr ("error: %s\n", err->message);
			g_error_free (err);
		}
		return FALSE;
	}
	
	g_print ("%s", sql);
	g_free (sql);
	
	return TRUE;
}


static void
print_ids (uint64_t *nums, size_t num)
{
	int i;
	for (i = 0; i != num && nums[i]; ++i)
		g_print ("%llu ", 
			 (long long unsigned int)nums[i]);
}

static void
print_stats (MuQueryStats *stats)
{
	
	g_print ("query statistics\n================\n");
	g_print ("number of messages              : %d\n", (int)stats->_msg_num); 

	g_print ("newest messages (msgids)        : "); 
	print_ids (stats->_newest_msg_ids, MU_QUERY_STATS_TOP);
	
	g_print ("\noldest messages (msgids)        : "); 
	print_ids (stats->_oldest_msg_ids, MU_QUERY_STATS_TOP);

	g_print ("\nbiggest messages (msgids)       : "); 
	print_ids (stats->_biggest_msg_ids, MU_QUERY_STATS_TOP);

	g_print ("\navg message size (bytes)        : %d\n", 
		 (int)stats->_avg_msg_size);

	g_print ("most popular senders (cids)     : "); 
	print_ids (stats->_most_popular_sender_ids, MU_QUERY_STATS_TOP);

	g_print ("\nmost popular recipients (cids)  : "); 
	print_ids (stats->_most_popular_recipient_ids, MU_QUERY_STATS_TOP);
	
	g_print ("\n");
}


static gboolean
print_statistics (MuQueryMgr *mgr, GSList *expr_strs, FindOptions *opt)
{
	GError *err = NULL;
	MuQueryStats stats;
	
	if (!mu_query_mgr_create_tmpview (mgr, expr_strs, "stats", &err)) {
		g_printerr ("error: %s\n", err ? err->message :
			    "failed to create tmpview");
		if (err)
			g_error_free (err);
		return FALSE;
	}
	
	if (!mu_query_mgr_get_tmpview_stats (mgr, "stats", &stats)) {
		g_printerr ("error: failed to get stats\n");
		return FALSE;
	}

	print_stats (&stats);

	return TRUE;
}


/* determine what output to generate */
static gboolean
generate_output (FindOptions *opt, GSList *expr_strs, const char* linkdir)
{
	MuQueryMgr *mgr;
	gboolean retval;
	GError *err = NULL;

	mgr = mu_query_mgr_new (mu_app_sqlite_path(), mu_app_xapian_path(),
				mu_app_bookmarks_path(), &err);
	if (!mgr) {
		if (err) {
			g_printerr ("error: %s\n", err->message);
			g_error_free (err);
		}
		g_printerr ("error: failed to initialize query; "
			    "maybe run 'mu-index' first?\n");
		return FALSE;
	}	
	
	err = NULL;
	if (!mu_query_mgr_preprocess (mgr, expr_strs, TRUE, &err)) {
		g_printerr ("error: failed to preprocess search expressions: %s\n",
			    err ? err->message: "");
		g_error_free (err);
		return FALSE;
	}
	
	/* no output option set? */
	if (!opt->_output) /* default to printing */
		retval = print_rows (mgr, expr_strs, opt);
	else {
		switch (opt->_output[0]) {
		case 'l':
			retval = generate_symlinks (mgr, expr_strs, linkdir,
						    opt); break;
		case 't':
			retval = print_rows (mgr, expr_strs, opt); break;
		case 's': 
			retval = print_sql (mgr, expr_strs, opt); break; 

		case 'a':
			retval = print_statistics (mgr, expr_strs, opt); break;

		default:
			g_printerr ("error: not a valid output type: '%s'\n", 
				    opt->_output);
			retval = FALSE;
		}
	}
	mu_query_mgr_destroy (mgr);
	return retval;
}


static gboolean
handle_options (int *argcp, char ***argvp)
{
	GError *err;
		
	memset (&OPTIONS, 0, sizeof(FindOptions));
	OPTIONS._format = "d,f,s"; /* default */

	err = NULL;
	if (!mu_conf_handle_options (mu_app_conf(),OPTION_ENTRIES, argcp, argvp, &err)) {
		g_printerr ("option parsing failed: %s\n",
			    (err && err->message) ? err->message : "?" );
		if (err)
			g_error_free (err);

		return FALSE;
	}

	/* descending on date is the default ... */
	if (!OPTIONS._sortfields && !OPTIONS._sortdir) {
		OPTIONS._sortfields = "d";
		OPTIONS._sort_ascending = FALSE;
	} else if (!OPTIONS._sortdir) 
		OPTIONS._sort_ascending = TRUE;
	else {
		switch (tolower(OPTIONS._sortdir[0])) {
		case 'a': OPTIONS._sort_ascending = TRUE; break;
		case 'd': OPTIONS._sort_ascending = FALSE; break;
		default:
			g_printerr ("error: not a valid sort direction\n");
			return FALSE;
		}
	}
				
	if (!mu_expr_helpers_sortfields_valid(OPTIONS._sortfields)) {
		g_printerr ("error: invalid sortfield(s)\n");
		return FALSE;
	}

	return TRUE;
}



int
main (int argc, char *argv[])
{
	int retval = 1;
	char *linkdir = NULL;

	do {
		GSList *args;

		if (!mu_app_init (&argc, &argv, "mu-find")) {
			g_printerr ("error: failed to init mu\n");
			return 1;
		}	
		
		if (!handle_options (&argc, &argv)) {
			g_printerr ("error: invalid options\n");
			break;
		}	

		linkdir = mu_util_homedir_expand (OPTIONS._linkdir); /* expand '~/' */
		
		if (argc < 2) {
			g_printerr ("error: one or more search expressions "
				    "expected\n");
			break;
		}
		
		args = mu_expr_helpers_strlist_from_args (argc-1, &argv[1]);
		retval = generate_output (&OPTIONS, args, linkdir) ? 0: 1;
		mu_expr_helpers_strlist_free (args);
		
	} while (0);
	
	g_free (linkdir);
	mu_app_uninit ();
		
	return retval;
}
