/* 
** Copyright (C) 2008 Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>
**
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 3, or (at your option) any
** later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software Foundation,
** Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  
**  
*/

#ifndef __MU_APP_PRIV_H__
#define __MU_APP_PRIV_H__

/* for use in test cases only */

/* 
 * this is the last component of the mu-dir;
 * by default the full path would be 
 * <homedir><MU_DIR>, or "~/.mu"
 */
#define MU_HOME_DIR ".mu"

/*
 * name of the mu meta-database; this is an SQLite3 database
 */
#define MU_SQLITE_DB_NAME  "mu-sqlite.db-" MU_DATABASE_VERSION

/*
 * name of the mu meta-database; this is a Xapian database.
 * there should be no reason for versioning
 */
#define MU_XAPIAN_DB_NAME  "mu-xapian.db-" MU_DATABASE_VERSION

/*
 * basename of the default bookmarks keyfile
 * (the full path, by default, would be ~/.mu/mu-bookmarks.conf)
 */
#define MU_EXPR_BOOKMARKS_NAME "mu-bookmarks.conf"

#endif /*__MU_APP_PRIV_H__*/
