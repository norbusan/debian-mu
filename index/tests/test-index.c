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

#include <stdlib.h>
#include <glib/gstdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "index/mu-index.h"
#include "msg/mu-msg-gmime.h"

char*
random_tmpdir (void)
{
	return g_strdup_printf ("%s%c%s.%x", g_get_tmp_dir(), 
				G_DIR_SEPARATOR, "mu-test-index", 
				(int)random()*getpid()*(int)time(NULL));	
}


static
void test_mu_new_destroy_index (void)
{
	MuIndex *index;
	char *name = random_tmpdir();

	char *sqlite_db  = g_strdup_printf ("%s/sqlite.db", name);
	char *xapian_db  = g_strdup_printf ("%s/xapian.db", name);

	g_assert (g_mkdir (name, 0700) == 0);
	
	index = mu_index_new (sqlite_db, xapian_db);
	g_assert (g_access(sqlite_db, F_OK) == 0);

	g_assert (index != NULL);
	
	mu_index_destroy (index);
	
	g_assert (g_access(sqlite_db, F_OK) == 0);
	
#ifdef MU_HAVE_XAPIAN
	g_assert (g_access (xapian_db, F_OK) == 0);
#endif /*MU_HAVE_XAPIAN*/	

	g_free (sqlite_db);
	g_free (xapian_db);
	g_free (name);
}


static MuResult
stats_callback (MuIndexStats *stats, void *user_data)
{
	((int)*(int*)user_data)++;
	return MU_OK;
}

static
void test_mu_index_stats_01 (void)
{
	MuIndex *index;
	MuIndexStats stats;
	char *name = random_tmpdir();
	int num;
	char *sqlite_db  = g_strdup_printf ("%s/sqlite.db", name);
	char *xapian_db  = g_strdup_printf ("%s/xapian.db", name);
	g_assert (g_mkdir (name, 0700) == 0);
	
	index = mu_index_new (sqlite_db, xapian_db);
	
	memset (&stats, 0, sizeof(MuIndexStats));

	num = 0;
	g_assert_cmpint (mu_index_stats (index, 
					 ABS_SRCDIR "/TestMaildir",
					 &stats, stats_callback, &num),
			 ==, MU_OK);
	g_assert_cmpint (num,==,10);
	g_assert_cmpint (stats._processed,==,10);
	
	mu_index_destroy (index);
	g_free (sqlite_db);
	g_free (xapian_db);
	g_free (name);
}


static
void test_mu_index_run_tuned (size_t sqlite_tx_size, 
			      size_t synchronous, size_t temp_store,
			      size_t xapian_tx_size, gboolean sort_inodes)
{
	MuIndex *index;
	MuIndexStats stats;
	char *name = random_tmpdir();
	int num;
	char *sqlite_db  = g_strdup_printf ("%s/sqlite.db", name);
	char *xapian_db  = g_strdup_printf ("%s/xapian.db", name);
	g_assert (g_mkdir (name, 0700) == 0);

	g_type_init ();
	mu_msg_gmime_init ();
	
	index = mu_index_new (sqlite_db, xapian_db);
	g_assert(index != NULL);
	
	mu_index_tune (index, sqlite_tx_size, synchronous, temp_store,
		       xapian_tx_size, sort_inodes);
	
	memset (&stats, 0, sizeof(MuIndexStats));

	num = 0;
	g_assert_cmpint (mu_index_run (index,ABS_SRCDIR "/TestMaildir",
				       FALSE, &stats, stats_callback, &num),
			 ==, MU_OK);
	g_assert_cmpint (num,==,10);
	g_assert_cmpint (stats._added,==,10);

	/* run again */
	memset (&stats, 0, sizeof(MuIndexStats));
	num = 0;
	g_assert_cmpint (mu_index_run (index,ABS_SRCDIR "/TestMaildir",
				       FALSE, &stats, stats_callback, &num),
			 ==, MU_OK);
	g_assert_cmpint (num,==,10);
	g_assert_cmpint (stats._added,==,0);
	g_assert_cmpint (stats._uptodate,==,10);

	mu_index_destroy (index);
	mu_msg_gmime_uninit ();

	g_free (sqlite_db);
	g_free (xapian_db);
	g_free (name);
}


static
void test_mu_index_run_01 (void)
{
	test_mu_index_run_tuned (1,2,1,1,FALSE); /* slowest */
}

static
void test_mu_index_run_02 (void)
{
	test_mu_index_run_tuned (100,0,2,1000,TRUE); /* default */
}


static void
shutup (void) {}

int
main (int argc, char *argv[])
{
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/index/new-destroy-index", test_mu_new_destroy_index);
	g_test_add_func ("/index/stats-01", test_mu_index_stats_01);
	g_test_add_func ("/index/run-01", test_mu_index_run_01);
	g_test_add_func ("/index/run-02", test_mu_index_run_02);

	g_log_set_handler (NULL,
			   G_LOG_LEVEL_DEBUG|G_LOG_LEVEL_MESSAGE|G_LOG_LEVEL_INFO,
			   (GLogFunc)shutup, NULL);

	return g_test_run ();
}
