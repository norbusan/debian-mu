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
#include <errno.h>
#include <string.h>

#include "mu/mu.h"
#include "app/mu-app.h"
#include "mu-path.h"


struct _MkdirOptions {
	int _mode;
};
typedef struct _MkdirOptions MkdirOptions;

int
main (int argc, char *argv[])
{
	int retval = 1;

	if (!mu_app_init (&argc, &argv, "mu-mkmdir")) {
		g_printerr ("failed to init mu\n");
		return 1;
	}
	
	MkdirOptions opts;
	memset (&opts, 0, sizeof(MkdirOptions));

	do { 
		GError *err = NULL;

		GOptionEntry entries [] = {
			{"mode", 'm', 0, G_OPTION_ARG_INT, &opts._mode},
			{NULL}
		};

		if (!mu_conf_handle_options (mu_app_conf(), entries, &argc, &argv, 
					     &err)) {
			g_printerr ("option parsing failed: %s\n", 
				    (err && err->message) ? err->message : "?" );
			if (err)
				g_error_free (err);
			break;
		}

		if (argc < 2) {
			g_printerr ("error: expected: directory name\n");
			retval = 1;
			break;
		}

		if (!opts._mode)
			opts._mode = 0755;
		
		if (!mu_path_make_maildir (argv[1], opts._mode)) {
			g_printerr ("error: failed to create maildir: %s\n",
				    strerror (errno));
			break;
		}
		
		retval = 0;

	} while (0);

	mu_app_uninit ();
	return retval;
}
