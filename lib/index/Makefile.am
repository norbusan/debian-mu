## Copyright (C) 2020 Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or
## (at your option) any later version.
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

AM_CPPFLAGS=							\
	$(CODE_COVERAGE_CPPFLAGS)

AM_CXXFLAGS=							\
	$(WARN_CXXFLAGS)					\
	$(GLIB_CFLAGS)						\
	$(XAPIAN_CFLAGS)					\
	$(ASAN_CXXFLAGS)					\
	$(CODE_COVERAGE_CFLAGS)					\
	-I${top_srcdir}/lib

AM_LDFLAGS=							\
	$(ASAN_LDFLAGS)

noinst_LTLIBRARIES=						\
	libmu-index.la

libmu_index_la_SOURCES=						\
	mu-indexer.cc						\
	mu-indexer.hh						\
	mu-scanner.cc						\
	mu-scanner.hh

libmu_index_la_LIBADD=						\
	$(GLIB_LIBS)						\
	$(CODE_COVERAGE_LIBS)

include $(top_srcdir)/aminclude_static.am
