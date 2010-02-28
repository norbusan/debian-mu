/* 
** Copyright (C) 2008 Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>
**
** This program is free software; you can redistribute it and/or modify
1** it under the terms of the GNU General Public License as published by
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /*HAVE_CONFIG_H*/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <errno.h>

#include "mu/mu.h"
#include "path/mu-path.h"

#include "mu-storage-sqlite.h"

#ifdef MU_HAVE_XAPIAN
#include "mu-storage-xapian.h"
struct _MuIndex {
	MuStorageSQLite  *_sqlite;
	MuStorageXapian  *_xapian;
	gboolean _sort_inodes;
};
# else
struct _MuIndex {
	MuStorageSQLite  *_sqlite;
	gboolean _sort_inodes;

};
#endif /*!MU_HAVE_XAPIAN*/

#include "mu-index.h"

MuIndex* 
mu_index_new (const char* mpath, const char *cpath)
{
	MuIndex *index;
	
	g_return_val_if_fail (mpath, NULL);
	g_return_val_if_fail (cpath, NULL);

	do {
		index = g_new0 (MuIndex, 1);		
		index->_sqlite = mu_storage_sqlite_new (mpath);
		if (!index->_sqlite) {
			g_warning ("%s: failed to get meta storage", __FUNCTION__); 
			break;
		}
		
#ifdef MU_HAVE_XAPIAN
		index->_xapian = mu_storage_xapian_new (cpath);
		if (!index->_xapian) {
			g_warning ("%s: failed to get xapian storage", __FUNCTION__); 
			break;
		}
#endif /*MU_HAVE_XAPIAN*/

		index->_sort_inodes = FALSE;
		g_debug ("created index object");

		return index;
	
	} while (0);
	
	mu_index_destroy (index);

	return NULL;
}


void 
mu_index_destroy (MuIndex *index)
{
	if (index) {
		mu_storage_sqlite_destroy (index->_sqlite);
#ifdef MU_HAVE_XAPIAN
		mu_storage_xapian_destroy (index->_xapian);
#endif /*MU_HAVE_XAPIAN*/

		g_free (index);
		g_debug ("destroyed index object");
	}
}

void
mu_index_tune (MuIndex *index, 
	       unsigned int sqlite_tx_size, 
	       unsigned int synchronous, 
	       unsigned int temp_store,
	       unsigned int xapian_tx_size,
	       gboolean sort_inodes)
{
	g_return_if_fail (index);
	
	mu_storage_sqlite_tune (index->_sqlite, 
				sqlite_tx_size,
				synchronous,
				temp_store);
#ifdef MU_HAVE_XAPIAN
	mu_storage_xapian_tune (index->_xapian,
				xapian_tx_size);
#endif /*MU_HAVE_XAPIAN*/
	index->_sort_inodes = sort_inodes;
}


struct _MuIndexCallbackData {
	MuIndexCallback       _cb;
	MuStorageSQLite*      _sqlite;
#ifdef MU_HAVE_XAPIAN
	MuStorageXapian*      _xapian;
#endif /*MU_HAVE_XAPIAN*/
	void*                 _user_data;
	MuIndexStats*         _stats;
	gboolean	      _force;
};
typedef struct _MuIndexCallbackData MuIndexCallbackData;

static MuResult
insert_or_update (const char* fullpath, MuIndexCallbackData *data)
{ 
	MuMsgGMime *msg;
	int64_t id; /* the id in the db */

	if (!(msg = mu_msg_gmime_new (fullpath))) {
		g_warning ("%s: failed to create mu_msg for %s",
			   __FUNCTION__, fullpath);
		return MU_ERROR;
	}
	
	if ((id = mu_storage_sqlite_store (data->_sqlite, msg)) == -1) {
		g_warning ("%s: storing %s failed", __FUNCTION__, fullpath);
		mu_msg_gmime_destroy (msg);
		return MU_ERROR;
	} 
	
#ifdef MU_HAVE_XAPIAN
	/* we got a valid id; scan the message contents as well */
	if (mu_storage_xapian_store (data->_xapian, id, msg) != MU_OK) {
		g_warning ("%s: storing content %s failed", __FUNCTION__, 
			   fullpath);
		/* ignore...*/
	} 
#endif /*MU_HAVE_XAPIAN*/

	mu_msg_gmime_destroy (msg);
	return MU_OK;	
}




static MuResult
run_callback_maybe ( MuIndexCallbackData *data)
{
	if (data && data->_cb) {
		MuResult result = 
			data->_cb (data->_stats, data->_user_data);
		if (result != MU_OK)
 			g_debug ("%s: callback said %d", __FUNCTION__, result);
	}
	return MU_OK;
}


static MuResult
on_run_maildir_file (const char* fullpath, time_t filestamp, 
		     MuIndexCallbackData *data)
{
	MuResult result;
	time_t dbstamp;
	gboolean uptodate;
	
	g_return_val_if_fail (fullpath, MU_ERROR);

	result = run_callback_maybe (data);
	if (result != MU_OK)
		return result;
	
	++data->_stats->_processed;
		
	/* see if we need to update/insert anything...*/
	dbstamp = mu_storage_sqlite_message_timestamp (data->_sqlite, fullpath);
	if (dbstamp >= filestamp && !data->_force) { /* msg is uptodate, no force */
		uptodate = TRUE;
		result = MU_OK;
	} else {
		uptodate = FALSE;
		result = insert_or_update(fullpath, data);
	}
	
	/* update statistics */
	if (result == MU_OK && data && data->_stats) {
		if (uptodate)
			++data->_stats->_uptodate;
		else if (dbstamp == 0)  /* 0 means no stamp, ie. a new msg */
			++data->_stats->_added;
		else
			++data->_stats->_updated;
	}	
	return result;
}


static gboolean
check_path (const char* path)
{
	g_return_val_if_fail (path, FALSE);

	if (!g_path_is_absolute (path)) {
		g_warning ("%s: path is not absolute '%s'", 
			   __FUNCTION__, path);
		return FALSE;
	}
	
	if (access (path, R_OK) != 0) {
		g_warning ("%s: cannot open '%s': %s", 
			   __FUNCTION__, path, strerror (errno));
		return FALSE;
	}
	
	return TRUE;
}


MuResult
mu_index_run (MuIndex *index, const char* path,
	      gboolean force, MuIndexStats *stats,
	      MuIndexCallback cb, void *user_data)
{
	MuIndexCallbackData cb_data;
	
	g_return_val_if_fail (index && index->_sqlite,
			      MU_ERROR);
#ifdef MU_HAVE_XAPIAN
	g_return_val_if_fail (index->_xapian,
			      MU_ERROR);
#endif /*MU_HAVE_XAPIAN*/
	g_return_val_if_fail (check_path (path), MU_ERROR);

	cb_data._cb        = cb;
	cb_data._user_data = user_data;
	cb_data._sqlite    = index->_sqlite;
#ifdef MU_HAVE_XAPIAN
	cb_data._xapian    = index->_xapian;
#endif /*MU_HAVE_XAPIAN*/
	cb_data._stats     = stats;
	cb_data._force     = force;

	return mu_path_walk_maildir (path, index->_sort_inodes,
				     (MuWalkCallback)on_run_maildir_file,
				     &cb_data);
}

static MuResult
on_stats_maildir_file (const char *fullpath, time_t timestamp, 
		       MuIndexCallbackData *cb_data)
{
	
	MuResult result;
	
	if (cb_data && cb_data->_cb)
		result = cb_data->_cb (cb_data->_stats, cb_data->_user_data);
	else
		result = MU_OK;

	if (result == MU_OK) { 
		if (cb_data->_stats)
			++cb_data->_stats->_processed;
		return MU_OK;
	} else 
		return result; /* MU_STOP or MU_OK */
}


MuResult
mu_index_stats (MuIndex *index, const char* path,
		MuIndexStats *stats, MuIndexCallback cb, 
		void *user_data)
{
	MuIndexCallbackData cb_data;
	
	g_return_val_if_fail (index, MU_ERROR);
	g_return_val_if_fail (check_path (path), MU_ERROR);

	cb_data._cb        = cb;
	cb_data._stats     = stats;
	cb_data._user_data = user_data;

	return mu_path_walk_maildir (path, index->_sort_inodes,
				     (MuWalkCallback)on_stats_maildir_file,
				     &cb_data);
}



static MuResult
sqlite_remove_callback (MuMsgStatus status, const char* path, 
			MuIndexCallbackData *cb_data)
{
	MuResult result;
	
	if (cb_data && cb_data->_cb)
		result = cb_data->_cb (cb_data->_stats, cb_data->_user_data);
	else
		result = MU_OK;

	switch (status) {
		
	/* callback is just a fyi; file still exists and will not be deleted */
	case MU_MSG_STATUS_EXISTS: 
		return MU_OK;
		
	case MU_MSG_STATUS_CLEANUP:
#ifdef MU_HAVE_XAPIAN
		return mu_storage_xapian_cleanup (cb_data->_xapian, path);
#endif /*MU_HAVE_XAPIAN*/
		return MU_OK; /* message will be removed */

	case MU_MSG_STATUS_CLEANED_UP:
		/* message was succesfully cleaned up from db; update stats */
		if (cb_data && cb_data->_stats)
			++cb_data->_stats->_cleaned_up; 
		break;
		
	default:
		g_warning ("%s: unexpected status %d", 
			   __FUNCTION__, status);
		return MU_ERROR;
	}

	return result;
}

MuResult
mu_index_cleanup (MuIndex *index, MuIndexStats *stats, MuIndexCallback cb, 
		  void *user_data)
{
	MuIndexCallbackData cb_data;
	MuResult result = MU_OK;
	
	g_return_val_if_fail (index, MU_ERROR);
	memset (&cb_data, 0, sizeof(MuIndexCallbackData));
	
	cb_data._cb        = cb;
	cb_data._user_data = user_data;
#ifdef MU_HAVE_XAPIAN
	cb_data._xapian    = index->_xapian;
#endif /*MU_HAVE_XAPIAN*/
		
	cb_data._stats     = stats;
	result = mu_storage_sqlite_cleanup 
		(index->_sqlite,
		 (MuStorageSQLiteCleanupCallback)sqlite_remove_callback,
		 &cb_data);
	
	return result;
}
