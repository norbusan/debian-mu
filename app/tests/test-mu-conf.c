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
#include "app/mu-conf.h"
#include "app/mu-app.h"

/* make sure creating a maildir succeeds */
static void
test_mu_conf (void)
{
	MuConf *conf;
	gboolean b;
	int     i;
	char*   s;

	char *d[] = {"testmu"};
	char **dummy = (char**)d;

	i = 1;
	g_assert (mu_app_init (&i, &dummy, "test-mu-conf"));

	g_assert ((conf = mu_conf_new ("test.conf", "foo")));
	
	/* positive */
	g_assert (mu_conf_get_bool (conf, "somebool", &b));
	g_assert (b);
	
	g_assert (mu_conf_get_int (conf, "someint", &i));
	g_assert (i == 42);
	
	g_assert (mu_conf_get_string (conf, "somestring", &s));
	g_assert_cmpstr (s, ==, "hello world");
	g_free (s);
	
	/* negative */
	g_assert (!mu_conf_get_string (conf, "anotherstring", &s));
	g_assert (!mu_conf_get_bool (conf, "anotherbool", &b));
	
	mu_conf_destroy (conf);
	mu_app_uninit ();
}

int
main (int argc, char *argv[])
{
	g_test_init (&argc, &argv, NULL);
	g_test_add_func ("/mu/conf", test_mu_conf);
	
	return g_test_run ();
}
