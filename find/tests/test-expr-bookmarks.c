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
#include "find/mu-expr-bookmarks.h"

static void
test_bookmarks_create_destroy (void)
{
	MuExprBookmarks *bookmarks;

	bookmarks = mu_expr_bookmarks_new ("bookmarks.test", NULL);
	g_assert (bookmarks);

	mu_expr_bookmarks_destroy (bookmarks);
}


static void
test_bookmarks_create_destroy_failed (void)
{
	MuExprBookmarks *bookmarks;

	if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDERR)) {
		bookmarks = mu_expr_bookmarks_new ("nonexisting.test", NULL);
		g_assert (!bookmarks);
		mu_expr_bookmarks_destroy (bookmarks);
	}
	g_test_trap_assert_failed(); /* expected to fail */
}


static void
test_bookmarks_find1 (void)
{
	MuExprBookmarks *bookmarks;
	char *bm;

	bookmarks = mu_expr_bookmarks_new ("bookmarks.test", NULL);
	g_assert (bookmarks);

	bm = mu_expr_bookmarks_resolve (bookmarks, "test1");
	g_assert_cmpstr (bm,==,"s:foo t:bar");
	g_free (bm);

	mu_expr_bookmarks_destroy (bookmarks);
}


static void
test_bookmarks_find1_failed (void)
{
	MuExprBookmarks *bookmarks;
	char *bm;

	bookmarks = mu_expr_bookmarks_new ("bookmarks.test", NULL);
	g_assert (bookmarks);

	bm = mu_expr_bookmarks_resolve (bookmarks, "nonexisting");
	g_assert (!bm);

	mu_expr_bookmarks_destroy (bookmarks);
}


static void
test_bookmarks_failed2 (void)
{
	if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDERR)) {
		/* GLIB-Crititical */
		mu_expr_bookmarks_resolve (NULL, "nonexisting");
	}
	g_test_trap_assert_failed(); /* expected to fail */
}


int
main (int argc, char *argv[])
{
	g_test_init (&argc, &argv, NULL);
	
	g_test_add_func ("/find/test-bookmarks-create-destroy", 
			 test_bookmarks_create_destroy);
	g_test_add_func ("/find/test-bookmarks-create-destroy-failed", 
			 test_bookmarks_create_destroy_failed);
	g_test_add_func ("/find/test-bookmarks-find1", 
				 test_bookmarks_find1);
	g_test_add_func ("/find/test-bookmarks-find1-failed", 
				 test_bookmarks_find1_failed);
	g_test_add_func ("/find/test-bookmarks-failed2", 
			 test_bookmarks_failed2);

	return g_test_run ();
}
