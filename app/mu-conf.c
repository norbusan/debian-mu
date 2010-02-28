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

#include <glib.h> /* for GKeyFile */
#include <stdlib.h>
#include <unistd.h>

#include "mu-app.h"

struct _MuConf {
	GKeyFile   *_keyfile;
	const char *_group;
};


MuConf*
mu_conf_new (const char* file, const char* group)
{
	MuConf *conf = NULL;
	GError *err = NULL;
	
	g_return_val_if_fail (group, NULL);
	
	conf = (MuConf*)malloc(sizeof(MuConf));
	if (!conf) 
		return NULL;

	conf->_group = group;
	conf->_keyfile = g_key_file_new ();
	if (!conf->_keyfile) {
		g_warning ("%s: can't create keyfile",__FUNCTION__);
		mu_conf_destroy (conf);
		return NULL;
	}
	
	/* it's OK if there is no file; it's handled as if there
	   were an empty file */
	if (access(file, R_OK) == 0)  {
		if (!g_key_file_load_from_file (conf->_keyfile, file, 
						G_KEY_FILE_NONE, &err)) {
			g_warning ("%s: can't load keyfile '%s': %s", 
				   __FUNCTION__, file, err->message);
			if (err)
				g_error_free (err);
			mu_conf_destroy (conf);
			return NULL;
		}
	}
	
	g_debug ("%s: created conf instance (%s, %s)",
		 __FUNCTION__, file, group);
	
	return conf;
}


void 
mu_conf_destroy (MuConf *conf)
{
	if (conf) {
		g_key_file_free (conf->_keyfile);
		free (conf);
		g_debug ("%s: destroyed conf instance",__FUNCTION__);
	}	
}


gboolean  
mu_conf_get_bool (MuConf *conf, const char* key, int* val)
{
	int value;
	GError *err = NULL;

	g_return_val_if_fail (conf, FALSE);
		
	value = g_key_file_get_boolean (conf->_keyfile, conf->_group, key, &err);
	if (err) {
		g_debug ("%s: failed to get bool %s", __FUNCTION__, key);
		g_error_free (err);
		return FALSE;
	}
	
	if (val)
		*val = value ? 1 : 0;
	
	return TRUE;
}



gboolean  
mu_conf_get_int (MuConf *conf, const char* key, int *val)
{
	int value;
	GError *err = NULL;

	g_return_val_if_fail (conf, FALSE);
		
	value = g_key_file_get_integer (conf->_keyfile, conf->_group, 
					key, &err);
	if (err) {
		g_debug ("%s: failed to get int %s", __FUNCTION__, key);
		g_error_free (err);
		return FALSE;
	}
		
	if (val)
		*val = value;
	
	return TRUE;

}


gboolean
mu_conf_get_string (MuConf *conf, const char* key, char **val)
{
	char* value;
	GError *err = NULL;

	g_return_val_if_fail (conf, FALSE);

	value = g_key_file_get_string (conf->_keyfile, conf->_group, key, &err);
	if (err) {
		g_debug ("%s: failed to get string %s: %s",
			   __FUNCTION__, key, err->message);
		g_error_free (err);
		return FALSE;
	}
	
	if (val)
		*val = value;
	
	return TRUE;
}


static void
get_options_from_conf (MuConf *conf, GOptionEntry* entries)
{
	int i;

	g_return_if_fail (conf);
	g_return_if_fail (entries && entries[0].long_name);
	
	i = 0;
	do {
		switch (entries[i].arg) {
		case G_OPTION_ARG_NONE:
			mu_conf_get_bool (conf, entries[i].long_name,
					  entries[i].arg_data);
			break;
		case G_OPTION_ARG_FILENAME:
		case G_OPTION_ARG_STRING:
			mu_conf_get_string (conf, entries[i].long_name,
					    entries[i].arg_data);
			break;
		case G_OPTION_ARG_INT:
			mu_conf_get_int (conf, entries[i].long_name,
					 entries[i].arg_data);
			break;
		default:
			g_return_if_reached ();
		}

	} while (entries[++i].long_name);
}


gboolean
mu_conf_handle_options (MuConf *conf, GOptionEntry *entries,
			int *argcp, char ***argvp, GError **err)
{
	GOptionContext *ocx;

	g_return_val_if_fail (conf, FALSE);
	g_return_val_if_fail (entries, FALSE);

	/* first we ask the config file */
	get_options_from_conf (conf, entries);
	
	/* now we check for possible overrides from cmdline */
	ocx = g_option_context_new (g_get_prgname());
	g_option_context_add_main_entries (ocx, entries, NULL);
	g_option_context_add_group (ocx, mu_app_get_option_group());

	if (!g_option_context_parse (ocx, argcp, argvp, err)) 
		return FALSE;
		
	g_option_context_free (ocx);

	return TRUE;
}



