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

include $(top_srcdir)/gtest.mk

INCLUDES=					\
	-I ${top_srcdir}/			\
	$(MU_GLIB2_CFLAGS)	

noinst_PROGRAMS= $(TEST_PROGS) 
TEST_PROGS += test-sql test-expr-helpers test-expr-bookmarks


test_sql_SOURCES = test-sql.c
test_sql_LDADD=					\
	$(MU_GLIB2_LIBS)			\
	${top_srcdir}/find/libmufind.la		

test_expr_helpers_SOURCES = test-expr-helpers.c		
test_expr_helpers_LDADD=				\
	$(MU_GLIB2_LIBS)				\
	${top_srcdir}/find/libmufind.la			

test_expr_bookmarks_SOURCES = test-expr-bookmarks.c		
test_expr_bookmarks_LDADD=				\
	$(MU_GLIB2_LIBS)				\
	${top_srcdir}/find/libmufind.la			


EXTRA_DIST=					\
	sql-test-cases.txt			\
	make-test-sql.pl			\
	bookmarks.test

