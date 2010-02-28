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

#include <glib.h>
#include <stdlib.h>

#include "mu-util.h"

char*
mu_util_homedir_expand (const char *path)
{
	const char* home;
	
	if (!path)
		return NULL;
	
	if (path[0] != '~' || path[1] != G_DIR_SEPARATOR)
		return g_strdup (path);

	home = getenv ("HOME");
	if (!home)
		home = g_get_home_dir ();
	
	return g_strdup_printf ("%s%s", home, path + 1);
}

