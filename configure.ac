## Copyright (C) 2008 Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>
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

AC_INIT([mu],[0.4],[http://www.djcbsoftware.nl/code/mu])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([app/mu-app.c])
AM_INIT_AUTOMAKE([dist-bzip2])

# we set the set the version of the metadata database schema here;
# it will become part of the db name, so we can automtically 
# recreate the database when we incompatible changes.
#
# the Xapian and SQLite database share this number, so when
# we upgrade, we can simply update all.
#
# note that MU_DATABASE_VERSION does not necessarily follow
# MU versioning.
#
AC_DEFINE(MU_DATABASE_VERSION,["0.4"], [Schema version of the databases])

AC_PROG_LIBTOOL

if test x$prefix = xNONE; then
   prefix=/usr/local
fi
AC_SUBST(prefix)

AC_PROG_CC
AC_PROG_CXX
AM_PROG_CC_STDC
AC_HEADER_STDC

# have pkg-config?
AC_PATH_PROG([PKG_CONFIG], [pkg-config], [no])
if test "x$PKG_CONFIG" = "xno"; then
   AC_MSG_ERROR([
   *** The pkg-config script could not be found. Please make sure it is
   *** in your path, or set the PKG_CONFIG environment variable
   *** to the full path to pkg-config.])
fi


# sqlite3?
PKG_CHECK_MODULES(MU_SQLITE3,sqlite3)
AC_SUBST(MU_SQLITE3_CFLAGS)
AC_SUBST(MU_SQLITE3_LIBS)

# glib2?
PKG_CHECK_MODULES(MU_GLIB2,glib-2.0)
AC_SUBST(MU_GLIB2_CFLAGS)
AC_SUBST(MU_GLIB2_LIBS)

# g_test was introduced in glib 2.16
PKG_CHECK_MODULES(g_test,glib-2.0 >= 2.16,
		  [have_gtest=yes],[have_gtest=no])
AM_CONDITIONAL(MU_HAVE_GTEST, test "x$have_gtest" = "xyes")
if test "x$have_gtest" = "xno"; then
   AC_MSG_WARN([You need GLIB version >= 2.16 to build the unit tests])
fi

# gmime2?	
PKG_CHECK_MODULES(MU_GMIME2,gmime-2.0)
AC_SUBST(MU_GMIME2_CFLAGS)
AC_SUBST(MU_GMIME2_LIBS)

dnl PKG_CHECK_MODULES(dummy,gmime-2.4,[have_gmime24=yes],[have_gmime24=no])
dnl if test "x$have_gmime24" = "xyes"; then
dnl    PKG_CHECK_MODULES(MU_GMIME2,gmime-2.4)
dnl    AC_SUBST(MU_GMIME2_CFLAGS)
dnl    AC_SUBST(MU_GMIME2_LIBS)
dnl    AC_DEFINE([MU_HAVE_GMIME24],[1],[whether we have gmime 2.4])
dnl else
dnl    PKG_CHECK_MODULES(MU_GMIME2,gmime-2.0)
dnl    AC_SUBST(MU_GMIME2_CFLAGS)
dnl    AC_SUBST(MU_GMIME2_LIBS)
dnl fi

# xapian?
AC_CHECK_PROG(XAPIAN,xapian-config,xapian-config,no)
AM_CONDITIONAL(MU_HAVE_XAPIAN,test "x$XAPIAN" != "xno")
if test "x$XAPIAN" = "xno"; then
   AC_MSG_WARN([cannot find Xapian (xapian-config); body search disabled])
   have_xapian="no"
else
   MU_XAPIAN_CXXFLAGS=`$XAPIAN --cxxflags`
   MU_XAPIAN_LIBS=`$XAPIAN --libs`		
   have_xapian="yes"
   AC_DEFINE(MU_HAVE_XAPIAN,[1],[Whether we have Xapian])
fi
AC_SUBST(MU_XAPIAN_CXXFLAGS)
AC_SUBST(MU_XAPIAN_LIBS)


# build GUI stuff?
dnl AC_PROG_VALAC([0.3.5])			      
AC_ARG_ENABLE(gtk,
              [AC_HELP_STRING([--enable-gtk],[Build GTK-based tools (default=no)])],
              [with_gtk=yes], [with_gtk=no])
if test "x$with_gtk" = "xyes"; then	      		      
   PKG_CHECK_MODULES(MU_GTK,gtk+-2.0 gobject-2.0)
   AC_SUBST(MU_GTK_LIBS)
   AC_SUBST(MU_GTK_CFLAGS)
fi
AM_CONDITIONAL([MU_BUILD_GTK],[test "x$with_gtk" = "xyes"])

AC_OUTPUT([
Makefile
mu/Makefile
mu/tests/Makefile
app/Makefile
app/tests/Makefile
index/Makefile
index/tests/Makefile
msg/Makefile
msg/tests/Makefile
find/Makefile
find/tests/Makefile
man/Makefile
path/Makefile
path/tests/Makefile
gtk/Makefile
gtk/index/Makefile
gtk/msgview/Makefile
gtk/widgets/Makefile
gtk/widgets/msgview/Makefile
gtk/widgets/indexrunner/Makefile
tests/Makefile
doc/Makefile
])


dnl---------------------------------
echo
echo "configure results for mu"
echo "========================"
echo "prefix              : $prefix" 
echo "unit tests          : $have_gtest"
echo "xapian              : $have_xapian"
echo "gtk+                : $with_gtk"
dnl echo -n "gmime2 api          : "
dnl if test "x$have_gmime24" = "xyes"; then
dnl    echo "2.4"
dnl else
dnl    echo "2.2"
dnl fi
if test "xhave_xapian" = "xno"; then
   echo "* Note, you'll need to install Xapian to enable search mail contents"
fi
echo
echo "now, type 'make' to build mu, or 'make test' to run the test suite"
dnl--------------------------------
