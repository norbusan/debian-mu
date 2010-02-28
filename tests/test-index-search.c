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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /*HAVE_CONFIG_H*/

#include <stdio.h>
#include <glib.h>
#include <glib-object.h>

#include <glib/gstdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "index/mu-index.h"
#include "find/mu-expr-helpers.h"
#include "msg/mu-msg-gmime.h"
#include "find/mu-query-mgr.h"

struct _TestData {
	char* spath;
	char* xpath;
	char* bmpath;
	MuQueryMgr *mgr;
};
typedef struct _TestData TestData;


static char*
random_tmpdir (void)
{
	return g_strdup_printf ("%s%c%s.%x", g_get_tmp_dir(), 
				G_DIR_SEPARATOR, "mu-test-index-search", 
				(int)random()*getpid()*(int)time(NULL));	
}

static char *
random_view (void)
{
	return g_strdup_printf ("view%x",(int)random()*getpid()*(int)time(NULL));
}


static MuMsgSQLite*
query_mgr_run_str (MuQueryMgr *mgr, const gchar *str, 
		   const char* sortfields, gboolean ascending,
		   GError **err)
{
	GSList *strlist;
	MuMsgSQLite *row;

	g_return_val_if_fail (mgr, NULL);
	g_return_val_if_fail (str, NULL);

	strlist = mu_expr_helpers_strlist_from_str (str,FALSE);
	row = mu_query_mgr_run (mgr, strlist, sortfields, ascending, err);

	mu_expr_helpers_strlist_free (strlist);

	return row;
}


static void
query_mgr_stats_str (MuQueryMgr *mgr, const gchar *str, MuQueryStats *stats)
{
	GSList *strlist;
	char *tmpview;

	g_return_if_fail (mgr);
	g_return_if_fail (str);

	strlist = mu_expr_helpers_strlist_from_str (str,FALSE);
	tmpview = random_view();

	g_assert (mu_query_mgr_create_tmpview(mgr, strlist, tmpview, NULL));
	g_assert (mu_query_mgr_get_tmpview_stats (mgr, tmpview, stats));

	mu_expr_helpers_strlist_free (strlist);
	g_free (tmpview);
}


static void
query_01 (gconstpointer data)
{
	MuMsgSQLite *row;
	TestData *mydata = (TestData*)data;

	row = query_mgr_run_str (mydata->mgr, "s:cperl-mode", NULL, 
				    TRUE, NULL);
	g_assert (row != NULL); 
	g_assert (mu_msg_sqlite_next (row, NULL)); /* move to first item */ 

	g_assert_cmpstr (mu_msg_sqlite_get_subject(row),==,
			 "Re: What does 'run' do in cperl-mode?");
	/* find only one */
	g_assert (!mu_msg_sqlite_next (row, NULL));

	mu_msg_sqlite_destroy (row);
}

static void
query_02 (gconstpointer data)
{
	MuMsgSQLite *row;
	TestData *mydata = (TestData*)data;

	row = query_mgr_run_str (mydata->mgr, "d:20080806-20080807 f:bspears", 
				    NULL, TRUE, NULL);
	g_assert (row != NULL); 
	g_assert (mu_msg_sqlite_next (row, NULL)); /* move to first item */ 

	g_assert_cmpstr (mu_msg_sqlite_get_subject(row),==,
			 "Error when compiling emacs-unicode-2 on windows");
	g_assert_cmpstr (mu_msg_sqlite_get_from(row),==,
			 "bspears@example.com");
	g_assert_cmpstr (mu_msg_sqlite_get_to(row),==,
			 "help-gnu-emacs@gnu.org");	
	g_assert_cmpstr (mu_msg_sqlite_get_subject(row),==,
			 "Error when compiling emacs-unicode-2 on windows");

	/* find only one */
	g_assert (!mu_msg_sqlite_next (row, NULL));

	mu_msg_sqlite_destroy (row);
}

static void
query_03 (gconstpointer data)
{
	MuMsgSQLite *row;
	int count = 0;
	TestData *mydata = (TestData*)data;

	row = query_mgr_run_str (mydata->mgr, "s:emacs", 
				    NULL, TRUE, NULL);
	g_assert (row != NULL); 
	
	while (mu_msg_sqlite_next (row, NULL))
		++count;

	g_assert_cmpint (count,==,6);

	mu_msg_sqlite_destroy (row);
}



static void
query_04 (gconstpointer data)
{
#if MU_HAVE_XAPIAN
	MuMsgSQLite *row;
	TestData *mydata = (TestData*)data;
	row = query_mgr_run_str (mydata->mgr, "x:facets", 
				    NULL, TRUE, NULL);
	g_assert (row != NULL); 
	g_assert(mu_msg_sqlite_next (row, NULL));
	g_assert_cmpstr (mu_msg_sqlite_get_subject(row),==,
			 "Re: [Xapian-discuss] FLAG_WILDCARD,"
			 " add_database and performance");

	/* there should be only one */
	g_assert (!mu_msg_sqlite_next (row, NULL));

	mu_msg_sqlite_destroy (row);
#endif /*MU_HAVE_XAPIAN*/
}

static void
query_05 (gconstpointer data)
{
#if MU_HAVE_XAPIAN
	MuMsgSQLite *row;
	TestData *mydata = (TestData*)data;

	row = query_mgr_run_str (mydata->mgr, "s:usability x:incurable", 
				    NULL, TRUE, NULL);
	g_assert (row != NULL); 
	g_assert(mu_msg_sqlite_next (row, NULL));
	g_assert_cmpstr (mu_msg_sqlite_get_to(row),==,
			 "help-gnu-emacs@gnu.org");
	g_assert_cmpstr (mu_msg_sqlite_get_from(row),==,
			 "anon@example.com");
	
	/* there should be only one */
	g_assert (!mu_msg_sqlite_next (row, NULL));

	mu_msg_sqlite_destroy (row);
#endif /*MU_HAVE_XAPIAN*/
}



static void
query_06 (gconstpointer data)
{
#if MU_HAVE_XAPIAN
	MuMsgSQLite *row;
	TestData *mydata = (TestData*)data;
	
	row = query_mgr_run_str (mydata->mgr, "d:-20080807 x:xapian", 
				    NULL, TRUE, NULL);
	g_assert (row != NULL); 
	g_assert(mu_msg_sqlite_next (row, NULL));
	g_assert_cmpstr (mu_msg_sqlite_get_subject(row),==,
			 "Re: [Xapian-discuss] FLAG_WILDCARD,"
			 " add_database and performance");
	
	/* there should be only one */
	g_assert (!mu_msg_sqlite_next (row, NULL));

	mu_msg_sqlite_destroy (row);
#endif /*MU_HAVE_XAPIAN*/
}


static void
query_07 (gconstpointer data)
{
	MuMsgSQLite *row;
	guint num;
	TestData *mydata = (TestData*)data;

	row = query_mgr_run_str (mydata->mgr, "F:N", 
				    NULL, TRUE, NULL);
	g_assert (row != NULL); 
	
	num = 0;
	while (mu_msg_sqlite_next (row, NULL))
		++num;
	g_assert_cmpuint (num, ==, 7);
	mu_msg_sqlite_destroy (row);

	row = query_mgr_run_str (mydata->mgr, "F:S", 
				    NULL, TRUE, NULL);
	g_assert (row != NULL); 
	num = 0;
	while (mu_msg_sqlite_next (row, NULL))
		++num;
	g_assert_cmpuint (num,==, 11);
	mu_msg_sqlite_destroy (row);

	row = query_mgr_run_str (mydata->mgr, "F:SN", 
				    NULL, TRUE, NULL);
	g_assert (row != NULL); 
	num = 0;
	while (mu_msg_sqlite_next (row, NULL))
		++num;
	g_assert_cmpuint (num, ==, 0);
	mu_msg_sqlite_destroy (row);
}



static void
stats_01 (gconstpointer data)
{
	TestData *mydata = (TestData*)data;
	MuQueryStats stats;

	query_mgr_stats_str (mydata->mgr, "F:N", &stats);
	
	g_assert_cmpuint(stats._msg_num,==,7);
	
	

}



static MuResult
stats_callback (MuIndexStats *stats, void *user_data)
{
	((int)*(int*)user_data)++;
	return MU_OK;
}

static void
mu_run_index (gconstpointer data)
{
	MuIndex *index;
	MuIndexStats stats;
	int num;
	TestData *mydata = (TestData*)data;

	g_type_init ();
	mu_msg_gmime_init ();
	
	index = mu_index_new (mydata->spath, mydata->xpath);
	g_assert(index != NULL);

	memset (&stats, 0, sizeof(MuIndexStats));

	num = 0;
	g_assert_cmpint (mu_index_run (index,ABS_SRCDIR "/TestMaildir",
				       FALSE, &stats, stats_callback, &num),
			 ==, MU_OK);
	g_assert_cmpint (num,==,18);
	g_assert_cmpint (stats._added,==,18);

	mu_index_destroy (index);

	mu_msg_gmime_uninit ();
}


static void
shutup (void) {}

int
main (int argc, char *argv[])
{
	TestData data;
	char *name;
	int result;

	memset (&data,0,sizeof(TestData));

	g_test_init (&argc, &argv, NULL);
	g_log_set_handler (NULL,
			   G_LOG_LEVEL_DEBUG|G_LOG_LEVEL_MESSAGE|G_LOG_LEVEL_INFO,
			   (GLogFunc)shutup, NULL);

	name   = random_tmpdir ();
	data.spath  = g_strdup_printf ("%s/sqlite.db", name);
	data.xpath  = g_strdup_printf ("%s/xapian.db", name);
	data.bmpath = NULL;

	g_assert (g_mkdir (name, 0700) == 0);
	mu_run_index (&data);

	data.mgr = mu_query_mgr_new (data.spath, data.xpath, data.bmpath, NULL);
	g_assert (data.mgr);
	
	g_test_add_data_func ("/all/test-query01", &data, query_01);
	g_test_add_data_func ("/all/test-query02", &data, query_02);
	g_test_add_data_func ("/all/test-query03", &data, query_03);
	g_test_add_data_func ("/all/test-query04", &data, query_04);
	g_test_add_data_func ("/all/test-query05", &data, query_05);
	g_test_add_data_func ("/all/test-query06", &data, query_06);
	g_test_add_data_func ("/all/test-query07", &data, query_07);

	g_test_add_data_func ("/all/test-stats01", &data, stats_01);

	result = g_test_run ();
	
	g_free (name);
	g_free (data.spath);
	g_free (data.xpath);
	g_free (data.bmpath);

	mu_query_mgr_destroy (data.mgr);

	return result;
}
