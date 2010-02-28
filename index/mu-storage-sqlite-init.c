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

const static char* MU_SQL_DEFINITION_SQL = 
#include "mu-storage.sql.h"
;

int
mu_storage_sqlite_init_contact_cat (MuStorageSQLite *storage)
{
	int i, result;
	const char* sql =
		"INSERT OR IGNORE INTO contact_type (id,descr)"
		" VALUES (?,?);";
	sqlite3_stmt* stmt = mu_storage_sqlite_get_prepared 
		(storage, MU_SQL_PREP_FILL_CONTACT_CAT_2, sql);

	struct { 
		int        _id; 
		const char* _descr; 
	} cats[] = {
		{ MU_MSG_CONTACT_TYPE_TO,   "to" },
		{ MU_MSG_CONTACT_TYPE_CC,   "cc" },
		{ MU_MSG_CONTACT_TYPE_BCC,  "bcc" },
		{ MU_MSG_CONTACT_TYPE_FROM, "from"}
	};

	g_return_val_if_fail (storage, SQLITE_ERROR);
	g_return_val_if_fail (stmt, SQLITE_ERROR);
	
		
	result = SQLITE_DONE;
	for (i = 0; i != sizeof(cats)/sizeof(cats[0]); ++i) {
		
		result = mu_storage_sqlite_bind_int (stmt, 1, cats[i]._id);
		if (result != SQLITE_DONE)
			break;
		
		result = mu_storage_sqlite_bind_string (stmt, 2, cats[i]._descr);
		if (result != SQLITE_DONE)
			break;
		
		result = mu_storage_sqlite_single_step (storage, stmt, NULL);
		if (result != SQLITE_DONE)
			break;
	}	

	return result;
}


int
mu_storage_sqlite_init_db (MuStorageSQLite *storage)
{
	char *err;
	int result;

	g_return_val_if_fail (storage, SQLITE_ERROR);
	
	/* let there be light! */
	result = sqlite3_exec(storage->_db, MU_SQL_DEFINITION_SQL,
			      NULL, NULL, &err);
	
	if (result != SQLITE_OK) {
		g_warning ("failed to init db: %s (%s)\n", 
			   sqlite3_errmsg(storage->_db), err);
		sqlite3_free (err);
		return result;
	} 
	
	return mu_storage_sqlite_init_contact_cat (storage);
}

sqlite3* 
mu_storage_sqlite_get_db (MuStorageSQLite *storage)
{
	g_return_val_if_fail (storage, NULL);

	return storage->_db;
}
