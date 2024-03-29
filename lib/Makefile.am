## Copyright (C) 2010-2021 Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>
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

# enforce compiling guile (optionally) first,then this dir first
# before descending into tests/
include $(top_srcdir)/gtest.mk

SUBDIRS= utils index

TESTDEFS=                                                       \
        -DMU_TESTMAILDIR=\"${abs_srcdir}/testdir\"              \
        -DMU_TESTMAILDIR2=\"${abs_srcdir}/testdir2\"            \
        -DMU_TESTMAILDIR4=\"${abs_srcdir}/testdir4\"            \
        -DABS_CURDIR=\"${abs_builddir}\"                        \
        -DABS_SRCDIR=\"${abs_srcdir}\"


AM_CFLAGS=                                                      \
        $(WARN_CFLAGS)                                          \
        $(GMIME_CFLAGS)                                         \
        $(XAPIAN_CFLAGS)                                        \
        $(GLIB_CFLAGS)                                          \
        $(GUILE_CFLAGS)                                         \
        $(ASAN_CFLAGS)                                          \
        $(CODE_COVERAGE_CFLAGS)                                 \
        $(TESTDEFS)                                             \
        -Wno-format-nonliteral                                  \
        -Wno-switch-enum                                        \
        -Wno-deprecated-declarations                            \
        -Wno-inline

AM_CXXFLAGS=                                                    \
        $(GMIME_CFLAGS)                                         \
        $(GLIB_CFLAGS)                                          \
        $(GUILE_CFLAGS)                                         \
        $(WARN_CXXFLAGS)                                        \
        $(XAPIAN_CXXFLAGS)                                      \
        $(ASAN_CXXFLAGS)                                        \
        $(CODE_COVERAGE_CFLAGS)                                 \
        $(TESTDEFS)

AM_CPPFLAGS=                                                    \
        $(CODE_COVERAGE_CPPFLAGS)

noinst_LTLIBRARIES=                                             \
        libmu.la

libmu_la_SOURCES=                                               \
        mu-bookmarks.cc                                         \
        mu-bookmarks.hh                                         \
        mu-contacts.cc                                          \
        mu-contacts.hh                                          \
        mu-data.hh                                              \
        mu-parser.cc                                            \
        mu-parser.hh                                            \
        mu-query.cc                                             \
        mu-query.hh                                             \
        mu-query-results.hh                                     \
        mu-query-match-deciders.cc                              \
        mu-query-match-deciders.hh                              \
        mu-query-threads.cc                                     \
        mu-query-threads.hh                                     \
        mu-runtime.cc                                           \
        mu-runtime.hh                                           \
        mu-script.cc                                            \
        mu-script.hh                                            \
        mu-server.cc                                            \
        mu-server.hh                                            \
        mu-store.cc                                             \
        mu-store.hh                                             \
        mu-tokenizer.cc                                         \
        mu-tokenizer.hh                                         \
        mu-tree.hh                                              \
        mu-xapian.cc                                            \
        mu-xapian.hh                                            \
        mu-maildir.cc                                           \
        mu-maildir.hh                                           \
        mu-flags.cc                                             \
        mu-flags.hh                                             \
        mu-msg-crypto.cc                                        \
        mu-msg-doc.cc                                           \
        mu-msg-doc.hh                                           \
        mu-msg-fields.c                                         \
        mu-msg-fields.h                                         \
        mu-msg-file.cc                                          \
        mu-msg-file.hh                                          \
        mu-msg-part.cc                                          \
        mu-msg-part.hh                                          \
        mu-msg-prio.c                                           \
        mu-msg-prio.h                                           \
        mu-msg-priv.hh                                          \
        mu-msg-sexp.cc                                          \
        mu-msg.cc                                               \
        mu-msg.hh

libmu_la_LIBADD=                                                \
        $(XAPIAN_LIBS)                                          \
        $(GMIME_LIBS)                                           \
        $(GLIB_LIBS)                                            \
        $(GUILE_LIBS)                                           \
        ${builddir}/index/libmu-index.la                        \
        $(CODE_COVERAGE_LIBS)

libmu_la_LDFLAGS=                                               \
        $(ASAN_LDFLAGS)

noinst_PROGRAMS=                                                \
        tokenize

tokenize_SOURCES=                                               \
        tokenize.cc

tokenize_LDADD=                                                 \
        $(WARN_LDFLAGS)                                         \
        libmu.la                                                \
        utils/libmu-utils.la

EXTRA_DIST=                                                     \
        doxyfile.in

noinst_PROGRAMS+=$(TEST_PROGS)

noinst_LTLIBRARIES+=                                            \
        libtestmucommon.la

TEST_PROGS += test-maildir
test_maildir_SOURCES= test-mu-maildir.cc
test_maildir_LDADD=  libtestmucommon.la

TEST_PROGS += test-msg-fields
test_msg_fields_SOURCES= test-mu-msg-fields.cc
test_msg_fields_LDADD=  libtestmucommon.la

TEST_PROGS += test-msg
test_msg_SOURCES= test-mu-msg.cc
test_msg_LDADD=  libtestmucommon.la

TEST_PROGS += test-store
test_store_SOURCES= test-mu-store.cc
test_store_LDADD= libtestmucommon.la

TEST_PROGS += test-query
test_query_SOURCES= test-query.cc
test_query_LDADD= libtestmucommon.la

TEST_PROGS += test-flags
test_flags_SOURCES= test-mu-flags.cc
test_flags_LDADD=  libtestmucommon.la

TEST_PROGS+=test-tokenizer
test_tokenizer_SOURCES=test-tokenizer.cc
test_tokenizer_LDADD=libtestmucommon.la

TEST_PROGS+=test-threads
test_threads_SOURCES=mu-query-threads.cc
test_threads_LDADD=libtestmucommon.la
test_threads_CXXFLAGS=$(AM_CXXFLAGS) -DBUILD_TESTS

TEST_PROGS += test-contacts
test_contacts_SOURCES= mu-contacts.cc
test_contacts_CXXFLAGS=$(AM_CXXFLAGS) -DBUILD_TESTS
test_contacts_LDADD= libtestmucommon.la

TEST_PROGS+=test-parser
test_parser_SOURCES=test-parser.cc
test_parser_LDADD=libtestmucommon.la

libtestmucommon_la_SOURCES=                                     \
        test-mu-common.cc                                       \
        test-mu-common.hh

libtestmucommon_la_LIBADD=                                      \
        libmu.la                                                \
        utils/libmu-utils.la

# note the question marks; make does not like files with ':', so we
# use the (also supported) version with '!' instead. We could escape
# the : with \: but automake does not recognize that....

# test messages, the '.ignore' message should be ignored
# when indexing
EXTRA_DIST+=                                                    \
        testdir/tmp/1220863087.12663.ignore                     \
        testdir/new/1220863087.12663_9.mindcrime                \
        testdir/new/1220863087.12663_25.mindcrime               \
        testdir/new/1220863087.12663_21.mindcrime               \
        testdir/new/1220863087.12663_23.mindcrime               \
        testdir/cur/1220863087.12663_5.mindcrime!2,S            \
        testdir/cur/1220863087.12663_7.mindcrime!2,RS           \
        testdir/cur/1220863087.12663_15.mindcrime!2,PS          \
        testdir/cur/1220863087.12663_19.mindcrime!2,S           \
        testdir/cur/1220863042.12663_1.mindcrime!2,S            \
        testdir/cur/1220863060.12663_3.mindcrime!2,S            \
        testdir/cur/1283599333.1840_11.cthulhu!2,               \
        testdir/cur/1305664394.2171_402.cthulhu!2,              \
        testdir/cur/1252168370_3.14675.cthulhu!2,S              \
        testdir/cur/encrypted!2,S                               \
        testdir/cur/multimime!2,FS                              \
        testdir/cur/signed!2,S                                  \
        testdir/cur/signed-encrypted!2,S                        \
        testdir/cur/special!2,Sabc                              \
        testdir/cur/multirecip!2,S                              \
        testdir2/bar/cur/mail1                                  \
        testdir2/bar/cur/mail2                                  \
        testdir2/bar/cur/mail3                                  \
        testdir2/bar/cur/mail4                                  \
        testdir2/bar/cur/mail5                                  \
        testdir2/bar/cur/181736.eml                             \
        testdir2/bar/cur/mail6                                  \
        testdir2/bar/tmp/.noindex                               \
        testdir2/bar/new/.noindex                               \
        testdir2/Foo/cur/mail5                                  \
        testdir2/Foo/cur/arto.eml                               \
        testdir2/Foo/cur/fraiche.eml                            \
        testdir2/Foo/tmp/.noindex                               \
        testdir2/Foo/new/.noindex                               \
        testdir2/wom_bat/cur/atomic                             \
        testdir2/wom_bat/cur/rfc822.1                           \
        testdir2/wom_bat/cur/rfc822.2                           \
        testdir4/1220863087.12663_19.mindcrime!2,S              \
        testdir4/1220863042.12663_1.mindcrime!2,S               \
        testdir4/1283599333.1840_11.cthulhu!2,                  \
        testdir4/1305664394.2171_402.cthulhu!2,                 \
        testdir4/1252168370_3.14675.cthulhu!2,S                 \
        testdir4/mail1                                          \
        testdir4/mail5                                          \
        testdir4/181736.eml                                     \
        testdir4/encrypted!2,S                                  \
        testdir4/multimime!2,FS                                 \
        testdir4/signed!2,S                                     \
        testdir4/signed-bad!2,S                                 \
        testdir4/signed-encrypted!2,S                           \
        testdir4/special!2,Sabc

TESTS=$(TEST_PROGS)

CLEANFILES=*.log *.trs *core* *vgdump* *.gcda *.gcno

include $(top_srcdir)/aminclude_static.am
