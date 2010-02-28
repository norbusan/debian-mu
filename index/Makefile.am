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

BUILT_SOURCES= mu-storage.sql.h

mu-storage.sql.h: mu-storage.sql
	awk '{print "\"" $$0  "\\n\"" }'  < $<  > $@

noinst_LTLIBRARIES =			\
	libmuindex.la			\
	libmuindexapp.la

libmuindex_la_SOURCES =			\
	mu-index.c			\
	mu-index.h             	 	\
	mu-index-app.h			\
	mu-index-app.c			\
	mu-storage-sqlite-helpers.c	\
	mu-storage-sqlite-init.c	\
	mu-storage-sqlite-priv.h	\
	mu-storage-sqlite.c		\
	mu-storage-sqlite.h

if MU_HAVE_XAPIAN
libmuindex_la_SOURCES += 	\
	mu-storage-xapian.h 	\
	mu-storage-xapian.cc
endif

libmuindex_la_LIBADD=				\
	$(MU_SQLITE3_LIBS)			\
	$(MU_XAPIAN_LIBS)			\
	${top_srcdir}/msg/libmumsg.la		\
	${top_srcdir}/path/libmupath.la		\
	${top_srcdir}/app/libmuapp.la


libmuindexapp_la_SOURCES =			\
	mu-index-app.h				\
	mu-index-app.c

libmuindexapp_la_LIBADD=			\
	libmuindex.la

bin_PROGRAMS=					\
	mu-index

# dummy.cc is empty, just a trick to force c++ linking...
mu_index_SOURCES=				\
	main.c					\
	dummy.cc

mu_index_LDADD=					\
	libmuindex.la				\
	libmuindexapp.la

EXTRA_DIST=			\
	mu-storage.sql
