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

SUBDIRS= .

if MU_HAVE_GTEST
SUBDIRS += tests
endif

INCLUDES=			\
	-I ${top_srcdir}/	\
	$(MU_GLIB2_CFLAGS)	\
	$(MU_SQLITE3_CFLAGS)	\
	$(MU_XAPIAN_CXXFLAGS)


noinst_LTLIBRARIES=		\
	libmufind.la

libmufind_la_SOURCES=		\
	mu-expr.c		\
	mu-expr.h		\
	mu-expr-bookmarks.c	\
	mu-expr-bookmarks.h	\
	mu-expr-sql.h		\
	mu-expr-sql.c		\
	mu-expr-type.h		\
	mu-expr-error.h		\
	mu-expr-type.c		\
	mu-expr-crit.h		\
	mu-expr-crit.c		\
	mu-expr-val.c		\
	mu-expr-val.h		\
	mu-expr-helpers.c	\
	mu-expr-helpers.h	\
	mu-query-mgr.h		\
	mu-query-mgr.c          \
	mu-query-sqlite.h 	\
	mu-query-sqlite.c	\
	mu-msg-sqlite.c		\
	mu-msg-sqlite.h		\
	mu-msg-sqlite-priv.h	


# conditionally compile xapian
if MU_HAVE_XAPIAN
libmufind_la_SOURCES += 	\
	mu-query-xapian.h 	\
	mu-query-xapian.cc	
endif

libmufind_la_LIBADD=			\
	$(MU_GLIB2_LIBS)		\
	$(MU_XAPIAN_LIBS)		\
	$(MU_SQLITE3_LIBS)		\
	${top_srcdir}/mu/libmu.la

bin_PROGRAMS=			\
	mu-find

mu_find_SOURCES=		\
	main.c			\
	dummy.cc

mu_find_LDADD=				\
	libmufind.la			\
	${top_srcdir}/path/libmupath.la	\
	${top_srcdir}/app/libmuapp.la
