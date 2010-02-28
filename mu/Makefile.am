## Copyright (C) 2008 Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##  
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##  
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

include $(top_srcdir)/gtest.mk

SUBDIRS=. 

if MU_HAVE_GTEST
SUBDIRS += tests
endif 

INCLUDES=${MU_GLIB2_CFLAGS}     \
	 $(MU_GMIME2_CFLAGS)

noinst_LTLIBRARIES=		\
	libmu.la

libmu_la_SOURCES=		\
	mu.h			\
	mu-msg-flags.c		\
	mu-msg-flags.h		\
	mu-msg-str.c		\
	mu-msg-str.h		\
	mu-msg.h

libmu_la_LIBADD=		\
	${MU_GLIB2_LIBS}
