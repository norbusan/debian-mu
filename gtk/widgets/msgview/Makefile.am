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

INCLUDES = 				\
	-I$(top_srcdir) 		\
	$(MU_GTK_CFLAGS)

# if MU_HAVE_GTEST
# SUBDIRS += tests
# endif

noinst_PROGRAMS =				\
	mu-msg-text-view-test			\
	mu-msg-header-view-test			\
	mu-msg-view-test

mu_msg_text_view_test_SOURCES=			\
	mu-msg-text-view-test.c

mu_msg_text_view_test_LDADD=			\
	libmumsgview.la

mu_msg_header_view_test_SOURCES=		\
	mu-msg-header-view-test.c

mu_msg_header_view_test_LDADD=			\
	libmumsgview.la

mu_msg_view_test_SOURCES=			\
	mu-msg-view-test.c

mu_msg_view_test_LDADD=				\
	libmumsgview.la


noinst_LTLIBRARIES =				\
	libmumsgview.la

libmumsgview_la_SOURCES =			\
	mu-msg-text-view.h			\
	mu-msg-text-view.c			\
	mu-msg-header-view.c			\
	mu-msg-header-view.h			\
	mu-msg-view.h				\
	mu-msg-view.c

libmumsgview_la_LIBADD=				\
	$(MU_GTK_LIBS)				\
	$(top_srcdir)/msg/libmumsg.la

