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

#include <stdio.h>
#include <string.h>
#include <alloca.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "mu/mu.h"
#include "mu/mu-msg-str.h"
#include "mu/mu-msg-flags.h"
#include "app/mu-app.h"
#include "msg/mu-msg-gmime.h"

enum {
	OPTION_FROM,
	OPTION_TO,
	OPTION_CC,
	OPTION_SUBJECT,
	OPTION_DATE,
	OPTION_BODY,
	OPTION_USER_AGENT,
	OPTION_MESSAGE_ID,
	OPTION_PRIORITY,
	OPTION_NUM
};

	
struct _MsgInfoOptions {
	gboolean _field[OPTION_NUM];
};
typedef struct _MsgInfoOptions MsgInfoOptions;

static MsgInfoOptions OPTIONS;
static GOptionEntry OPTION_ENTRIES [] = {
	{"subject", 's', 0, G_OPTION_ARG_NONE, &OPTIONS._field[OPTION_SUBJECT],
	 "get the message subject"},
	{"sender", 'f', 0, G_OPTION_ARG_NONE, &OPTIONS._field[OPTION_FROM],
	 "get the message sender (From:)"},
	{"to", 't', 0, G_OPTION_ARG_NONE, &OPTIONS._field[OPTION_TO],
	 "get the message recipients (To:)"},
	{"cc", 'c', 0, G_OPTION_ARG_NONE, &OPTIONS._field[OPTION_CC],
	 "get the carbon-copy recipients (Cc:)"},
	{"date", 'd', 0, G_OPTION_ARG_NONE,&OPTIONS._field[OPTION_DATE],
	 "get the message date"},
	{"body", 'b', 0, G_OPTION_ARG_NONE, &OPTIONS._field[OPTION_BODY],
	 "get the message body (default)"},
	{"user-agent", 'u', 0, G_OPTION_ARG_NONE, &OPTIONS._field[OPTION_USER_AGENT],
	 "get the message user agent"},
	{"message-id", 'i', 0, G_OPTION_ARG_NONE,&OPTIONS._field[OPTION_MESSAGE_ID],
	 "get the message-id"},
	{"priority", 'p', 0, G_OPTION_ARG_NONE, &OPTIONS._field[OPTION_PRIORITY],
	 "get the message priority"},
	{NULL}
};

static gboolean
display_info_field (MuMsgGMime *msg, MsgInfoOptions *opts)
{
	int i;
	for (i = 0; i != OPTION_NUM; ++i) {

		const char *str = NULL;

		if (!opts->_field[i])
			continue;
		
		switch (i) {
	
		case OPTION_FROM:
			str = mu_msg_gmime_get_from (msg); break;
		case OPTION_TO:
			str = mu_msg_gmime_get_to (msg); break;
		case OPTION_CC:
			str = mu_msg_gmime_get_cc (msg); break;
		case OPTION_SUBJECT:
			str = mu_msg_gmime_get_subject (msg); break;
		case OPTION_BODY:
			str = mu_msg_gmime_get_body_text (msg); 
			if (!str)
				str = mu_msg_gmime_get_body_html (msg);
			break;
		case OPTION_USER_AGENT:
			str = mu_msg_gmime_get_user_agent (msg); break;
		case OPTION_MESSAGE_ID:
			str = mu_msg_gmime_get_msgid (msg); break;
		case OPTION_PRIORITY:
			str = mu_msg_str_prio(
				mu_msg_gmime_get_priority (msg)); break;
		case OPTION_DATE: {
			time_t t = mu_msg_gmime_get_date (msg);
			if (t) 
				str = mu_msg_str_date_s(t);
			break;
		}
		default:
			g_warning ("%s: invalid field %d", __FUNCTION__, i);
			return FALSE;
		}

		if (str)
			g_print ("%s\n", str);
	}
	return TRUE;
}


static gboolean 
display_info (const char* file, MsgInfoOptions *opts)
{
	MuMsgGMime *msg;	
	g_return_val_if_fail (file, FALSE);

	if (access (file, R_OK) != 0) {
		g_printerr ("error opening %s: %s\n", file, strerror(errno));
		return FALSE;
	}
	
	msg = mu_msg_gmime_new (file);
	if (!msg) {
		g_printerr ("error: failed to analyze message %s\n", file);
		return FALSE;
	}

	display_info_field (msg, opts);

	mu_msg_gmime_destroy (msg);
	return TRUE;
}


gboolean
get_options (int *argcp, char ***argvp)
{	
	GError *err = NULL;
	int i;
	gboolean has_opt;
	
	memset (&OPTIONS, 0, sizeof(MsgInfoOptions));	

	if (!mu_conf_handle_options (mu_app_conf(),OPTION_ENTRIES, 
				     argcp, argvp, &err)) {
		g_printerr ("option parsing failed: %s\n", err ? err->message : "");
		if (err)
			g_error_free (err);
		return FALSE;
	}

	/* default to body, if no options are set... */
	has_opt = FALSE;
	for (i = 0; i != OPTION_NUM; ++i) {
		if (OPTIONS._field[i]) {
			has_opt = TRUE;
			break;
		}
	}

	if (!has_opt)
		OPTIONS._field[OPTION_BODY] = TRUE;
	
	if (*argcp <= 1) {
		g_printerr ("error: expected: some mailfile(s)\n");
		return FALSE;
	}
	
	return TRUE;
}



int
main (int argc, char *argv[])
{
	int retval; 
	int i;

	if (!mu_app_init (&argc, &argv, "mu-msginfo")) {
		g_printerr ("failed to init mu\n");
		return 1;
	}

	if (!get_options (&argc, &argv)) {
		g_printerr ("error: failed to handle options\n");
		mu_app_uninit();
		return 1;
	}
		
	mu_msg_gmime_init();

	retval = 0;
	for (i = 1; i != argc; ++i) {
		if (!display_info (argv[i], &OPTIONS)) {
			retval = 1;
			break;
		}
	}
	
	mu_msg_gmime_uninit ();
	mu_app_uninit ();
	
	return retval;
}
