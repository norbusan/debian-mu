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

#ifndef __MU_STORAGE_PRIV_H__
#define __MU_STORAGE_PRIV_H__

#include <sqlite3.h>

enum _MuStoragePrepQueryID {
	MU_SQL_PREP_FILL_CONTACT_CAT_2 = 0,
	MU_SQL_PREP_FIND_CONTACT_BY_NAME_ADDR_2,
	MU_SQL_PREP_FIND_CONTACT_BY_NAME_1,
	MU_SQL_PREP_FIND_CONTACT_BY_ADDR_1,
	MU_SQL_PREP_GET_MESSAGE_PATHS_0,
	MU_SQL_PREP_INSERT_CONTACT_2,
	MU_SQL_PREP_INSERT_FOLDER_1,
	MU_SQL_PREP_INSERT_MESSAGE_11,
	MU_SQL_PREP_INSERT_MESSAGE_CONTACT_3,
	MU_SQL_PREP_FIND_MESSAGE_BY_PATH_1,
	MU_SQL_PREP_UPDATE_FOLDER_2,
	MU_SQL_PREP_UPDATE_MESSAGE_6,
	MU_SQL_PREP_REMOVE_MSG_1,
	MU_SQL_PREP_QUERY_NUM
};
typedef enum _MuStoragePrepQueryID MuStoragePrepQueryID;

struct _MuStorageSQLite {
	sqlite3      *_db;
	sqlite3_stmt * _prepared[MU_SQL_PREP_QUERY_NUM]; 
	unsigned int  _transaction_size; /* how many msg updates/inserts are in a 
					  * transaction? */ 
	gboolean      _in_transaction;
};

typedef sqlite_int64 sqlite3_int64;

/* helpers --> implemented in storage-helpers.c */
sqlite3_stmt* mu_storage_sqlite_prepare_stmt (MuStorageSQLite *storage, 
				       const char* sql);

sqlite3_stmt* mu_storage_sqlite_get_prepared (MuStorageSQLite *storage, 
				       MuStoragePrepQueryID id, 
				       const char* sql);
int           mu_storage_sqlite_single_step  (MuStorageSQLite *storage, 
				       sqlite3_stmt *stmt, 
				       sqlite_int64* rowid);
int           mu_storage_sqlite_step         (MuStorageSQLite *storage,
				       sqlite3_stmt *stmt);
int           mu_storage_sqlite_last_step    (MuStorageSQLite *storage,
				       sqlite3_stmt *stmt);

int           mu_storage_sqlite_bind_int     (sqlite3_stmt *stmt, int n, int i);
int           mu_storage_sqlite_bind_string  (sqlite3_stmt *stmt, int n, 
				       const char* str);
int           mu_storage_sqlite_bind_int64   (sqlite3_stmt *stmt, int n, 
				       sqlite_int64 i);

gboolean     mu_storage_sqlite_begin_transaction (MuStorageSQLite *storage);
gboolean     mu_storage_sqlite_commit_transaction (MuStorageSQLite *storage);
gboolean     mu_storage_sqlite_rollback_transaction (MuStorageSQLite *storage);

/* init --> implemented in storage-init.c */
int mu_storage_sqlite_init_db          (MuStorageSQLite *storage);
int mu_storage_sqlite_init_contact_cat (MuStorageSQLite *storage);




#endif /*__MU_STORAGE_SQLITE_PRIV_H__*/
