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
	$(MU_GLIB2_CFLAGS)			\
	-DABS_SRCDIR=\"${abs_srcdir}\"		

noinst_PROGRAMS= $(TEST_PROGS) 

TEST_PROGS += test-path
test_path_SOURCES = test-path.c
test_path_LDADD=				\
	$(MU_GLIB2_LIBS)			\
	${top_srcdir}/path/libmupath.la		

TEST_PROGS += test-walk
test_walk_SOURCES = test-walk.c
test_walk_LDADD=				\
	$(MU_GLIB2_LIBS)			\
	${top_srcdir}/path/libmupath.la


# note the question marks; make does not like files with ':'...
EXTRA_DIST=						\
	TestMaildir/cur/cur0002.server?2,S		\
	TestMaildir/cur/cur0003.server?2,S		\
	TestMaildir/subdir/new/subdirnew004.server	\
	TestMaildir/subdir/cur/subdirr0005.server?2,S   \
	TestMaildir/new/new0001.server			\
	TestMaildir/tmp/tmp0005				\
	TestMaildir/.dotdir/new/new006

