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

TEST_PROGS += test-index-search
test_index_search_SOURCES = test-index-search.c
test_index_search_LDADD=			\
	$(MU_GLIB2_LIBS)			\
	${top_srcdir}/index/libmuindex.la	\
	${top_srcdir}/find/libmufind.la		

# note the question marks; make does not like files with ':'...
EXTRA_DIST=							\
	TestMaildir/cur/1220871554.476_19.wintersun?2,S		\
	TestMaildir/cur/1220871559.476_21.wintersun?2,S		\
	TestMaildir/cur/1220871561.476_23.wintersun?2,S		\
	TestMaildir/cur/1220871569.476_25.wintersun?2,S		\
	TestMaildir/cur/1220871581.476_27.wintersun?2,S		\
	TestMaildir/cur/1220871581.476_29.wintersun?2,S		\
	TestMaildir/cur/1220871581.476_31.wintersun?2,S		\
	TestMaildir/cur/1220871581.476_37.wintersun?2,S		\
	TestMaildir/cur/1220871589.476_39.wintersun?2,S		\
	TestMaildir/cur/1220871600.476_41.wintersun?2,S		\
	TestMaildir/cur/1220903861.18268_3.mindcrime?2,S	\
	TestMaildir/new/1220871502.476_5.wintersun		\
	TestMaildir/new/1220871527.476_11.wintersun		\
	TestMaildir/new/1220871527.476_13.wintersun		\
	TestMaildir/new/1220871527.476_9.wintersun		\
	TestMaildir/new/1220871533.476_15.wintersun		\
	TestMaildir/new/1220871541.476_17.wintersun		\
	TestMaildir/new/1220871600.476_43.wintersun		\
	TestMaildir/tmp/1220871485.476_1.wintersun		\
	TestMaildir/tmp/1220871494.476_3.wintersun		
