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
#include "mu/mu-msg-str.h"

static void
test_display_str_size (void)
{
	g_assert_cmpstr (mu_msg_str_size_s(1000),==,"1.0k");
	g_assert_cmpstr (mu_msg_str_size_s(1000*1000),==,"1.0M");
	g_assert_cmpstr (mu_msg_str_size_s(1000*1000*1000),==,"1000.0M");
}

static void
test_display_str_flags (void)
{
	g_assert_cmpstr (mu_msg_str_flags_s(MU_MSG_FLAG_NEW),==,"N");
	g_assert_cmpstr (mu_msg_str_flags_s
			 (MU_MSG_FLAG_DRAFT|MU_MSG_FLAG_REPLIED),==,"DR");
	g_assert_cmpstr (mu_msg_str_flags_s
			 (MU_MSG_FLAG_FLAGGED|
			  MU_MSG_FLAG_PASSED|
			  MU_MSG_FLAG_SEEN),==,"FPS");
}


int
main (int argc, char *argv[])
{
	g_test_init (&argc, &argv, NULL);
	g_test_add_func ("/find/test-display-str-size", test_display_str_size);
	g_test_add_func ("/find/test-display-str-flags", test_display_str_flags);

	return g_test_run ();
}
