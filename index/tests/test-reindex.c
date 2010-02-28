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

#define _XOPEN_SOURCE 500 /* for random() */

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


static MuResult
stats_callback (MuIndexStats *stats, void *user_data)
{
	((int)*(int*)user_data)++;
	return MU_OK;
}


static
void test_mu_index_reindex ()
{
	MuIndex *index;
	MuIndexStats stats;
	char *name = random_tmpdir();
	int num;
	char *sqlite_db  = g_strdup_printf ("%s/sqlite.db", name);
	char *xapian_db  = g_strdup_printf ("%s/xapian.db", name);
	g_assert (g_mkdir (name, 0700) == 0);
	char *cmd;
	char *maildir;
	
	g_type_init ();
	mu_msg_gmime_init ();
	
	index = mu_index_new (sqlite_db, xapian_db);
	g_assert(index != NULL);
		
	memset (&stats, 0, sizeof(MuIndexStats));

	num = 0;
	
	/* copy the test maildir to our random dir */
	cmd = g_strdup_printf ("cp -R " ABS_SRCDIR "/TestMaildir %s",
			       name);
	g_assert (system (cmd) == 0);
	g_free(cmd);
	
	maildir = g_strdup_printf ("%s/TestMaildir", name);
	g_assert_cmpint (mu_index_run (index,maildir,
				       FALSE, &stats, stats_callback, &num),
			 ==, MU_OK);
	g_assert_cmpint (num,==,10);
	g_assert_cmpint (stats._added,==,10);

	/* run again */
	memset (&stats, 0, sizeof(MuIndexStats));
	num = 0;
	g_assert_cmpint (mu_index_run (index,maildir,
				       FALSE, &stats, stats_callback, &num),
			 ==, MU_OK);
	g_assert_cmpint (num,==,10);
	g_assert_cmpint (stats._added,==,0);
	g_assert_cmpint (stats._uptodate,==,10);

	sleep (1); /* sleep one seconds, or we are too fast for the 
		      timestamps to change...*/

	/* now, remove two files, update two */
	cmd = g_strdup_printf ("cd %s; "
			"rm cur/1220863042.12663_1.mindcrime:2,S;"
			"rm cur/1220863060.12663_3.mindcrime:2,S", maildir);
	g_assert (system (cmd) == 0);
	g_free (cmd);
	cmd = g_strdup_printf ("cd %s; "
			"touch cur/;"
			"touch cur/1220863087.12663_19.mindcrime:2,S;"
			"touch new/1220863087.12663_25.mindcrime", maildir);	
	g_assert (system (cmd) == 0);
	g_free (cmd);

	/* run again */
	memset (&stats, 0, sizeof(MuIndexStats));
	num = 0;
	g_assert_cmpint (mu_index_run (index,maildir,
				       FALSE, &stats, stats_callback, &num),
			 ==, MU_OK);

	g_assert_cmpint (num,==,8); /* there should be only two left */
	g_assert_cmpint (stats._added,==,0);
	g_assert_cmpint (stats._uptodate,==,6);
	g_assert_cmpint (stats._updated,==,2);

	mu_index_destroy (index);
	mu_msg_gmime_uninit ();

	g_free (sqlite_db);
	g_free (xapian_db);
        g_free (maildir);
	g_free (name);
}



static void
shutup (void) {}

int
main (int argc, char *argv[])
{
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/index/reindex", test_mu_index_reindex);

	g_log_set_handler (NULL,
			   G_LOG_LEVEL_DEBUG|G_LOG_LEVEL_MESSAGE|G_LOG_LEVEL_INFO,
			   (GLogFunc)shutup, NULL);

	return g_test_run ();
}
