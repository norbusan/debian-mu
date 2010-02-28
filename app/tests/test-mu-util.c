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
#include "app/mu-util.h"

static void
test_homedir_expand (void)
{
	const char *d1 = "~/foo/bar"; /* should change */
	const char *d2 = "~lala";     /* should not change */

	char *expected, *actual;

	const char *homedir = g_getenv ("HOME");
	if (!homedir)
		homedir = g_get_home_dir ();

	expected = g_strdup_printf ("%s/foo/bar", homedir);
	actual   = mu_util_homedir_expand (d1);
	g_assert_cmpstr (expected,==,actual);
	g_free (expected);
	g_free (actual);

	expected = g_strdup(d2);
	actual   = mu_util_homedir_expand (d2);
	g_assert_cmpstr (expected,==,actual);
	g_free (expected);
	g_free (actual);	

	
	g_assert (mu_util_homedir_expand(NULL) == NULL);
}

int
main (int argc, char *argv[])
{
	g_test_init (&argc, &argv, NULL);
	
	g_test_add_func ("/mu/homedir-expand", test_homedir_expand); 

	return g_test_run ();
}
