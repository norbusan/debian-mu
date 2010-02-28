#!/usr/bin/perl -w

## Copyright (C) 2008 Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>
##
## This program is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by the
## Free Software Foundation; either version 3, or (at your option) any
## later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software Foundation,
## Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  

# this is perl script to generate the SQL unit tests for mu-find; it uses
# output of mu-find to generate the test cases... the idea is that we create
# the test cases once, make sure they are correct, and test later releases
# against the verified-correct output. Reason for automizing this is that
# it's a lot of work to manually update all the cases, e.g., when there are
# small layout changes in the generated sql. 

use strict;

sub generate_test_func($$);

die "can't find ../mu-find"
  unless (-x "../mu-find");

# read testcase from test-cases.txt
my @testcases;
open(CASES,"<sql-test-cases.txt");
while(<CASES>) { 
    next if ((/^#/) || (/^\s*$/));
    push @testcases, $_ if (chomp $_);
}
close(CASES);

sub generate_test_func($$){
    my ($name, $expr) = @_;
    my $txt;

$txt=<<EOF;
static void
$name (void)
{
	char *sql;
	GSList *expr_strs;
        MuExprList *exprs;
	const char *expr=\"$expr\";
        const char *expected=
EOF
   
    my $sql =`../mu-find -o s $expr`;
    chomp($sql);
    $sql =~ s/"/\\"/mg;
    $sql =~ s/^/\t"/mg;
    $sql =~ s/$/\\n"/mg;
    
$txt.=<<EOF;
$sql;

	expr_strs = mu_expr_helpers_strlist_from_str (expr,NULL); g_assert (expr_strs);
	exprs = mu_expr_list_new     (expr_strs, NULL);  g_assert (exprs);
	sql   = mu_expr_sql_generate (exprs,NULL,"d",FALSE); g_assert (sql);
	mu_expr_helpers_strlist_free (expr_strs);

	g_assert_cmpstr (sql,==,expected);
	
	mu_expr_list_destroy (exprs, FALSE);
	g_free (sql);
}
EOF
    return $txt;
}



my $filedata=<<EOF;
/* 
** Copyright (C) 2008 Dirk-Jan C. Binnema <djcb\@djcbsoftware.nl>
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

\#include <glib.h>

\#include "find/mu-expr.h"
\#include "find/mu-expr-sql.h"
\#include "find/mu-expr-helpers.h"

EOF



open(OUTPUT, '>test-sql.c') 
  or die "can't open test-sql.c: $!";
print OUTPUT $filedata;

my $i=1;
foreach my $case (@testcases) {
    my $testname = "test_sql_case_" . $i++;  
    print OUTPUT generate_test_func ($testname,$case);
}

my $maindata=<<EOF;

int
main (int argc, char *argv[])
{
	g_test_init (&argc, &argv, NULL);
EOF

$i=1;
foreach my $case (@testcases) {
    my $testname = "test_sql_case_" . $i++;  
    $maindata .= "\tg_test_add_func (\"/find/sql/$testname\", $testname);\n"; 
}

$maindata.=<<EOF;
        return g_test_run ();
}

EOF

print OUTPUT $maindata;

close OUTPUT;

exit 0;




