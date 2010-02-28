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

#include <stdlib.h>
#include <stdio.h>
#include <glib.h>

#include "mu/mu.h"

#include "mu-storage-sqlite.h"
#include "mu-storage-sqlite-priv.h"


sqlite3_stmt*
mu_storage_sqlite_prepare_stmt (MuStorageSQLite *storage, const char* sql)
{
	int result;
	sqlite3_stmt *stmt;

	g_return_val_if_fail (storage, NULL);
	g_return_val_if_fail (sql, NULL);
	
	result = sqlite3_prepare_v2(storage->_db, sql, -1, &stmt, NULL);
	if (result != SQLITE_OK) {
		g_warning ("failed to prepare statement: %s",
			   sqlite3_errmsg(storage->_db));
		g_debug ("failed statement: %s", sql);
		return NULL;
	}
	
	mu_storage_sqlite_last_step (storage, stmt);
	
	return stmt;
}

sqlite3_stmt*
mu_storage_sqlite_get_prepared (MuStorageSQLite *storage, 
				MuStoragePrepQueryID id, const char* sql)
{
	g_return_val_if_fail (storage, NULL);
	g_return_val_if_fail (id >= 0 && id < MU_SQL_PREP_QUERY_NUM, NULL);
	g_return_val_if_fail (sql, NULL);

	if (storage->_prepared[id])
		return storage->_prepared[id];
	else 
		return storage->_prepared[id] = 
			mu_storage_sqlite_prepare_stmt (storage, sql);
}


int
mu_storage_sqlite_single_step (MuStorageSQLite *storage, sqlite3_stmt *stmt,
			       sqlite3_int64* rowid)
{
	int result;

	g_return_val_if_fail (storage, SQLITE_ERROR);
	g_return_val_if_fail (stmt, SQLITE_ERROR);

	result = sqlite3_step (stmt);
	if (result != SQLITE_DONE) 
		g_warning ("failed to execute statement: %s",
			   sqlite3_errmsg(storage->_db));
	else if (rowid) 
		*rowid = sqlite3_last_insert_rowid (storage->_db);
		
	if (mu_storage_sqlite_last_step (storage, stmt) != SQLITE_OK)
		g_warning ("mu_storage_sqlite_last_step failed: %s",
			    sqlite3_errmsg(storage->_db));

	return result; 
}


int
mu_storage_sqlite_step (MuStorageSQLite *storage, sqlite3_stmt *stmt)
{
	int result;
	
	g_return_val_if_fail (storage, SQLITE_ERROR);
	g_return_val_if_fail (stmt, SQLITE_ERROR);

	result = sqlite3_step (stmt);
	if (result != SQLITE_DONE && result != SQLITE_ROW) 
		g_warning ("failed to step statement: %s",
			    sqlite3_errmsg(storage->_db));

	return result;
}


int
mu_storage_sqlite_last_step (MuStorageSQLite *storage, sqlite3_stmt *stmt)
{
	int result;
	
	g_return_val_if_fail (storage, SQLITE_ERROR);
	g_return_val_if_fail (stmt, SQLITE_ERROR);

	result = sqlite3_reset (stmt);
	if (result != SQLITE_OK) {
		g_warning ("failed to reset statement: %s",
			    sqlite3_errmsg(storage->_db));
		return result;
	}

	result = sqlite3_clear_bindings (stmt);
	if (result != SQLITE_OK) 
		g_warning ("failed to clear bindings: %s",
			    sqlite3_errmsg(storage->_db));
	
	return result;
}


int
mu_storage_sqlite_bind_int (sqlite3_stmt *stmt, int n, int i)
{
	int result;

	g_return_val_if_fail (stmt, SQLITE_ERROR);
	g_return_val_if_fail (n != 0, SQLITE_ERROR);
	
	if ((result = sqlite3_bind_int (stmt, n, i)) != SQLITE_OK)
		g_warning ("failed to bind int to param %d", n);
	
	return result;
}


int 
mu_storage_sqlite_bind_int64 (sqlite3_stmt *stmt, int n, sqlite3_int64 i)
{
	int result;

	g_return_val_if_fail (stmt, SQLITE_ERROR);
	g_return_val_if_fail (n != 0, SQLITE_ERROR);

	if ((result = sqlite3_bind_int64 (stmt, n, i)) != SQLITE_OK)
		g_warning ("failed to bind int64 to param %d", n);
	
	return result;
}


int 
mu_storage_sqlite_bind_string (sqlite3_stmt *stmt, int n, const char* str)
{
	int result;
		
	g_return_val_if_fail (stmt, SQLITE_ERROR);
	g_return_val_if_fail (n != 0, SQLITE_ERROR);

	if ((result = sqlite3_bind_text (stmt, n, str, -1, SQLITE_STATIC))	
	    != SQLITE_OK) 
		g_warning ("failed to bind string to param %d", n);
	
	return result;
}



gboolean
mu_storage_sqlite_begin_transaction (MuStorageSQLite *storage)
{
	gchar *err;

	g_return_val_if_fail (storage, FALSE);
	g_return_val_if_fail (!storage->_in_transaction, FALSE);

	if (sqlite3_exec(storage->_db, "BEGIN TRANSACTION;",
			 NULL, NULL, &err) != SQLITE_OK) {
		g_warning ("failed to start transaction: %s", err);
		sqlite3_free (err);
		return FALSE;
	}
	storage->_in_transaction = TRUE;
	return TRUE;

}

gboolean
mu_storage_sqlite_commit_transaction (MuStorageSQLite *storage)
{
	gchar *err;
	
	g_return_val_if_fail (storage, FALSE);
	g_return_val_if_fail (storage->_in_transaction, FALSE);
		
	if (sqlite3_exec(storage->_db, "COMMIT;",
			 NULL, NULL, &err) != SQLITE_OK) {
		g_warning ("failed to commit transaction: %s", err);
		sqlite3_free (err);
		return FALSE;
	}
	storage->_in_transaction = FALSE;
	return TRUE;

}

gboolean
mu_storage_sqlite_rollback_transaction (MuStorageSQLite *storage)
{
	gchar *err;

	g_return_val_if_fail (storage, FALSE);
	g_return_val_if_fail (storage->_in_transaction, FALSE);

	if (sqlite3_exec(storage->_db, "ROLLBACK;",
			 NULL, NULL, &err) != SQLITE_OK) {
		g_warning ("failed to rollback transaction: %s", err);
		sqlite3_free (err);
		return FALSE;
	}
	storage->_in_transaction = FALSE;
	return TRUE;
}
