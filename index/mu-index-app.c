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
#define _XOPEN_SOURCE

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "app/mu-util.h"
#include "app/mu-app.h"

#include "msg/mu-msg-gmime.h"
#include "mu-index-app.h"


static gboolean _INITIALIZED = FALSE;
static char*    _MAILDIR     = NULL;

/*
 * this is set by signal handlers and checked by our
 * callbacks, and allows us to terminate gracefully
 * (close database etc.)
 */
static gboolean _CAUGHT_SIGNAL = FALSE;

struct _IndexOptions {
	gboolean quiet;
	gboolean cleanup;
	const char *maildir;

	unsigned int sqlite_transaction_size;
	unsigned int synchronous;
	unsigned int temp_store;
	
	unsigned int xapian_transaction_size;
	gboolean sort_inodes;
};
typedef struct _IndexOptions IndexOptions;

static IndexOptions INDEX_OPTIONS;
static GOptionEntry OPTION_ENTRIES[] = {
	/* the tune- options are for performance tuning,
	 * but the defaults should be good for most */
	{"tune-sqlite-transaction-size", 0, G_OPTION_FLAG_HIDDEN,
	 G_OPTION_ARG_INT, &INDEX_OPTIONS.sqlite_transaction_size},
	{"tune-synchronous", 0, G_OPTION_FLAG_HIDDEN,
	 G_OPTION_ARG_INT, &INDEX_OPTIONS.synchronous},
	{"tune-temp-store", 0, G_OPTION_FLAG_HIDDEN,
	 G_OPTION_ARG_INT, &INDEX_OPTIONS.temp_store},
	{"tune-xapian-transaction-size", 0, G_OPTION_FLAG_HIDDEN,
	 G_OPTION_ARG_INT, &INDEX_OPTIONS.xapian_transaction_size},
	{"tune-sort-inodes", 0, G_OPTION_FLAG_HIDDEN,
	 G_OPTION_ARG_INT, &INDEX_OPTIONS.sort_inodes},	
	{"maildir", 'm', 0, G_OPTION_ARG_FILENAME, &INDEX_OPTIONS.maildir,
	 "top of the maildir"},
	{"quiet", 'q', 0, G_OPTION_ARG_NONE, &INDEX_OPTIONS.quiet,
	 "don't show progress info during indexation"},
	{NULL}
};

static void
set_default_options (void)
{
	memset (&INDEX_OPTIONS, 0, sizeof(IndexOptions));
	
	/*defaults:*/
	INDEX_OPTIONS.sqlite_transaction_size = 100;   
	INDEX_OPTIONS.synchronous      = 0;    /* OFF */ 
	INDEX_OPTIONS.temp_store       = 2;    /* MEMORY */
	INDEX_OPTIONS.xapian_transaction_size = 1000;   
	INDEX_OPTIONS.sort_inodes      = TRUE;
}

static void
sig_handler (int i)
{
	/* when pressed twice... */
	if (_CAUGHT_SIGNAL)
		exit (0);

	g_print ("\ncaught signal %d; exiting loop\n", i);
	_CAUGHT_SIGNAL = TRUE;
}

static void
install_sig_handler (void)
{
	struct sigaction action;
	int i, sigs[] = { SIGINT, SIGHUP, SIGTERM };

	_CAUGHT_SIGNAL = FALSE;

	action.sa_handler = sig_handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;

	for (i = 0;  i!= sizeof(sigs)/sizeof(sigs[0]); ++i) 
		if (sigaction (sigs[i], &action, NULL) != 0)
			g_printerr ("error: installing sighandler %d failed\n",
				    sigs[i]);
}



static char*
get_default_maildir (void)
{
	const char* home;

	if (getenv("MAILDIR")) 
		return g_strdup (getenv("MAILDIR"));
	
	home = getenv ("HOME");
	if (!home)
		home = g_get_home_dir ();

	return g_strdup_printf ("%s%c%s", home, G_DIR_SEPARATOR,
				"Maildir");
}


static gboolean
handle_options (int *argcp, char ***argvp)
{
	GError *err = NULL;
	if (!mu_conf_handle_options (mu_app_conf(),OPTION_ENTRIES, argcp, argvp, 
				     &err)) {
		g_printerr ("option parsing failed: %s\n", 
			    (err && err->message) ? err->message : "?" );
		if (err)
			g_error_free (err);
		return FALSE;
	}
	return TRUE;
}


static gchar*
get_maildir_and_check (const char* maildir_or_null, GError **err)
{
	gchar* maildir; 

	if (!maildir_or_null) {
		if (INDEX_OPTIONS.maildir)
			maildir = mu_util_homedir_expand (INDEX_OPTIONS.maildir);
		else
			maildir = get_default_maildir ();
	} else
		maildir = g_strdup(maildir_or_null);

	if (!g_path_is_absolute (maildir)) {
		g_set_error (err, 0, 0, "'%s' is not an absolute path",
			     maildir);
		g_free (maildir);
		return NULL;
	}
	
	if (access (maildir, R_OK) != 0) {
		g_set_error (err, 0, 0, "'%s' is not a valid Maildir: %s",
			      maildir, strerror(errno));
		g_free (maildir);
		return NULL;
	}

	return maildir;
}


gboolean
mu_index_app_init (int *argcp, char***argvp, GError **err)
{
	if (_INITIALIZED)
		return TRUE;

	g_return_val_if_fail (argcp, FALSE);
	g_return_val_if_fail (argvp, FALSE);

	install_sig_handler ();
	set_default_options ();

	if (!mu_app_init (argcp, argvp, "mu-index")) {
		g_set_error (err, 0, 0, "failed init mu");
		return FALSE;
	}
	if (!handle_options(argcp, argvp)) {
		g_set_error (err, 0, 0, "failed to handle options");
		return FALSE;
	}
	
	_MAILDIR = get_maildir_and_check (*argcp > 1 ? (*argvp)[1] : NULL, 
					  err);	
	if (!_MAILDIR)
		return FALSE;
	
	mu_msg_gmime_init ();

	return _INITIALIZED = TRUE;
}



gboolean
mu_index_app_uninit (void)
{
	if (!_INITIALIZED)
		return TRUE;

	g_free (_MAILDIR);
	_MAILDIR = NULL;

	mu_msg_gmime_uninit ();
	mu_app_uninit ();

	_INITIALIZED = FALSE;

	return TRUE;
}


static void
tune_index_settings (MuIndex* index)
{
	/* optimization tuning */
	mu_index_tune (index, 
		       INDEX_OPTIONS.sqlite_transaction_size,
		       INDEX_OPTIONS.synchronous,
		       INDEX_OPTIONS.temp_store,
		       INDEX_OPTIONS.xapian_transaction_size,
		       INDEX_OPTIONS.sort_inodes);
}


MuIndex*
mu_index_app_get_index (GError **err)
{
	MuIndex *index;
	
	g_return_val_if_fail (_INITIALIZED, NULL);
	
	index = mu_index_new (mu_app_sqlite_path(), mu_app_xapian_path());
	if (!index) {
		g_set_error (err, 0, 0, "cannot get index object");
		return NULL;
	}

	tune_index_settings (index);
	return index;
}


gboolean
mu_index_app_quiet (void)
{
	g_return_val_if_fail (_INITIALIZED, FALSE);
	
	return INDEX_OPTIONS.quiet;
}


const char*
mu_index_app_get_maildir(void)
{
	g_return_val_if_fail (_INITIALIZED, FALSE);
	
	return _MAILDIR;
}

const gboolean
mu_index_app_caught_signal (void)
{
	g_return_val_if_fail (_INITIALIZED, FALSE);
	
	return _CAUGHT_SIGNAL;

}
