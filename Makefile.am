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

SUBDIRS=			\
	mu			\
	app			\
	path			\
	msg			\
	find			\
	index			\
	man			\
	doc	

if MU_HAVE_GTEST
SUBDIRS += tests
endif

if MU_BUILD_GTK
SUBDIRS += gtk
endif 

EXTRA_DIST=			\
	AUTHORS			\
	COPYING			\
	ChangeLog		\
	INSTALL			\
	NEWS			\
	README			\
	TODO			\
	autogen.sh

# so we can say 'make test'
check: test

tags:
	gtags

cc10:
	@pmccabe `find -name '*.c'` | sort -nr | awk '($$1 > 10)'

cc20:
	@pmccabe `find -name '*.c'` | sort -nr | awk '($$1 > 20)'
