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

#include <sqlite3.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <glib/gstdio.h>
#include "mu-msg-sqlite.h"
#include "mu-msg-sqlite-priv.h"
#include "mu-expr-error.h"

#include "mu-query-sqlite.h"

struct _MuQuerySQLite {
	sqlite3 *_db;
};

MuQuerySQLite *
mu_query_sqlite_new (const char* dbpath, GError **err)
{
	int result;
	MuQuerySQLite *query;

	g_return_val_if_fail (dbpath, NULL);

	if (g_access (dbpath, R_OK) != 0) { 
		g_set_error (err, 0, MU_EXPR_ERROR_SQLITE_INIT,
			     "'%s' is not a readable sqlite db", dbpath);
		return NULL;
	}

	query = (MuQuerySQLite*)malloc(sizeof(MuQuerySQLite));
	if (!query)
		return NULL;

	result = sqlite3_open (dbpath, &query->_db);
	/* unfortunately, sqlite3_open_v2 is for sqlite >= 3.5 */
	/*   result = sqlite3_open_v2 (dbpath,&mgr->_db,  */
	/* 			  SQLITE_OPEN_READONLY, */
	/* 			  NULL); */
	if (result != SQLITE_OK) {
		g_set_error (err, 0, MU_EXPR_ERROR_SQLITE_INIT,
			     "failed to open db '%s'", dbpath);
		return NULL;
	}
	
	g_message ("opened database '%s' (sqlite version %d)",
		   dbpath, sqlite3_libversion_number());
	
	return query;
}

void
mu_query_sqlite_destroy (MuQuerySQLite *query)
{
	if (!query)
		return;
 
	if (sqlite3_close(query->_db) != SQLITE_OK)
		g_warning ("%s: failed to close db: %s",
			   __FUNCTION__, sqlite3_errmsg(query->_db));
	else
		g_message ("closed database");
	
	free (query);
}


MuMsgSQLite* 
mu_query_sqlite_run (MuQuerySQLite *query, const char *sql, GError **err)
{
	sqlite3_stmt *stmt;
	int result;
	int retry = 3;
	
	g_return_val_if_fail (query && query->_db, NULL);
	
	/* do the rather ugly usleep/retry; it greatly improves reliability
	 * when mu-index is running concurrently...
	 */
	do {
		result = sqlite3_prepare_v2 (query->_db, sql, -1, &stmt, NULL);
	} while (result == SQLITE_BUSY && retry-- > 0 && usleep(5000)==0);
	
	if (result != SQLITE_OK) {
		g_set_error (err, 0, 0,
			     "failed to prepare statement: %s",
			     sqlite3_errmsg(query->_db));
		return NULL;
	}
	
	return mu_msg_sqlite_new (query->_db, stmt);
}

gboolean 
mu_query_sqlite_exec (MuQuerySQLite *query, const char *sql, GError **err)
{	
	g_return_val_if_fail (query && query->_db, FALSE);

	if (sqlite3_exec (query->_db, sql, NULL, NULL, NULL) != SQLITE_OK) {
		g_set_error (err, 0, 0,
			     "failed to exec statement: %s",
			     sqlite3_errmsg(query->_db));
		return FALSE;
	}
	return TRUE;
}


enum _MuStatType {
	OLDEST,
	NEWEST,
	BIGGEST
};
typedef enum _MuStatType MuStatType;


static gboolean
get_newest_oldest_biggest_messages (MuQuerySQLite *query,
				    const char *tmpview,
				    uint64_t* nums, 
				    MuStatType type)
{
	char* sql;
	sqlite3_stmt *stmt = NULL;
	int result, i;

	switch (type) {
	case OLDEST:
		sql =g_strdup_printf (
			"SELECT \"m.id\" FROM %s "
			"ORDER BY \"m.mdate\" ASC LIMIT %d;",
			tmpview, MU_QUERY_STATS_TOP);
		break;
	case NEWEST:
		sql =g_strdup_printf (
			"SELECT \"m.id\" FROM %s "
			"ORDER BY \"m.mdate\" DESC LIMIT %d;",
			tmpview, MU_QUERY_STATS_TOP);
		break;
	case BIGGEST:
		sql =g_strdup_printf (
			"SELECT \"m.id\" FROM %s "
			"ORDER BY \"m.msize\" DESC LIMIT %d;",
			tmpview, MU_QUERY_STATS_TOP);
		break;
	default:
		g_return_val_if_reached (FALSE);
	}
	
	result = sqlite3_prepare_v2(query->_db, sql, -1, &stmt, NULL);
	g_free (sql);
	if (result != SQLITE_OK)	
		return FALSE;
			
	i = 0;
	while (sqlite3_step (stmt) == SQLITE_ROW) 
		nums[i++] = sqlite3_column_int64 (stmt, 0);

	if (!sqlite3_finalize (stmt)==SQLITE_OK)
		return FALSE;
		
	return TRUE;
}


static gboolean
get_popular_contacts (MuQuerySQLite *query,
		      const char *tmpview,
		      uint64_t* nums, 
		      MuMsgContactType ctype)
{
	char* sql;
	sqlite3_stmt *stmt = NULL;
	int result, i;

	sql = g_strdup_printf (
		"SELECT c.id, COUNT(*) AS num\n"
		"FROM contact c, message_contact mc, %s v\n"
		"WHERE v.\"m.id\" = mc.message_id AND c.id = mc.contact_id\n"
		"AND   mc.contact_type_id = %d\n"
		"GROUP BY c.id ORDER BY num DESC LIMIT %d\n",
		tmpview, ctype, MU_QUERY_STATS_TOP);
		
	result = sqlite3_prepare_v2(query->_db, sql, -1, &stmt, NULL);
	g_free (sql);
	if (result != SQLITE_OK)	
		return FALSE;
			
	i = 0;
	while (sqlite3_step (stmt) == SQLITE_ROW) 
		nums[i++] = sqlite3_column_int64 (stmt, 0);

	if (!sqlite3_finalize (stmt)==SQLITE_OK)
		return FALSE;
		
	return TRUE;
}


static gboolean
get_count_avg (MuQuerySQLite *query, const char *tmpview,
	       size_t *count, size_t *size)
{
	char* sql;
	sqlite3_stmt *stmt = NULL;
	int result, i;

	sql = g_strdup_printf (
		"SELECT count(*),avg(\"m.msize\")\n"
		"FROM  %s\n", tmpview);
		
	result = sqlite3_prepare_v2(query->_db, sql, -1, &stmt, NULL);
	g_free (sql);
	if (result != SQLITE_OK)	
		return FALSE;
			
	i = 0;
	result = sqlite3_step (stmt);
	if (result == SQLITE_ROW) {
		*count = (size_t)sqlite3_column_int(stmt, 0);
		*size  = (size_t)sqlite3_column_int(stmt, 1);
	}
	if (!sqlite3_finalize (stmt)==SQLITE_OK)
		return FALSE;
		
	return result;
}

		    


gboolean
mu_query_sqlite_get_stats (MuQuerySQLite *query, const char* tmpview,
			   MuQueryStats* stats)
{
	g_return_val_if_fail (query, FALSE);
	g_return_val_if_fail (tmpview, FALSE);
	g_return_val_if_fail (stats, FALSE);

	memset (stats, 0, sizeof(MuQueryStats));

	if (!get_newest_oldest_biggest_messages (query, tmpview, 
						 stats->_newest_msg_ids, NEWEST))
		return FALSE;
	    
	if (!get_newest_oldest_biggest_messages (query, tmpview, 
						 stats->_oldest_msg_ids, OLDEST))
		return FALSE;
	
	if (!get_newest_oldest_biggest_messages (query, tmpview, 
						 stats->_biggest_msg_ids, BIGGEST))
		return FALSE;

	if (!get_popular_contacts (query, tmpview, 
				   stats->_most_popular_sender_ids, 
				   MU_MSG_CONTACT_TYPE_FROM))
		return FALSE;
	
	if (!get_popular_contacts (query, tmpview, 
				   stats->_most_popular_recipient_ids, 
				   MU_MSG_CONTACT_TYPE_TO))
	    return FALSE;
	
	if (!get_count_avg(query,tmpview,&stats->_msg_num,&stats->_avg_msg_size))
		return FALSE;

	return TRUE;
}

