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

#define _XOPEN_SOURCE /* for fileno() */
#include <stdio.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <alloca.h>

#include <glib.h>
#include <glib-object.h>

#include "mu-app.h"
#include "mu-app-priv.h"
#include "mu-log.h"

/* globals we use for this mu singleton */
static struct {
	int     _init;
	char*   _home;
	MuConf* _conf;
	MuLog*  _log;
	guint   _log_handler_id;

	GOptionGroup *_option_group;

	gboolean _version;
	gboolean _debug;
	gboolean _log_stderr;
	gboolean _log_append;

	char* _sqlite_path;
	char* _xapian_path;
	char* _bookmarks_path;
} MU_APP_DATA;


static gboolean
mu_app_initialized (void)
{
	return MU_APP_DATA._init;
}


GOptionGroup*
mu_app_get_option_group (void)
{
	GOptionEntry options[] = {
		{"version", 'v', 0, G_OPTION_ARG_NONE, &MU_APP_DATA._version,
		 "display version and copyright information"},
		 {"home", 'h', 0, G_OPTION_ARG_FILENAME, &MU_APP_DATA._home,
		 "mu home directory"},
		{"debug", 'd', 0, G_OPTION_ARG_NONE, &MU_APP_DATA._debug,
		 "run in debug mode"},
		{"log-stderr", 's', 0, G_OPTION_ARG_NONE, &MU_APP_DATA._log_stderr,
		 "log to standard error"},
		{"log-append", 'a', 0, G_OPTION_ARG_NONE, &MU_APP_DATA._log_append,
		 "append to the current logfile (instead of overwriting it)"},
		{NULL}
	};

	GOptionGroup *og;

	g_return_val_if_fail (mu_app_initialized(), NULL);

	og = g_option_group_new ("mu",
				 "General options:",
				 "mu help description",
				 NULL, NULL);

	g_option_group_add_entries (og, options);

	return og;
}


/*
 * create the mu-homedir if it does not already exist; if it exists,
 * make sure it has the right properties; return 0 if OK, -1 otherwise
 * will print to stderr, because there may be nothing else...
 *
 * obviously, things might still fail later if something changes after
 * this, but then we get errors there; here we can catch 'm early
 */
static int
create_home_maybe (const char* home)
{
	if (access (home, F_OK) == 0) { /* is it existing? */
		struct stat statbuf;
		if (stat (home, &statbuf) != 0) {
			fprintf (stderr, "error: stat of '%s' failed: %s\n",
				 home, strerror(errno));
			return -1;
		}

		if (!S_ISDIR(statbuf.st_mode)) {
			fprintf (stderr, "error: '%s' is not a directory\n",
				 home);
			return -1;
		}

		if (statbuf.st_uid != getuid()) {
			fprintf (stderr, "error: user must own '%s'\n", home);
			return -1;
		}

		if ((statbuf.st_mode & 0777) != 0700) {
			fprintf (stderr,
				 "error: '%s' must have 0700 permissions\n",
				 home);
			return -1;
		}

	} else { /* no; try to create it instead */
		if (mkdir (home, 0700) != 0) {
			fprintf (stderr, "error: cannot create '%s': %s\n",
				 home, strerror(errno));
			return -1;
		}
	}
	return 0; /* all looks ok */
}


static char*
default_mu_app_home (void)
{
	const char* home;

	home = getenv ("HOME");
	if (!home)
		home = g_get_home_dir ();
	if (!home) {
		g_warning ("no home dir defined in HOME or /etc/passwd");
		return NULL;
	}

	return g_strdup_printf ("%s" G_DIR_SEPARATOR_S MU_HOME_DIR, home);
}

static MuConf*
init_conf (void)
{
	MuConf *conf;
	char *confpath;
	confpath = g_strdup_printf ("%s/" MU_CONF_NAME, mu_app_home());

	conf = mu_conf_new (confpath, g_get_prgname());
	g_free (confpath);

	return conf;
}

struct _LogHandlerData {
	MuLog   *_log;
	gboolean _debug;
};
typedef struct _LogHandlerData LogHandlerData;


static void
log_handler (const gchar* log_domain, GLogLevelFlags log_level,
	     const gchar* msg, LogHandlerData *data)
{
	if (log_level == G_LOG_LEVEL_DEBUG && !data->_debug)
	 	return;

	mu_log_write (data->_log, log_domain ? log_domain : "mu",
		      log_level, msg);
}


static void
silence (void) { return; }

static gboolean
init_log_silence ()
{
	MU_APP_DATA._log_handler_id =
		g_log_set_handler (NULL,
				   G_LOG_LEVEL_DEBUG|
				   G_LOG_LEVEL_MESSAGE|
				   G_LOG_LEVEL_INFO,
				   (GLogFunc)silence, NULL);
	/* FIXME: we assume log_handler_id wont be 0 after this call;
	 * check this...
	 */
	return TRUE;

}


static gboolean
init_log (gboolean log_stderr, gboolean log_append)
{
	static LogHandlerData data;

	if (MU_APP_DATA._log_handler_id)
		g_log_remove_handler (NULL, MU_APP_DATA._log_handler_id);

	if (log_stderr)
		MU_APP_DATA._log = mu_log_new_with_fd (fileno(stderr), 0);
	else {
		char *logfile = g_strdup_printf ("%s/%s.%s",
						 mu_app_home(),
						 g_get_prgname(),
						 MU_LOG_SUFFIX);
		MU_APP_DATA._log = mu_log_new_with_file (logfile, log_append);
		g_free (logfile);
	}

	if (!MU_APP_DATA._log)
		return FALSE;

	data._log   = MU_APP_DATA._log;
	data._debug = MU_APP_DATA._debug;

	MU_APP_DATA._log_handler_id =
		g_log_set_handler (NULL, G_LOG_LEVEL_MASK,
				   (GLogFunc)log_handler, &data);

	return TRUE;
}


const gchar*
mu_app_version (void)
{
	if (!mu_app_initialized())
		return NULL;

	return 	"mu v" VERSION "; mu-db v" MU_DATABASE_VERSION
		"\nCopyright (C) 2008 Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>\n\n"
		"License GPLv3+: GNU GPL version 3 or later\n\n"
		"This is free software: you are free to change and redistribute it.\n"
		"There is NO WARRANTY, to the extent permitted by law.";
}


static gboolean
check_app_data (void)
{
	if (!MU_APP_DATA._home || create_home_maybe (MU_APP_DATA._home) < 0) {
		mu_app_uninit();
		return FALSE;
	}

	if (!init_log (MU_APP_DATA._log_stderr, MU_APP_DATA._log_append)) {
		mu_app_uninit ();
		return FALSE;
	}

	if (!(MU_APP_DATA._conf = init_conf())) {
		mu_app_uninit();
		return FALSE;
	}

	return TRUE;
}



gboolean
mu_app_init (int *argcp, char ***argvp, const char* appname)
{
	GOptionContext *context;
	GOptionGroup *group;

	g_return_val_if_fail (argcp && argvp && appname, FALSE);
		
	if (mu_app_initialized())
		return TRUE;
	
	memset (&MU_APP_DATA,0,sizeof(MU_APP_DATA));

	MU_APP_DATA._init = 1;
	g_set_prgname (appname);
	
	/* until we have the real log set up, silence debug/info logging */
	init_log_silence ();

	group = mu_app_get_option_group ();
	context = g_option_context_new ("mu");
	g_option_context_set_ignore_unknown_options (context, TRUE);
	g_option_context_add_group (context, group);
	g_option_context_set_help_enabled(context, FALSE);

	if (!g_option_context_parse (context, argcp, argvp, NULL)) {
		g_printerr ("%s: error parsing command line\n", __FUNCTION__);
		g_option_context_free (context);
		return FALSE;
	}

	g_option_context_free (context);

	if (MU_APP_DATA._version) {
		g_print ("%s\n", mu_app_version());
		exit (0);
	}

	if (!MU_APP_DATA._home)
		MU_APP_DATA._home = default_mu_app_home ();

	if (!check_app_data())
		return FALSE;
	
	g_debug ("mu initialized");
	return TRUE;
}


void
mu_app_uninit (void)
{
	g_free  (MU_APP_DATA._home);
	g_free  (MU_APP_DATA._sqlite_path);
	g_free  (MU_APP_DATA._xapian_path);
	g_free  (MU_APP_DATA._bookmarks_path);

	mu_conf_destroy (MU_APP_DATA._conf);

	/* we won't log to the log anymore once it's dead... */
	g_debug ("mu uninitialized; going silent");

	mu_log_destroy  (MU_APP_DATA._log);

	/* restore normal logging */
	g_log_remove_handler (NULL, MU_APP_DATA._log_handler_id);

	memset (&MU_APP_DATA, 0, sizeof(MU_APP_DATA));
}


const char*
mu_app_home (void)
{
	g_return_val_if_fail (mu_app_initialized(), NULL);

	return MU_APP_DATA._home;
}



MuConf*
mu_app_conf (void)
{
	g_return_val_if_fail (mu_app_initialized(), NULL);

	return MU_APP_DATA._conf;

}

const char*
mu_app_sqlite_path (void)
{
	g_return_val_if_fail (mu_app_initialized(), NULL);

	if (MU_APP_DATA._sqlite_path == NULL) {
		MU_APP_DATA._sqlite_path =
			g_strdup_printf ("%s"
					 G_DIR_SEPARATOR_S MU_SQLITE_DB_NAME,
					 mu_app_home());
	}
	return MU_APP_DATA._sqlite_path;
}


const char*
mu_app_xapian_path (void)
{
	g_return_val_if_fail (mu_app_initialized(), NULL);

	if (MU_APP_DATA._xapian_path == NULL) {
		MU_APP_DATA._xapian_path =
			g_strdup_printf ("%s"
					 G_DIR_SEPARATOR_S MU_XAPIAN_DB_NAME,
					 mu_app_home());

	}
	return MU_APP_DATA._xapian_path;
}


const char*
mu_app_bookmarks_path (void)
{
	g_return_val_if_fail (mu_app_initialized(), NULL);

	if (MU_APP_DATA._bookmarks_path == NULL) {
		MU_APP_DATA._bookmarks_path =
			g_strdup_printf ("%s"
					 G_DIR_SEPARATOR_S MU_EXPR_BOOKMARKS_NAME,
					 mu_app_home());

	}
	return MU_APP_DATA._bookmarks_path;
}




