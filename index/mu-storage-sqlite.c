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

#include <sqlite3.h>
#include <glib.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "mu/mu.h"

#include "mu-storage-sqlite.h"
#include "mu-storage-sqlite-priv.h"

MuStorageSQLite*
mu_storage_sqlite_new (const char* dbpath)
{
	MuStorageSQLite *storage;
	int result;

	if (!dbpath)
		return NULL;
	
	storage = (MuStorageSQLite*)malloc(sizeof(MuStorageSQLite));
	if (!storage)
		return NULL;
	
	memset (storage, 0, sizeof(MuStorageSQLite));

	/*
	 * by default, update/insert 100 messages per transaction
	 */
	storage->_transaction_size = 100;
	storage->_in_transaction   = FALSE;

	result = sqlite3_open (dbpath, &storage->_db);
	if (result  != SQLITE_OK) {
		g_warning ("failed to open db '%s'", dbpath);
		return NULL;
	}
	
	if (mu_storage_sqlite_init_db (storage) != MU_OK) {
		g_warning ("failed to initialize storage '%s'", dbpath);
		mu_storage_sqlite_destroy (storage);
		return NULL;
	}

	g_message ("%s: opened %s (sqlite version %d)",
		   __FUNCTION__, dbpath, sqlite3_libversion_number());


	return storage;
}

void
mu_storage_sqlite_destroy (MuStorageSQLite *storage)
{
	int i;

	if (!storage)
		return;

	/* if we were still in a transaction, commit uncommited changes */
	if (storage->_in_transaction) 
		if (!mu_storage_sqlite_commit_transaction(storage))
			g_warning ("%s: failed to commit last database changes",
				   __FUNCTION__);

	/* finalize prepared statements */
	for (i = 0; i != MU_SQL_PREP_QUERY_NUM; ++i) {
		if (!storage->_prepared[i])
			continue;
		if (sqlite3_finalize (storage->_prepared[i]) != SQLITE_OK) 
			g_warning ("failed to finalize stmt %d (%s)", i, 
				    sqlite3_errmsg(storage->_db));
	}
	
	/* close the db */
	if (storage->_db) {
		if (sqlite3_close(storage->_db) != SQLITE_OK) 
			g_warning ("%s: failed to close db: %s",
				 __FUNCTION__, sqlite3_errmsg(storage->_db));	
		else 
			g_message ("closed database");
	}
	
	free (storage);
}


void
mu_storage_sqlite_tune (MuStorageSQLite *storage, 
			unsigned int size,
			unsigned int synchronous, 
			unsigned int temp_store)
{
	g_return_if_fail (storage);

	if (size > 0) {
		g_message ("tune: set transaction size to %u", (unsigned int)size);
		storage->_transaction_size = size;
	}

	if (synchronous >= 0 && synchronous <= 2) {
		char *err, *q = g_strdup_printf ("PRAGMA synchronous = %u;", 
						 synchronous);
		g_message ("tune: set PRAGMA synchronous to %u", synchronous);
		if (sqlite3_exec(storage->_db, q, NULL, NULL, &err) != SQLITE_OK) {
			g_warning ("query '%s' failed: %s", q, err);
			sqlite3_free (err);
		}	
		g_free (q);
	}

	if (temp_store >= 0 && temp_store <= 2) {
		char *err, *q = g_strdup_printf ("PRAGMA temp_store = %u;", 
						 temp_store);
		g_message ("tune: set PRAGMA temp_store to %u", temp_store);
		if (sqlite3_exec(storage->_db, q, NULL, NULL, &err) != SQLITE_OK) {
			g_warning ("query '%s' failed: %s", q, err);
			sqlite3_free (err);
		}	
		g_free (q);
	}
} 


static sqlite3_int64
find_message_by_path (MuStorageSQLite *storage, const char* path,  
		      time_t *timestamp)
{
	sqlite_int64 msg_id = 0;
	const char* sql = 
		"SELECT id, tstamp FROM message "
		" WHERE mpath = ? LIMIT 1";
	sqlite3_stmt* stmt = mu_storage_sqlite_get_prepared 
		(storage, MU_SQL_PREP_FIND_MESSAGE_BY_PATH_1, sql);
	
	g_return_val_if_fail (storage, 0);
	g_return_val_if_fail (path, 0);

	mu_storage_sqlite_bind_string (stmt, 1, path);
	
	if (mu_storage_sqlite_step (storage, stmt) == SQLITE_ROW) { 
		msg_id = sqlite3_column_int64 (stmt, 0);
		if (timestamp)
			*timestamp = sqlite3_column_int64(stmt, 1);
	}
	mu_storage_sqlite_last_step (storage, stmt);

	return msg_id;
}


static sqlite3_int64
update_message_with_id (MuStorageSQLite *storage, MuMsgGMime *msg, sqlite3_int64 msg_id)
{
	int result, col = 0;
	const char* sql = 
		"UPDATE message SET "
		"msg_id=?,tstamp=?,"
		"mdate=?,msize=?,sender=?,recipients=?,cc=?,"
		"subject=?,flags=?,priority=? "
		"WHERE id = ?";
	sqlite3_stmt* stmt = mu_storage_sqlite_get_prepared 
		(storage, MU_SQL_PREP_UPDATE_MESSAGE_6, sql);
	
	g_return_val_if_fail (storage, 0);
	g_return_val_if_fail (msg_id != 0, 0);
	
	mu_storage_sqlite_bind_string (stmt, ++col, mu_msg_gmime_get_msgid(msg));
	mu_storage_sqlite_bind_int (stmt, ++col, mu_msg_gmime_get_timestamp(msg));
	mu_storage_sqlite_bind_int (stmt, ++col, mu_msg_gmime_get_date(msg));
	mu_storage_sqlite_bind_int (stmt, ++col,  mu_msg_gmime_get_file_size(msg));
	mu_storage_sqlite_bind_string (stmt, ++col, mu_msg_gmime_get_from(msg));
	mu_storage_sqlite_bind_string (stmt, ++col, mu_msg_gmime_get_to(msg));
	mu_storage_sqlite_bind_string (stmt, ++col, mu_msg_gmime_get_cc(msg));
	mu_storage_sqlite_bind_string (stmt, ++col, mu_msg_gmime_get_subject(msg));
	mu_storage_sqlite_bind_int (stmt, ++col, mu_msg_gmime_get_flags(msg)); 
	mu_storage_sqlite_bind_int (stmt, ++col, mu_msg_gmime_get_priority(msg));
 	mu_storage_sqlite_bind_int (stmt, ++col, msg_id);
	
	result = mu_storage_sqlite_single_step (storage, stmt, NULL);
	if (result != SQLITE_DONE) {
		g_warning ("%s failed for id %d (%d)", 
			    __FUNCTION__, (int)msg_id, result);  
		return 0;
	} else
		return msg_id;
}


static sqlite3_int64
insert_message (MuStorageSQLite *storage, MuMsgGMime *msg)
{
	sqlite3_int64 id;
	int result, col = 0;
	const char* sql = 
		"INSERT INTO message "
		"(msg_id,tstamp,mpath,mdate,"
                " msize,sender,recipients,cc,subject,flags,priority) "
		"VALUES (?,?,?,?,?,?,?,?,?,?,?)";
	sqlite3_stmt* stmt = mu_storage_sqlite_get_prepared 
		(storage, MU_SQL_PREP_INSERT_MESSAGE_11, sql);

	g_return_val_if_fail (storage, 0);
	g_return_val_if_fail (stmt, 0);
	
	mu_storage_sqlite_bind_string (stmt, ++col, mu_msg_gmime_get_msgid(msg));
	mu_storage_sqlite_bind_int (stmt, ++col, mu_msg_gmime_get_timestamp(msg));
	mu_storage_sqlite_bind_string (stmt, ++col, mu_msg_gmime_get_path(msg));
	mu_storage_sqlite_bind_int (stmt, ++col, mu_msg_gmime_get_date(msg));
	mu_storage_sqlite_bind_int (stmt, ++col,  mu_msg_gmime_get_file_size(msg));
	mu_storage_sqlite_bind_string (stmt, ++col, mu_msg_gmime_get_from(msg));
	mu_storage_sqlite_bind_string (stmt, ++col, mu_msg_gmime_get_to(msg));
	mu_storage_sqlite_bind_string (stmt, ++col, mu_msg_gmime_get_cc(msg));
	mu_storage_sqlite_bind_string (stmt, ++col, mu_msg_gmime_get_subject(msg));
	mu_storage_sqlite_bind_int (stmt, ++col, mu_msg_gmime_get_flags(msg)); 
	mu_storage_sqlite_bind_int (stmt, ++col, mu_msg_gmime_get_priority(msg));

	result = mu_storage_sqlite_single_step (storage, stmt, &id);
	
	if (result != SQLITE_DONE)
		return 0;

	return id;
}

static sqlite3_int64
create_or_update_message (MuStorageSQLite *storage, MuMsgGMime *msg)
{
	sqlite3_int64 msg_id = 0;

	g_return_val_if_fail (msg, 0);

	msg_id = find_message_by_path (storage, mu_msg_gmime_get_path(msg),
				       NULL);
	if (msg_id == 0)
		return insert_message (storage, msg);
	else 
		return update_message_with_id (storage, msg, msg_id);
}

static sqlite3_stmt*
get_find_contact_name_addr (MuStorageSQLite *storage, 
			       const char* name, const char* addr)
{
	sqlite3_stmt* stmt;
	const char* sql = "SELECT id FROM contact WHERE "
		"cname = ? AND address = ? LIMIT 1";
	
	stmt = mu_storage_sqlite_get_prepared 
		(storage, MU_SQL_PREP_FIND_CONTACT_BY_NAME_ADDR_2, sql);
	g_return_val_if_fail (stmt, NULL);
	
	mu_storage_sqlite_bind_string (stmt, 1, name); 
	mu_storage_sqlite_bind_string (stmt, 2, addr);

	return stmt;
}


static sqlite3_stmt*
get_find_contact_name (MuStorageSQLite *storage, const char* name)
{
	sqlite3_stmt* stmt;
	const char* sql = "SELECT id FROM contact WHERE "
		"cname = ? LIMIT 1";
	
	stmt = mu_storage_sqlite_get_prepared 
		(storage, MU_SQL_PREP_FIND_CONTACT_BY_NAME_1, sql);
	g_return_val_if_fail (stmt, NULL);
	
	mu_storage_sqlite_bind_string (stmt, 1, name); 
	
	return stmt;
}


static sqlite3_stmt*
get_find_contact_addr (MuStorageSQLite *storage, const char* addr)
{
	sqlite3_stmt* stmt;
	const char* sql = "SELECT id FROM contact WHERE "
		"address = ? LIMIT 1";
	
	stmt = mu_storage_sqlite_get_prepared 
		(storage, MU_SQL_PREP_FIND_CONTACT_BY_ADDR_1, sql);
	g_return_val_if_fail (stmt, NULL);
	
	mu_storage_sqlite_bind_string (stmt, 1, addr); 
	
	return stmt;
}


static sqlite3_int64
find_contact (MuStorageSQLite *storage, 
	      const char* name, const char* addr)
{
	sqlite3_int64 cid = 0;
	sqlite3_stmt* stmt = NULL;
	
	g_return_val_if_fail (storage, 0);
	
	if (name && addr && strlen(name) && strlen(addr))
		stmt = get_find_contact_name_addr (storage, name, addr);
	else if (name && strlen(name))
		stmt = get_find_contact_name (storage, name);
	else if (addr && strlen(addr))
		stmt = get_find_contact_addr (storage, addr);
	else 
		g_warning ("%s: invalid parameters", __FUNCTION__);
	
	g_return_val_if_fail (stmt, 0);
	
	if (mu_storage_sqlite_step (storage, stmt) == SQLITE_ROW)
		cid = sqlite3_column_int64 (stmt, 0);

	mu_storage_sqlite_last_step (storage, stmt);

	return cid;
}


static sqlite3_int64
insert_contact (MuStorageSQLite *storage, const char* name, const char* addr)
{
	int result;
	sqlite3_int64 cid = 0;
	const char* sql = 
		"INSERT INTO contact (cname,address) "
		"VALUES (?,?)";
	sqlite3_stmt* stmt = mu_storage_sqlite_get_prepared 
		(storage, MU_SQL_PREP_INSERT_CONTACT_2, sql);
	if (!stmt)
		return 0;	

	g_return_val_if_fail (storage, 0);
	g_return_val_if_fail (stmt, 0);
	g_return_val_if_fail (addr||name, 0);

	mu_storage_sqlite_bind_string (stmt, 1, name); 
	mu_storage_sqlite_bind_string (stmt, 2, addr);
	
	result = mu_storage_sqlite_single_step (storage, stmt, &cid);
	if (result != SQLITE_DONE) {
		g_warning ("%s: failed", __FUNCTION__);
		return 0;
	}

	return cid;
}


static sqlite3_int64
insert_contact_if_new (MuStorageSQLite *storage, const char* name, const char* addr)
{
	sqlite3_int64 cid;

	cid = find_contact (storage, name, addr);

	if (cid == 0) /* does not exist yet */
		cid = insert_contact (storage, name, addr);

	return cid;
}

static sqlite3_int64
insert_message_contact_if_new (MuStorageSQLite *storage, sqlite3_int64 msg_id, 
			       sqlite3_int64 contact_id, 
			       MuMsgContactType type)
{
	int result;
	sqlite3_int64 mc_id;
	const char* sql = 
		"INSERT INTO message_contact "
		"      (message_id,contact_id,contact_type_id) "
		"VALUES (?,?,?)";
	sqlite3_stmt* stmt = mu_storage_sqlite_get_prepared 
		(storage, MU_SQL_PREP_INSERT_MESSAGE_CONTACT_3, sql);
	
	g_return_val_if_fail (storage, 0);
	g_return_val_if_fail (msg_id, 0);
	g_return_val_if_fail (contact_id, 0);
	
	mu_storage_sqlite_bind_int (stmt, 1, msg_id);
	mu_storage_sqlite_bind_int (stmt, 2, contact_id);
	mu_storage_sqlite_bind_int (stmt, 3, type);

	result = mu_storage_sqlite_single_step (storage, stmt, &mc_id);
	if (result != SQLITE_DONE) {
		g_warning ("%s: failed", __FUNCTION__);
		return 0;
	}
	
	return mc_id;
}



struct _CallbackInfo {
	MuStorageSQLite *_storage;
	MuMsgGMime      *_info;
	sqlite3_int64    _msg_id;
};
typedef struct _CallbackInfo CallbackInfo;


static MuResult 
msginfo_contact_cb (MuMsgContact *contact, CallbackInfo *cbinfo)
{
	sqlite3_int64 cid, mid;
	
	cid = insert_contact_if_new (cbinfo->_storage,
				     contact->_name,
				     contact->_addr);
	if (cid == 0) {
		g_warning ("%s: insert_contact_if_new failed",__FUNCTION__);
		return MU_ERROR;
	}

	mid  = insert_message_contact_if_new (cbinfo->_storage, 
					      cbinfo->_msg_id, 
					      cid, 
					      contact->_type);
	if (mid == 0) {
		g_warning ("%s: insert_message_contact_if_new failed",
			    __FUNCTION__);
		return MU_ERROR;
	}

	return MU_OK; 
}



static MuResult
insert_new_contacts_for_message (MuStorageSQLite *storage, MuMsgGMime *msg, 
				 sqlite3_int64 msg_id)
{
	MuResult result;
	CallbackInfo cbinfo = { storage, msg, msg_id };
	
	result = mu_msg_gmime_get_contacts_foreach 
		(msg, 
		 (MuMsgGMimeContactsCallback)msginfo_contact_cb, 
		 &cbinfo);
	if (result ==MU_OK)
		return MU_OK;
	else 
		return MU_ERROR;
}


int64_t
mu_storage_sqlite_store (MuStorageSQLite *storage, MuMsgGMime *msg)
{
	sqlite3_int64 msg_id;
	static int processed = 10;

	g_return_val_if_fail (storage, MU_ERROR);
	g_return_val_if_fail (msg, MU_ERROR);
	g_return_val_if_fail (g_path_is_absolute (mu_msg_gmime_get_path(msg)), 
			      MU_ERROR);
	
	if (!storage->_in_transaction)
		if (!mu_storage_sqlite_begin_transaction(storage))
			return -1;

	msg_id = create_or_update_message (storage, msg);
	
	g_debug("processed %s", mu_msg_gmime_get_path (msg));
	
	if (msg_id) { 
		/* we only commit for every <transaction size> messages;
		*  for optimization reasons */
		insert_new_contacts_for_message (storage, msg, msg_id);
		if ((++processed % storage->_transaction_size) == 0)
			if (!mu_storage_sqlite_commit_transaction(storage))
				return -1;

		return (int64_t)msg_id;
	} else {
		mu_storage_sqlite_rollback_transaction (storage);
		return -1;
	}
}

time_t
mu_storage_sqlite_message_timestamp (MuStorageSQLite *storage, 
				     const char *fullpath)
{
	time_t t;
	sqlite3_int64 mid;

	mid = find_message_by_path (storage, fullpath, &t);	
	
	return mid != 0 ? t : 0;
}

struct _MuStorageSQLiteCleanupData {
	MuStorageSQLiteCleanupCallback _cb;
	void                           *_data;
};
typedef struct _MuStorageSQLiteCleanupData MuStorageSQLiteCleanupData;


static MuResult
remove_msg (MuStorageSQLite *storage, sqlite3_int64 id)
{
	int result;
	const char* sql=
		"DELETE FROM message WHERE id=?";
	sqlite3_stmt* stmt = mu_storage_sqlite_get_prepared
		(storage, MU_SQL_PREP_REMOVE_MSG_1, sql);
	if (!stmt)
		return MU_ERROR;
	
	mu_storage_sqlite_bind_int64 (stmt, 1, id);
	
	result = mu_storage_sqlite_single_step (storage, stmt, NULL);
	if (result != SQLITE_DONE) {
		g_warning ("%s failed for id %d (%d)", 
			    __FUNCTION__, (int)id, result);  
		return MU_ERROR;
	}

	return MU_OK;
}


MuResult 
mu_storage_sqlite_cleanup (MuStorageSQLite *storage, 
			   MuStorageSQLiteCleanupCallback cb, 
			   void *data)
{
	MuResult result = MU_OK;
	const char* sql= "SELECT m.id, m.mpath FROM message m";
	sqlite3_stmt* stmt = mu_storage_sqlite_get_prepared
		(storage, MU_SQL_PREP_GET_MESSAGE_PATHS_0, sql);
	
	g_return_val_if_fail (storage && cb, MU_ERROR);
	
	while (mu_storage_sqlite_step (storage, stmt) == SQLITE_ROW) {
		MuMsgStatus status;
		sqlite3_int64 id = sqlite3_column_int64 (stmt, 0);
		const char* mpath = (const char*)sqlite3_column_text (stmt, 1);

		if (access(mpath, F_OK) == 0) 
			status = MU_MSG_STATUS_EXISTS; /* file exists */
		else { 
			status = MU_MSG_STATUS_CLEANUP;  /* file does not exist */
			g_message ("%s does not exist", mpath);
		}
		
		if (cb) {
			result = cb (status, mpath, data);
			if (result != MU_OK) {
				g_message ("%s: %s occured", __FUNCTION__,
					   result == MU_STOP ? "stop" : "error");
				break; /* stop or error */
			}
		}

		/* we want to remove, and callback did not complain */
		if (status == MU_MSG_STATUS_CLEANUP) {
			result = remove_msg (storage, id);
			if (result != MU_OK) {
				g_warning ("error deleting %ld (%s)", 
					   (long int)id, mpath);
				break;
			}
			/* call the callback once more to tell deletion succeeded */
			result = cb (MU_MSG_STATUS_CLEANED_UP, mpath, data);
		}
	}

	mu_storage_sqlite_last_step (storage, stmt);
	return result;
}








