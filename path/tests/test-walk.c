/* 
** Copyright (C) 2008 Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>
**
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 3, or (at your option) any
** later version.
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
#include <glib.h>
#include <glib/gstdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "path/mu-path.h"

static MuResult
dummy_callback(void)
{
	return MU_OK;
}

static void
test_walk_non_existing (void)
{
	/* we should get an error for non-existing dir */
	if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDERR)) {		
		g_assert (mu_path_walk_maildir ("/foo/bar/ReallyNonExisting@#*&@",
						FALSE,
						(MuWalkCallback)dummy_callback, 
						NULL) == MU_ERROR);
	}
	g_test_trap_assert_failed ();
}


static void
test_walk_maildir_dummy (void)
{
	g_assert (mu_path_walk_maildir ("TestMaildir",
					FALSE,
					(MuWalkCallback)dummy_callback, 
					NULL) == MU_OK); 
}



static MuResult
walk_callback (const char* fullpath, time_t timestamp, void *data)
{
	/* make sure the right message are included, and ignored */

	++(*(int*)data);
	if (strcmp (fullpath, "TestMaildir/new/new0001.server") == 0)
		return MU_OK;
	if (strcmp (fullpath, "TestMaildir/cur/cur0003.server:2,S") == 0)
		return MU_OK;
	if (strcmp (fullpath, "TestMaildir/cur/cur0002.server:2,S") == 0)
		return MU_OK;
	if (strcmp (fullpath, "TestMaildir/subdir/new/subdirnew004.server") == 0)
		return MU_OK;
	if (strcmp (fullpath, "TestMaildir/.dotdir/new/new006") == 0)
		return MU_OK;

	if (strcmp (fullpath, "TestMaildir/subdir/cur/subdirr0005.server!2,S") 
		== 0)
		return MU_OK; 
	
	g_assert_not_reached ();

	return MU_OK;
}

static void
test_walk_maildir (void)
{
	int count = 0;

	g_assert (mu_path_walk_maildir ("TestMaildir", 
					FALSE,
					(MuWalkCallback)walk_callback, 
					&count) == MU_OK); 
	g_assert (count == 6);
}


static void
shutup (void) {}

int
main (int argc, char *argv[])
{
	g_test_init (&argc, &argv, NULL);
	
	g_test_add_func ("/path/walk-non-existing", test_walk_non_existing);
	g_test_add_func ("/path/walk-maildir-dummy", test_walk_maildir_dummy);	
	g_test_add_func ("/path/walk-maildir", test_walk_maildir);

	g_log_set_handler (NULL,
			   G_LOG_LEVEL_DEBUG|G_LOG_LEVEL_MESSAGE|G_LOG_LEVEL_INFO,
			   (GLogFunc)shutup, NULL); 
	return g_test_run ();
}
