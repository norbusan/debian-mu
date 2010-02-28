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
#include <stdlib.h>
#include <unistd.h>

#include "mu/mu-msg-str.h"

#include "mu-msg-sqlite.h"
#include "mu-msg-sqlite-priv.h"
#include "mu-expr-type.h"


struct _MuMsgSQLite {
	sqlite3 *_db;
	sqlite3_stmt *_stmt;
};

MuMsgSQLite*
mu_msg_sqlite_new (sqlite3 *db, sqlite3_stmt *stmt)
{
	MuMsgSQLite *row;

	g_return_val_if_fail (stmt, NULL);

	row = (MuMsgSQLite*)malloc(sizeof(MuMsgSQLite));
	if (!row)
		return NULL;

	row->_db   = db;
	row->_stmt = stmt;

	return row;
}

void
mu_msg_sqlite_destroy (MuMsgSQLite *row)
{
	if (row) {
		sqlite3_finalize (row->_stmt);
		free (row);
	}
}

gboolean 
mu_msg_sqlite_next (MuMsgSQLite *row, GError **err)
{
	int result;
	int retry = 3;

	g_return_val_if_fail (row, FALSE);

	/* do the rather ugly usleep/retry; it greatly improves reliability
	 * when mu-index is running concurrently...
	 */
	do {
		result = sqlite3_step (row->_stmt);
	} while (result == SQLITE_BUSY && retry-- > 0 && usleep(5000)==0);
	
	if (result == SQLITE_DONE)
		return FALSE; /* we're done */
		
	if (result != SQLITE_ROW) {
		g_set_error (err, 0, 0,
			     "failed to step statement: %s",
			     sqlite3_errmsg(row->_db));
		return FALSE;
	}
	return TRUE;
}

int
mu_msg_sqlite_get_id (MuMsgSQLite *row)
{
	return sqlite3_column_int64(row->_stmt, MU_EXPR_TYPE_ID);
}


const char* 
mu_msg_sqlite_get_msgid (MuMsgSQLite *row)
{
	return (const char*)sqlite3_column_text(row->_stmt, 
						MU_EXPR_TYPE_MSG_ID);
}


time_t
mu_msg_sqlite_get_timestamp (MuMsgSQLite *row)
{
	return (time_t)sqlite3_column_int(row->_stmt, MU_EXPR_TYPE_TIMESTAMP);
}


const char*
mu_msg_sqlite_get_path (MuMsgSQLite *row)
{
	return (const char*)sqlite3_column_text(row->_stmt, 
						MU_EXPR_TYPE_PATH);
}


time_t
mu_msg_sqlite_get_date (MuMsgSQLite *row)
{
	return (time_t)sqlite3_column_int(row->_stmt, MU_EXPR_TYPE_DATE);
} 


size_t
mu_msg_sqlite_get_size (MuMsgSQLite *row)
{
	return (size_t)sqlite3_column_int(row->_stmt, MU_EXPR_TYPE_SIZE);
}

const char*
mu_msg_sqlite_get_from (MuMsgSQLite *row)
{
	return (const char*)sqlite3_column_text(row->_stmt, MU_EXPR_TYPE_FROM);
}


const char*
mu_msg_sqlite_get_to (MuMsgSQLite *row)
{
	return (const char*)sqlite3_column_text(row->_stmt, MU_EXPR_TYPE_TO);
}


const char* 
mu_msg_sqlite_get_cc (MuMsgSQLite *row)
{
	return (const char*)sqlite3_column_text(row->_stmt, MU_EXPR_TYPE_CC);
}


const char* 
mu_msg_sqlite_get_subject (MuMsgSQLite *row)
{
	return (const char*)sqlite3_column_text(row->_stmt, 
						MU_EXPR_TYPE_SUBJECT);
}


MuMsgFlags
mu_msg_sqlite_get_flags (MuMsgSQLite *row)
{
	return sqlite3_column_int(row->_stmt, MU_EXPR_TYPE_FLAGS);
}


MuMsgPriority
mu_msg_sqlite_get_priority (MuMsgSQLite *row)
{
	return sqlite3_column_int(row->_stmt, MU_EXPR_TYPE_PRIORITY);

}

static const char*
str_field_s (MuMsgSQLite *row, char fieldchar)
{
	const char* str;
	static char buf[2] = { '\0', '\0' };
	
	switch (fieldchar) {
	case 't': str = mu_msg_sqlite_get_to (row); break;
	case 'c': str = mu_msg_sqlite_get_cc (row); break;
	case 'f': str = mu_msg_sqlite_get_from(row); break;
	case 's': str = mu_msg_sqlite_get_subject (row); break;
	case 'm': str = mu_msg_sqlite_get_msgid (row); break;
	case 'F': str = mu_msg_str_flags_s(
		mu_msg_sqlite_get_flags (row)); break;
	case 'd': str = mu_msg_str_date_s(mu_msg_sqlite_get_date(row)); break;
	case 'z': str = mu_msg_str_size_s(mu_msg_sqlite_get_size(row)); break;
	case 'p': str = mu_msg_sqlite_get_path(row); break;
	default:
		buf[0] = fieldchar;
		return buf;
	}
	
	return str ? str : "";
}


gchar* 
mu_msg_sqlite_to_string (MuMsgSQLite *row, const char* rowformat)
{
	GString *str;
	const char* c;

	g_return_val_if_fail (row, NULL);

	if (!rowformat)
		return NULL;

	str = g_string_sized_new (512);

	c = rowformat;
	while (c[0]) {
		g_string_append (str, str_field_s (row, c[0]));
		++c;
	}
	
	return g_string_free (str, FALSE);
}
