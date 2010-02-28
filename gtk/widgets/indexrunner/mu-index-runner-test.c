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
#include "index/mu-index.h"
#include "index/mu-index-app.h"
#include "mu-index-runner.h"


static void
on_response (GtkWidget *dialog)
{
	gtk_widget_destroy (dialog);
	gtk_main_quit ();
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
	content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	gtk_container_add (GTK_CONTAINER(content), GTK_WIDGET(runner));

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
