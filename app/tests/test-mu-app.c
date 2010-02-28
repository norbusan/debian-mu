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

#include <glib.h>
#include <stdlib.h>

#include "app/mu-app.h"
#include "app/mu-app-priv.h"
#include "app/mu-util.h"

static void
test_mu_app_missing_arg (void)
{
	char* my_argv[] = { "test-mu" };
	int my_argc = sizeof(my_argv)/sizeof(my_argv[0]);
	char **argvp = (char**)my_argv;

	if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDERR)) 
		g_assert (mu_app_init (NULL, &argvp, "test-mu") == FALSE);	
	g_test_trap_assert_failed ();

	if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDERR)) 
		g_assert (mu_app_init (&my_argc, NULL, "test-mu") == FALSE);
	g_test_trap_assert_failed ();
	
	if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDERR)) 
		g_assert (mu_app_init (&my_argc, &argvp, NULL) == FALSE);	
	g_test_trap_assert_failed ();
}



static void
test_mu_app_home (void)
{
	char *mu_home_str;
	char* my_argv[] = {
		"test-mu"
	};
	char **argvp = (char**)my_argv;
	int my_argc = sizeof(my_argv)/sizeof(my_argv[0]);
	const char *home;

	g_assert (mu_app_init (&my_argc, &argvp, "test-mu"));

	home = getenv("HOME");
	if (!home)
		home = g_get_home_dir();
	
	mu_home_str = g_strdup_printf ("%s%c%s", home, 
				       G_DIR_SEPARATOR,
				       MU_HOME_DIR);
	g_assert_cmpstr (mu_app_home(), ==, mu_home_str);
	g_free (mu_home_str);
	   
	mu_app_uninit ();
}


static void
test_mu_app_sqlite_storage_path (void)
{
	char *storage;
	char* my_argv[] = {
		"test-mu"
	};
	int my_argc = sizeof(my_argv)/sizeof(my_argv[0]);
	char **argvp = (char**)my_argv;

	g_assert (mu_app_init (&my_argc, &argvp, "test-mu"));
	
	storage = g_strdup_printf ("%s%c%s", mu_app_home(), 
				   G_DIR_SEPARATOR,
				   MU_SQLITE_DB_NAME);

	g_assert_cmpstr (mu_app_sqlite_path(), ==, storage);
	g_free (storage);
	
	mu_app_uninit ();
}

static void
test_mu_app_xapian_storage_path (void)
{
	char *storage;
	char* my_argv[] = {
		"test-mu"
	};
	int my_argc = sizeof(my_argv)/sizeof(my_argv[0]);
	char **argvp = (char**)my_argv;

	g_assert (mu_app_init (&my_argc, &argvp, "test-mu"));
	
	storage = g_strdup_printf ("%s%c%s", mu_app_home(), 
				   G_DIR_SEPARATOR,
				   MU_XAPIAN_DB_NAME);

	g_assert_cmpstr (mu_app_xapian_path(), ==, storage);
	g_free (storage);
	
	mu_app_uninit ();
}


int
main (int argc, char *argv[])
{
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/app/init-missing-arg", 
			 test_mu_app_missing_arg);
	g_test_add_func ("/app/home",
			 test_mu_app_home);
	g_test_add_func ("/app/sqlite-storage-path", 
			 test_mu_app_sqlite_storage_path);
	g_test_add_func ("/app/xapian-storage-path", 
			 test_mu_app_xapian_storage_path);

	return g_test_run ();
}
