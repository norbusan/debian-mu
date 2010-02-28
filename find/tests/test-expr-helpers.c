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
#include "find/mu-expr-helpers.h"

static void
test_strlist_from_args (void)
{
	GSList *args;
	char *argv[3] = {"aap", "noot", "mies"};
	
	args = mu_expr_helpers_strlist_from_args (3, argv);
	g_assert(args);
	g_assert(g_slist_length(args) == 3);
	
	/* note, here we get 'm in the same order as the input */
	g_assert_cmpstr ((char*)args->data, == ,"aap");
	g_assert_cmpstr ((char*)args->next->data, == ,"noot");
	g_assert_cmpstr ((char*)args->next->next->data, == ,"mies");
	
	mu_expr_helpers_strlist_free (args);
}


static void
test_strlist_from_str_1 (void)
{
	GSList *args;
	GError *err = NULL;
	const char *str = "hello world 'foo bar'";
	
	args = mu_expr_helpers_strlist_from_str (str, &err);
	if (err) {
		g_message ("error: %s\n", err->message);
		g_error_free (err);
	}
		
	g_assert(args);
	g_assert(g_slist_length(args) == 3);

	/* note, here we get 'm in the same order as the input */
	g_assert_cmpstr ((char*)args->data, == ,"hello");
	g_assert_cmpstr ((char*)args->next->data, == ,"world");
	g_assert_cmpstr ((char*)args->next->next->data, == ,"foo bar");
	
	mu_expr_helpers_strlist_free (args);
}



static void
test_strlist_from_str_2 (void)
{
	GSList *args;
	const char *str = " 'foo bar'    'cuux'la ";
	
	args = mu_expr_helpers_strlist_from_str (str, NULL);
	g_assert(args);
	g_assert(g_slist_length(args) == 2);

	/* note, here we get 'm in the same order as the input */
	g_assert_cmpstr ((char*)args->data, == ,"foo bar");
	g_assert_cmpstr ((char*)args->next->data, == ,"cuuxla");
	
	mu_expr_helpers_strlist_free (args);
}



static void
test_strlist_from_str_3 (void)
{
	GSList *args;
	const char *str = " test \\'quote \\\\la\\\\ ";
	
	args = mu_expr_helpers_strlist_from_str (str, NULL);
	g_assert(args);
	g_assert(g_slist_length(args) == 3);

	/* note, here we get 'm in the same order as the input */
	g_assert_cmpstr ((char*)args->data, == ,"test");
	g_assert_cmpstr ((char*)args->next->data, == ,"'quote");
	g_assert_cmpstr ((char*)args->next->next->data, == ,"\\la\\");
		
	mu_expr_helpers_strlist_free (args);
}


static void
test_sortfields_valid (void)
{
	g_assert (mu_expr_helpers_sortfields_valid("sdz"));
	g_assert (!mu_expr_helpers_sortfields_valid("sdQ"));
}


int
main (int argc, char *argv[])
{
	g_test_init (&argc, &argv, NULL);
	
	g_test_add_func ("/find/test-list-from-args", test_strlist_from_args);
	g_test_add_func ("/find/test-list-from-str1", test_strlist_from_str_1);
	g_test_add_func ("/find/test-list-from-str2", test_strlist_from_str_2);
	g_test_add_func ("/find/test-list-from-str3", test_strlist_from_str_3);
	g_test_add_func ("/find/test-sortfields-valid", test_sortfields_valid);

	return g_test_run ();
}
