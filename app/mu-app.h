/* 
** Copyright (C) 2008 Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
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

#ifndef __MU_APP_H__
#define __MU_APP_H__

#include "config.h"

#include <glib.h>
#include "mu-conf.h"

/** 
 * initialize the mu system; should only be called once
 * mu will interpret the arguments in argc,argv
 * 
 * @param argc pointer to argc argument of caller, not NULL
 * @param argv pointer to argv argument of caller, not NULL
 * @param appname name of the app; not NULL
 * 
 * @return TRUE if succesfull, FALSE otherwise
 */
gboolean mu_app_init   (int *argcp, char ***argvp, const char* appname);


/** 
 * unitialize mu; call when done with the mu system, so
 * resources can be freed
 */
void mu_app_uninit (void);


/** 
 * get the mu-home dir (default : ~/.mu)
 * 
 * @return the mu home dir, or NULL in case of error
 */
const char* mu_app_home         (void);


/** 
 * get the mu configurator
 * 
 * @return the mu configurator, or NULL in case of error
 */
MuConf *      mu_app_conf         (void);


/** 
 * get the path to the SQLite3 database file for metadata
 * 
 * @return path to the database file, or NULL in case of error
 */
const char* mu_app_sqlite_path (void);


/** 
 * get the path to the Xapian database file for content data
 * 
 * @return path to the database file, or NULL in case of error
 */
const char* mu_app_xapian_path (void);


/** 
 * get the path to the query expression bookmarks file
 * 
 * @return the path to the bookmarks file, or NULL in case of error 
 */
const char* mu_app_bookmarks_path (void);


/** 
 * get the option group, for adding to mu-using command-lines
 * 
 * @return the option group, or NULL in case of error
 */
GOptionGroup* mu_app_get_option_group (void);


/** 
 * get the MU version and copyright information
 * 
 * @return a string which contains version/copyright information.
 *         should not be freed.
 */
const gchar* mu_app_version (void);


#endif /*__MU_APP_H__*/
