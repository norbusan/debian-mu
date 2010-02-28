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
#include "app/mu-app.h"
#include "index/mu-index.h"
#include "index/mu-index-app.h"

#include "gtk/widgets/indexrunner/mu-index-runner.h"


/* the on_response/on_quit construction is a bit ugyly, but needed
 * so we can gracefully close the database; otherwise we'd be called
 * by the database, *after* we're already destroyed...
 */
static gboolean
on_quit (GtkWidget *dialog)
{
	gtk_widget_destroy (dialog);
	gtk_main_quit ();

	return FALSE;
}

static void
on_response (GtkDialog *dialog)
{
	MuIndexRunner *runner;

	runner = (MuIndexRunner*)g_object_get_data (G_OBJECT(dialog),"runner");
	mu_index_runner_stop (runner);
	g_timeout_add (1000, (GSourceFunc)on_quit, dialog);
}


int
main (int argc, char *argv[])
{
	GtkWidget *dialog, *content, *runner;
	MuIndex *index = NULL;
	GError *err;
	MuResult iresult;
	err = NULL;

	gtk_init (&argc, &argv);

	if (!mu_index_app_init (&argc, &argv, &err)) {
		g_printerr ("error: initialization failed: %s\n", 
			    err ? err->message : "see the logs");
		if (err)
			g_error_free (err);
		return 1;
	}
		
	if (!(index = mu_index_app_get_index (&err))) {
		g_printerr ("error: getting index failed: %s\n", 
			    err ? err->message : "see the logs");
		if (err)
			g_error_free (err);
		mu_index_app_uninit ();
		return 1;
	}

	iresult = MU_OK;

	runner = mu_index_runner_new (index,
				      mu_index_app_get_maildir());
	dialog = gtk_dialog_new_with_buttons ("Indexing....",
					      NULL,
					      GTK_DIALOG_MODAL,
					      GTK_STOCK_CANCEL,GTK_RESPONSE_REJECT,
					      NULL);
	gtk_widget_set_size_request (dialog, 300, 140);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

	content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	gtk_container_add (GTK_CONTAINER(content), GTK_WIDGET(runner));

	g_object_set_data (G_OBJECT(dialog), "runner", runner);
	g_signal_connect_swapped (dialog, "response", 
				  G_CALLBACK (on_response),
				  dialog);

	gtk_widget_show_all (dialog);

	mu_index_runner_start (MU_INDEX_RUNNER(runner));
	gtk_main ();

	mu_index_destroy (index);
	mu_index_app_uninit ();
	
	/* 'stop' is not considered an error */
	return  (iresult == MU_OK || iresult == MU_STOP) ? 0 : 1;
}
