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
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <glib.h>

#include "mu/mu.h"
#include "msg/mu-msg-gmime.h"
#include "mu-msg-header-view.h"

int
main (int argc, char *argv[])
{
	GtkWidget *win, *viewer;
	MuMsgGMime *msg;
	GError *err;
	err = NULL;

	gtk_init (&argc, &argv);
	if (!argc > 1) {
		g_printerr ("usage: mu-msg-header-view [OPTIONS] mailfile\n");
		return 1;
	}

	viewer = mu_msg_header_view_new ();
	win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_size_request (win, 200, 100);

	gtk_container_add (GTK_CONTAINER(win), GTK_WIDGET(viewer));
	gtk_widget_show_all (win);

	mu_msg_gmime_init ();
	msg = mu_msg_gmime_new(argv[1]);
	if (!msg) {
		g_printerr ("failed to create msg\n");
		mu_msg_gmime_uninit ();
		return 1;
	}

	mu_msg_header_view_set_message (MU_MSG_HEADER_VIEW(viewer),msg);

	g_signal_connect (G_OBJECT(win), "delete-event", gtk_main_quit, NULL);
	
	gtk_main ();
	
	mu_msg_gmime_destroy (msg);
	mu_msg_gmime_uninit ();
	
	/* 'stop' is not considered an error */
	return  0;
}
