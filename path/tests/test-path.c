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

#include <stdio.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "path/mu-path.h"


char*
random_tmpdir (void)
{
	return g_strdup_printf ("%s%c%s.%x", g_get_tmp_dir(), 
				G_DIR_SEPARATOR, "mu-test-path", 
				(int)random()*getpid()*(int)time(NULL));	
}


/* make sure creating a maildir succeeds */
static void
test_mu_path_make_maildir (void)
{
	char *name = random_tmpdir();

	const char *subs[] = { "", "/tmp", "/cur", "/new" };
	int i;
	
	g_assert (mu_path_make_maildir (name, 0700));
	
	for (i = 0; i != sizeof(subs)/sizeof(subs[0]); ++i) {
		char* path = g_strdup_printf ("%s%s", name, subs[i]);
		g_assert (g_access (path, F_OK) == 0);
		g_free (path);
	}

	g_free (name);
}


static void
test_mu_path_get_file_flags (void)
{
	int i;

	struct {
		const char* path;
		MuMsgFlags expected;
	} cases[] = {
		/* in new/, so gets the NEW flag */
		{"/test/new/1220591523.25027_5.mindcrime", 
		 MU_MSG_FLAG_NEW},
	
		/* in new/, so ignore flags but NEW */
		{"/test/new/1218743724.25878_1.mindcrime:2,S",
		 MU_MSG_FLAG_NEW},
	
		/* seen */
		{"/tests/foo/cur/1218743724.25878_1.mindcrime:2,S",
		 MU_MSG_FLAG_SEEN},
		
		/* seen and replied */
		{"/tests/foo/cur/1218743724.25878_1.mindcrime:2,SR",
		 MU_MSG_FLAG_SEEN|MU_MSG_FLAG_REPLIED},
		
		/* seen and replied, using '!' separator */
		{"/tests/foo/cur/1218743724.25878_1.mindcrime!2,SR",
		 MU_MSG_FLAG_SEEN|MU_MSG_FLAG_REPLIED},

		/*seen and replied, using '!' separator */
		{"/tests/bla/cur/12sdfdsf8743724.25878_1.mindcrime!2,SR",
		 MU_MSG_FLAG_SEEN|MU_MSG_FLAG_REPLIED},

		/*draft and passed using '!' separator */
		{"/tests/bla/cur/12sdfdsf8743724.25878_1.mindcrime!2,DP",
		 MU_MSG_FLAG_DRAFT|MU_MSG_FLAG_PASSED},

		/*seen and replied and something random to be ignored*/
		{"/tests/bla/cur/12sdfdsf8743724.25878_1.mindcrime!2,SRZ",
		 MU_MSG_FLAG_SEEN|MU_MSG_FLAG_REPLIED},
		
		/*multiple times T, Z and R*/
		{"/tests/bla/cur/12sdfdsf8743724.25878_1.mindcrime:2,TTTZR",
		 MU_MSG_FLAG_TRASHED|MU_MSG_FLAG_REPLIED},
	};

	for (i = 0; i != sizeof (cases)/sizeof(cases[0]); ++i) {
		g_assert_cmpint (mu_path_get_file_flags (cases[i].path), ==, 
				 cases[i].expected);
	}
}

static void
test_mu_path_get_file_flags_failed (void)
{
	int i;

	struct {
		const char* path;
		MuMsgFlags expected;
	} cases[] = {		
		/* not a file*/
		{NULL,
		 MU_MSG_FLAG_UNKNOWN},
		
		/* not a file*/
		{"/tests/bla/cur/12sdfdsf8743724.25878_1.mindcrime:2,TTTZR/",
		 MU_MSG_FLAG_UNKNOWN},
	};

	for (i = 0; i != sizeof (cases)/sizeof(cases[0]); ++i) {
		if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDERR)) {
			g_assert_cmpint (mu_path_get_file_flags (cases[i].path), ==, 
					 cases[i].expected);
		}
		g_test_trap_assert_failed(); /* expected to fail */
	}
}


static void
test_mu_path_symlink_message (void)
{
	char *dir = random_tmpdir();
	struct stat statbuf;
	char *expected; 

	g_assert (mu_path_make_maildir (dir, 0700));
	
	g_assert (mu_path_symlink_message (ABS_SRCDIR 
					   "/TestMaildir/cur/cur0002.server:2,S",
					   dir));

	expected = g_strdup_printf ("%s/cur/cur0002.server:2,S", dir);
	g_debug ("%s", expected);
		
	g_assert (access (expected, F_OK) == 0);

	g_assert (stat (expected, &statbuf) == 0);
	g_assert (statbuf.st_mode & S_IFLNK);

	g_free (expected);
	g_free (dir);
}

static void
test_mu_path_symlink_message_failed (void)
{
	char *dir = random_tmpdir();

	g_assert (mu_path_make_maildir (dir, 0700));

	if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDERR)) {
		g_assert (mu_path_symlink_message ("non-existing",
						   dir));
	}
	g_test_trap_assert_failed(); /* expected to fail */


	if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDERR)) {
		g_assert (mu_path_symlink_message 
			  (ABS_SRCDIR "/TestMaildir/cur/cur0002.server:2,S",
			   "non-existing"));
	}
	g_test_trap_assert_failed(); /* expected to fail */

	g_free (dir);
}

static void
shutup (void) {}

int
main (int argc, char *argv[])
{
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/path/make-maildir", test_mu_path_make_maildir);
	g_test_add_func ("/path/get-file-flags", test_mu_path_get_file_flags);
	g_test_add_func ("/path/get-file-flags-failed",
			 test_mu_path_get_file_flags_failed);
	g_test_add_func ("/path/symlink-message",test_mu_path_symlink_message);
	g_test_add_func ("/path/symlink-message-failed",
			 test_mu_path_symlink_message_failed);

	g_log_set_handler (NULL,
			   G_LOG_LEVEL_DEBUG|G_LOG_LEVEL_MESSAGE|G_LOG_LEVEL_INFO,
			   (GLogFunc)shutup, NULL);

	return g_test_run ();
}
