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

INCLUDES= $(MU_GMIME2_CFLAGS)	

noinst_LTLIBRARIES=		\
	libmumsg.la		

libmumsg_la_SOURCES=		\
	mu-msg-gmime.h		\
	mu-msg-gmime.c		\
	mu-msg-gmime-init.h	\
	mu-msg-gmime-init.c	

libmumsg_la_LIBADD=				\
	${top_srcdir}/mu/libmu.la		\
	${top_srcdir}/path/libmupath.la		\
	$(MU_GMIME2_LIBS)	

bin_PROGRAMS=				\
	mu-msginfo

mu_msginfo_SOURCES=			\
	main.c			

mu_msginfo_LDADD=			\
	libmumsg.la			\
	${top_srcdir}/app/libmuapp.la			
