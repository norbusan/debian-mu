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

#ifndef __MU_EXPR_HELPERS_H__
#define __MU_EXPR_HELPERS_H__
	
#include <glib.h>

/** 
 * take a char*[] and turn it into a GSList
 * 
 * @param argc numbers of strings
 * @param argv array of strings
 * 
 * @return a newly allocated GSList of the arguments; or NULL in case
 * of error. use mu_exprs_helpers_strlist_free when done with the list
 */	
GSList   *mu_expr_helpers_strlist_from_args    (int argc, char *argv[]);

/** 
 * take a string and turn it into a GSList of parts; the parts are
 * seperated by one or more spaces. You can include spaces by quoting
 * the string parts using "'", and quotes can be escaped with \
 * ie.:
 * "foo bar 'hello \'world'" => {"foo", "bar", "hello 'world"}
 * 
 * @param str a string 
 * @param and err ptr to receive error information
 * 
 * @return a newly allocated GSList or NULL in case
 * of error. use mu_exprs_helpers_strlist_free when done with the list
 */
GSList   *mu_expr_helpers_strlist_from_str     (const char* str, GError **err);


/** 
 * free a list of strings, as produced by mu_expr_helpers_strlist_from_args or
 * mu_expr_helpers_strlist_from_str
 *
 * @param lst a list or NULL
 */
void      mu_expr_helpers_strlist_free         (GSList *lst);


/** 
 * checks whether the sortfields are valid (see mu-find(1))
 * 
 * @param str a string with sortfields
 * 
 * @return TRUE if valid, FALSE otherwise
 */
gboolean  mu_expr_helpers_sortfields_valid     (const char *str);

#endif /*__MU_EXPR_HELPERS_H__*/
