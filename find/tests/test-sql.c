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

#include "find/mu-expr.h"
#include "find/mu-expr-sql.h"
#include "find/mu-expr-helpers.h"

static void
test_sql_case_1 (void)
{
	char *sql;
	GSList *expr_strs;
        MuExprList *exprs;
	const char *expr="f:Bob s:Seattle";
        const char *expected=
	"SELECT   m.recipients, m.cc, m.sender, m.subject, m.msg_id, \n"
	"         NULL AS dummy2, m.flags, m.mdate, m.msize, m.mpath, \n"
	"         m.priority, m.tstamp, m.id\n"
	"FROM     message m\n"
	"WHERE    m.sender LIKE '%Bob%'\n"
	"AND      m.subject LIKE '%Seattle%'\n"
	"ORDER BY m.mdate DESC;\n";

	expr_strs = mu_expr_helpers_strlist_from_str (expr,NULL); g_assert (expr_strs);
	exprs = mu_expr_list_new     (expr_strs, NULL);  g_assert (exprs);
	sql   = mu_expr_sql_generate (exprs,NULL,"d",FALSE); g_assert (sql);
	mu_expr_helpers_strlist_free (expr_strs);

	g_assert_cmpstr (sql,==,expected);
	
	mu_expr_list_destroy (exprs, FALSE);
	g_free (sql);
}
static void
test_sql_case_2 (void)
{
	char *sql;
	GSList *expr_strs;
        MuExprList *exprs;
	const char *expr="fs:Hello c:World";
        const char *expected=
	"SELECT   m.recipients, m.cc, m.sender, m.subject, m.msg_id, \n"
	"         NULL AS dummy2, m.flags, m.mdate, m.msize, m.mpath, \n"
	"         m.priority, m.tstamp, m.id\n"
	"FROM     message m\n"
	"WHERE    (m.sender LIKE '%Hello%' OR m.subject LIKE '%Hello%')\n"
	"AND      m.cc LIKE '%World%'\n"
	"ORDER BY m.mdate DESC;\n";

	expr_strs = mu_expr_helpers_strlist_from_str (expr,NULL); g_assert (expr_strs);
	exprs = mu_expr_list_new     (expr_strs, NULL);  g_assert (exprs);
	sql   = mu_expr_sql_generate (exprs,NULL,"d",FALSE); g_assert (sql);
	mu_expr_helpers_strlist_free (expr_strs);

	g_assert_cmpstr (sql,==,expected);
	
	mu_expr_list_destroy (exprs, FALSE);
	g_free (sql);
}
static void
test_sql_case_3 (void)
{
	char *sql;
	GSList *expr_strs;
        MuExprList *exprs;
	const char *expr="F:N s:linux";
        const char *expected=
	"SELECT   m.recipients, m.cc, m.sender, m.subject, m.msg_id, \n"
	"         NULL AS dummy2, m.flags, m.mdate, m.msize, m.mpath, \n"
	"         m.priority, m.tstamp, m.id\n"
	"FROM     message m\n"
	"WHERE    m.flags & 2 = 2\n"
	"AND      m.subject LIKE '%linux%'\n"
	"ORDER BY m.mdate DESC;\n";

	expr_strs = mu_expr_helpers_strlist_from_str (expr,NULL); g_assert (expr_strs);
	exprs = mu_expr_list_new     (expr_strs, NULL);  g_assert (exprs);
	sql   = mu_expr_sql_generate (exprs,NULL,"d",FALSE); g_assert (sql);
	mu_expr_helpers_strlist_free (expr_strs);

	g_assert_cmpstr (sql,==,expected);
	
	mu_expr_list_destroy (exprs, FALSE);
	g_free (sql);
}
static void
test_sql_case_4 (void)
{
	char *sql;
	GSList *expr_strs;
        MuExprList *exprs;
	const char *expr="s:^linux F:RT";
        const char *expected=
	"SELECT   m.recipients, m.cc, m.sender, m.subject, m.msg_id, \n"
	"         NULL AS dummy2, m.flags, m.mdate, m.msize, m.mpath, \n"
	"         m.priority, m.tstamp, m.id\n"
	"FROM     message m\n"
	"WHERE    m.subject NOT LIKE '%linux%'\n"
	"AND      m.flags & 80 = 80\n"
	"ORDER BY m.mdate DESC;\n";

	expr_strs = mu_expr_helpers_strlist_from_str (expr,NULL); g_assert (expr_strs);
	exprs = mu_expr_list_new     (expr_strs, NULL);  g_assert (exprs);
	sql   = mu_expr_sql_generate (exprs,NULL,"d",FALSE); g_assert (sql);
	mu_expr_helpers_strlist_free (expr_strs);

	g_assert_cmpstr (sql,==,expected);
	
	mu_expr_list_destroy (exprs, FALSE);
	g_free (sql);
}
static void
test_sql_case_5 (void)
{
	char *sql;
	GSList *expr_strs;
        MuExprList *exprs;
	const char *expr="s:=gnu f:^richard F:N";
        const char *expected=
	"SELECT   m.recipients, m.cc, m.sender, m.subject, m.msg_id, \n"
	"         NULL AS dummy2, m.flags, m.mdate, m.msize, m.mpath, \n"
	"         m.priority, m.tstamp, m.id\n"
	"FROM     message m\n"
	"WHERE    m.subject = 'gnu'\n"
	"AND      m.sender NOT LIKE '%richard%'\n"
	"AND      m.flags & 2 = 2\n"
	"ORDER BY m.mdate DESC;\n";

	expr_strs = mu_expr_helpers_strlist_from_str (expr,NULL); g_assert (expr_strs);
	exprs = mu_expr_list_new     (expr_strs, NULL);  g_assert (exprs);
	sql   = mu_expr_sql_generate (exprs,NULL,"d",FALSE); g_assert (sql);
	mu_expr_helpers_strlist_free (expr_strs);

	g_assert_cmpstr (sql,==,expected);
	
	mu_expr_list_destroy (exprs, FALSE);
	g_free (sql);
}
static void
test_sql_case_6 (void)
{
	char *sql;
	GSList *expr_strs;
        MuExprList *exprs;
	const char *expr="z:1M-2M";
        const char *expected=
	"SELECT   m.recipients, m.cc, m.sender, m.subject, m.msg_id, \n"
	"         NULL AS dummy2, m.flags, m.mdate, m.msize, m.mpath, \n"
	"         m.priority, m.tstamp, m.id\n"
	"FROM     message m\n"
	"WHERE    m.msize >= 1000000 AND m.msize <= 2000000\n"
	"ORDER BY m.mdate DESC;\n";

	expr_strs = mu_expr_helpers_strlist_from_str (expr,NULL); g_assert (expr_strs);
	exprs = mu_expr_list_new     (expr_strs, NULL);  g_assert (exprs);
	sql   = mu_expr_sql_generate (exprs,NULL,"d",FALSE); g_assert (sql);
	mu_expr_helpers_strlist_free (expr_strs);

	g_assert_cmpstr (sql,==,expected);
	
	mu_expr_list_destroy (exprs, FALSE);
	g_free (sql);
}
static void
test_sql_case_7 (void)
{
	char *sql;
	GSList *expr_strs;
        MuExprList *exprs;
	const char *expr="z:2k-";
        const char *expected=
	"SELECT   m.recipients, m.cc, m.sender, m.subject, m.msg_id, \n"
	"         NULL AS dummy2, m.flags, m.mdate, m.msize, m.mpath, \n"
	"         m.priority, m.tstamp, m.id\n"
	"FROM     message m\n"
	"WHERE    m.msize >= 2000\n"
	"ORDER BY m.mdate DESC;\n";

	expr_strs = mu_expr_helpers_strlist_from_str (expr,NULL); g_assert (expr_strs);
	exprs = mu_expr_list_new     (expr_strs, NULL);  g_assert (exprs);
	sql   = mu_expr_sql_generate (exprs,NULL,"d",FALSE); g_assert (sql);
	mu_expr_helpers_strlist_free (expr_strs);

	g_assert_cmpstr (sql,==,expected);
	
	mu_expr_list_destroy (exprs, FALSE);
	g_free (sql);
}
static void
test_sql_case_8 (void)
{
	char *sql;
	GSList *expr_strs;
        MuExprList *exprs;
	const char *expr="z:-1000000";
        const char *expected=
	"SELECT   m.recipients, m.cc, m.sender, m.subject, m.msg_id, \n"
	"         NULL AS dummy2, m.flags, m.mdate, m.msize, m.mpath, \n"
	"         m.priority, m.tstamp, m.id\n"
	"FROM     message m\n"
	"WHERE    m.msize <= 1000000\n"
	"ORDER BY m.mdate DESC;\n";

	expr_strs = mu_expr_helpers_strlist_from_str (expr,NULL); g_assert (expr_strs);
	exprs = mu_expr_list_new     (expr_strs, NULL);  g_assert (exprs);
	sql   = mu_expr_sql_generate (exprs,NULL,"d",FALSE); g_assert (sql);
	mu_expr_helpers_strlist_free (expr_strs);

	g_assert_cmpstr (sql,==,expected);
	
	mu_expr_list_destroy (exprs, FALSE);
	g_free (sql);
}
static void
test_sql_case_9 (void)
{
	char *sql;
	GSList *expr_strs;
        MuExprList *exprs;
	const char *expr="d:-20050101";
        const char *expected=
	"SELECT   m.recipients, m.cc, m.sender, m.subject, m.msg_id, \n"
	"         NULL AS dummy2, m.flags, m.mdate, m.msize, m.mpath, \n"
	"         m.priority, m.tstamp, m.id\n"
	"FROM     message m\n"
	"WHERE    m.mdate <= 1104616799\n"
	"ORDER BY m.mdate DESC;\n";

	expr_strs = mu_expr_helpers_strlist_from_str (expr,NULL); g_assert (expr_strs);
	exprs = mu_expr_list_new     (expr_strs, NULL);  g_assert (exprs);
	sql   = mu_expr_sql_generate (exprs,NULL,"d",FALSE); g_assert (sql);
	mu_expr_helpers_strlist_free (expr_strs);

	g_assert_cmpstr (sql,==,expected);
	
	mu_expr_list_destroy (exprs, FALSE);
	g_free (sql);
}
static void
test_sql_case_10 (void)
{
	char *sql;
	GSList *expr_strs;
        MuExprList *exprs;
	const char *expr="d:20060101-20061231";
        const char *expected=
	"SELECT   m.recipients, m.cc, m.sender, m.subject, m.msg_id, \n"
	"         NULL AS dummy2, m.flags, m.mdate, m.msize, m.mpath, \n"
	"         m.priority, m.tstamp, m.id\n"
	"FROM     message m\n"
	"WHERE    m.mdate >= 1136066400 AND m.mdate <= 1167602399\n"
	"ORDER BY m.mdate DESC;\n";

	expr_strs = mu_expr_helpers_strlist_from_str (expr,NULL); g_assert (expr_strs);
	exprs = mu_expr_list_new     (expr_strs, NULL);  g_assert (exprs);
	sql   = mu_expr_sql_generate (exprs,NULL,"d",FALSE); g_assert (sql);
	mu_expr_helpers_strlist_free (expr_strs);

	g_assert_cmpstr (sql,==,expected);
	
	mu_expr_list_destroy (exprs, FALSE);
	g_free (sql);
}
static void
test_sql_case_11 (void)
{
	char *sql;
	GSList *expr_strs;
        MuExprList *exprs;
	const char *expr="d:20070602-";
        const char *expected=
	"SELECT   m.recipients, m.cc, m.sender, m.subject, m.msg_id, \n"
	"         NULL AS dummy2, m.flags, m.mdate, m.msize, m.mpath, \n"
	"         m.priority, m.tstamp, m.id\n"
	"FROM     message m\n"
	"WHERE    m.mdate >= 1180731600\n"
	"ORDER BY m.mdate DESC;\n";

	expr_strs = mu_expr_helpers_strlist_from_str (expr,NULL); g_assert (expr_strs);
	exprs = mu_expr_list_new     (expr_strs, NULL);  g_assert (exprs);
	sql   = mu_expr_sql_generate (exprs,NULL,"d",FALSE); g_assert (sql);
	mu_expr_helpers_strlist_free (expr_strs);

	g_assert_cmpstr (sql,==,expected);
	
	mu_expr_list_destroy (exprs, FALSE);
	g_free (sql);
}
static void
test_sql_case_12 (void)
{
	char *sql;
	GSList *expr_strs;
        MuExprList *exprs;
	const char *expr="P:=/home/user/Maildir/secret F:T";
        const char *expected=
	"SELECT   m.recipients, m.cc, m.sender, m.subject, m.msg_id, \n"
	"         NULL AS dummy2, m.flags, m.mdate, m.msize, m.mpath, \n"
	"         m.priority, m.tstamp, m.id\n"
	"FROM     message m\n"
	"WHERE    m.mpath = '/home/user/Maildir/secret'\n"
	"AND      m.flags & 64 = 64\n"
	"ORDER BY m.mdate DESC;\n";

	expr_strs = mu_expr_helpers_strlist_from_str (expr,NULL); g_assert (expr_strs);
	exprs = mu_expr_list_new     (expr_strs, NULL);  g_assert (exprs);
	sql   = mu_expr_sql_generate (exprs,NULL,"d",FALSE); g_assert (sql);
	mu_expr_helpers_strlist_free (expr_strs);

	g_assert_cmpstr (sql,==,expected);
	
	mu_expr_list_destroy (exprs, FALSE);
	g_free (sql);
}
static void
test_sql_case_13 (void)
{
	char *sql;
	GSList *expr_strs;
        MuExprList *exprs;
	const char *expr="p:LNH";
        const char *expected=
	"SELECT   m.recipients, m.cc, m.sender, m.subject, m.msg_id, \n"
	"         NULL AS dummy2, m.flags, m.mdate, m.msize, m.mpath, \n"
	"         m.priority, m.tstamp, m.id\n"
	"FROM     message m\n"
	"WHERE    m.priority & 7 = 7\n"
	"ORDER BY m.mdate DESC;\n";

	expr_strs = mu_expr_helpers_strlist_from_str (expr,NULL); g_assert (expr_strs);
	exprs = mu_expr_list_new     (expr_strs, NULL);  g_assert (exprs);
	sql   = mu_expr_sql_generate (exprs,NULL,"d",FALSE); g_assert (sql);
	mu_expr_helpers_strlist_free (expr_strs);

	g_assert_cmpstr (sql,==,expected);
	
	mu_expr_list_destroy (exprs, FALSE);
	g_free (sql);
}
static void
test_sql_case_14 (void)
{
	char *sql;
	GSList *expr_strs;
        MuExprList *exprs;
	const char *expr="p:L";
        const char *expected=
	"SELECT   m.recipients, m.cc, m.sender, m.subject, m.msg_id, \n"
	"         NULL AS dummy2, m.flags, m.mdate, m.msize, m.mpath, \n"
	"         m.priority, m.tstamp, m.id\n"
	"FROM     message m\n"
	"WHERE    m.priority & 1 = 1\n"
	"ORDER BY m.mdate DESC;\n";

	expr_strs = mu_expr_helpers_strlist_from_str (expr,NULL); g_assert (expr_strs);
	exprs = mu_expr_list_new     (expr_strs, NULL);  g_assert (exprs);
	sql   = mu_expr_sql_generate (exprs,NULL,"d",FALSE); g_assert (sql);
	mu_expr_helpers_strlist_free (expr_strs);

	g_assert_cmpstr (sql,==,expected);
	
	mu_expr_list_destroy (exprs, FALSE);
	g_free (sql);
}

int
main (int argc, char *argv[])
{
	g_test_init (&argc, &argv, NULL);
	g_test_add_func ("/find/sql/test_sql_case_1", test_sql_case_1);
	g_test_add_func ("/find/sql/test_sql_case_2", test_sql_case_2);
	g_test_add_func ("/find/sql/test_sql_case_3", test_sql_case_3);
	g_test_add_func ("/find/sql/test_sql_case_4", test_sql_case_4);
	g_test_add_func ("/find/sql/test_sql_case_5", test_sql_case_5);
	g_test_add_func ("/find/sql/test_sql_case_6", test_sql_case_6);
	g_test_add_func ("/find/sql/test_sql_case_7", test_sql_case_7);
	g_test_add_func ("/find/sql/test_sql_case_8", test_sql_case_8);
	g_test_add_func ("/find/sql/test_sql_case_9", test_sql_case_9);
	g_test_add_func ("/find/sql/test_sql_case_10", test_sql_case_10);
	g_test_add_func ("/find/sql/test_sql_case_11", test_sql_case_11);
	g_test_add_func ("/find/sql/test_sql_case_12", test_sql_case_12);
	g_test_add_func ("/find/sql/test_sql_case_13", test_sql_case_13);
	g_test_add_func ("/find/sql/test_sql_case_14", test_sql_case_14);
        return g_test_run ();
}

