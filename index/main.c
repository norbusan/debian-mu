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
#include "app/mu-util.h"
#include "msg/mu-msg-gmime.h"

#include "mu-index.h"
#include "mu-index-app.h"

			
static void
show_progress (int msg, int total)
{
	static int len = 0;
	static int s = 0;
	const char states[] = {'-', '\\', '|', '/'};
	int i;

	for (i = 0; i != len; ++i)
		fprintf (stdout, "\b");

	msg++; /* we're starting with 1 not 0 */
			
	len = printf ("%c mu-index processing msg %d of %d (%0.1f%%)%s",
		      states[s++ % sizeof(states)],
		      msg,
		      total,
		      total ? ((float)msg*100/(float)total) : 0,
		      msg == total ? "\n" : "");
	fflush (stdout);
}


static MuResult
index_run_callback (MuIndexStats *stats, int *msgcount)
{
	show_progress (stats->_processed, *msgcount);

	if (mu_index_app_caught_signal())
		return MU_STOP;
	else
		return MU_OK;
}

static MuResult
index_stats_callback (MuIndexStats *stats, void *user_data)
{
	static int seen = 0;
	if (seen++ % 5000 == 0) {
		printf (".");
		fflush (stdout);
	}	

	if (mu_index_app_caught_signal())
		return MU_STOP;
	else
		return MU_OK;
}

static MuResult 
run_with_progress_info (MuIndex *index, const char* maildir)
{
	MuIndexStats stats;
	MuResult iresult;
	int counted;

	time_t start = time (NULL);
	
	g_print ("mu-index: scanning %s, storing in %s\n", 
		 maildir, mu_app_sqlite_path());
	
	g_print ("* counting msgs..");
	memset (&stats, 0, sizeof(MuIndexStats));
	iresult = mu_index_stats (index, maildir, &stats, index_stats_callback,
				  NULL);
	if (iresult != MU_OK)
		return iresult;
	g_print ("%d message%s found\n", stats._processed,
		stats._processed == 1 ? "" : "s");

	counted = stats._processed;
	
	g_print ("* cleaning up database..");
	memset (&stats, 0, sizeof(MuIndexStats));
	iresult = mu_index_cleanup (index, &stats, index_stats_callback, NULL);
	if (iresult != MU_OK) {
		if (iresult == MU_STOP)
			g_print ("user stopped\n");
		else
			g_print ("some error occured; check the logs\n");
		
		return iresult;
	}

	g_print ("%d message%s cleaned up\n", stats._cleaned_up,
		 stats._cleaned_up == 1 ? "" : "s");

	memset (&stats, 0, sizeof(MuIndexStats));
	iresult = mu_index_run (index, maildir, FALSE, &stats, 
				(MuIndexCallback)index_run_callback, 
				&counted);
	if (counted != stats._processed) 
		g_print ("\nNOTE: # of msgs may differ from initial count\n");

	g_print ("* message(s) up-to-date : %d\n", stats._uptodate);
	g_print ("* message(s) added      : %d\n", stats._added);
	g_print ("* message(s) updated    : %d\n", stats._updated);
	g_print ("* message(s) cleaned up : %d\n", stats._cleaned_up);
		
	g_print ("\n* time taken: %ld second(s)\n", time(NULL) - start); 
	
	return iresult;
}


int
main (int argc, char *argv[])
{
	MuIndex *index = NULL;
	GError *err = NULL;
	MuResult iresult;

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
		
	/* now do it! */
	if (mu_index_app_quiet()) /* do we want progress info? */
		iresult = mu_index_run (index, mu_index_app_get_maildir(), 
					FALSE, NULL, NULL, NULL);
	else  /* be very silent */
		iresult = run_with_progress_info (index,
						  mu_index_app_get_maildir());
	
	mu_index_destroy (index);
	mu_index_app_uninit ();

	/* 'stop' is not considered an error */
	return  (iresult == MU_OK || iresult == MU_STOP) ? 0 : 1;
}
