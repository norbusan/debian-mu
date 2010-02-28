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

#ifndef __MU_MU_CONF_H__
#define __MU_MU_CONF_H__

#include <glib.h>

struct _MuConf;
typedef struct _MuConf MuConf;

MuConf   *mu_conf_new         (const char* file, const char* group);
void      mu_conf_destroy     (MuConf *conf);

gboolean  mu_conf_get_bool     (MuConf *conf, const char* key, int *val);
gboolean  mu_conf_get_int      (MuConf *conf, const char* key, int *val);
gboolean  mu_conf_get_string   (MuConf *conf, const char* key, char **val);

gboolean  mu_conf_handle_options (MuConf *conf, GOptionEntry *entries,
				  int *argc, char ***argvp, GError **err);

/*
 * name of the mu configuration file.
 * by default in <homedir><MU-DIR>/<MU_CONF_NAME>, or
 * ~/.mu/mu.conf
 */
#define MU_CONF_NAME    "mu.conf"

#endif /*__MU_MU_CONF_H__*/
