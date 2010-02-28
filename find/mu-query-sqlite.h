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

#ifndef __MU_QUERY_SQLITE_H__
#define __MU_QUERY_SQLITE_H__

#include <glib.h>
#include <inttypes.h>

#include "mu-msg-sqlite.h"

/*
 * MuQuerySQLite. Usually, it is not used directly, but through
 * MuQueryMgr
 */
struct _MuQuerySQLite;
typedef struct _MuQuerySQLite MuQuerySQLite;

/** 
 * create a new MuQuerySqlite instance. 
 * 
 * @param dbpath path to the sqlite db to search
 * @param err receives error information (if there is any)
 * 
 * @return a new MuQuerySqlite instance, or NULL in case of error.
 * when the instance is no longer needed, use mu_query_sqlite_destroy
 * to free it
 */
MuQuerySQLite *mu_query_sqlite_new      (const char* dbpath,
					 GError **err);

/** 
 * destroy the MuQueryMgr instance
 * 
 * @param mgr a MuQueryMgr instance, or NULL
 */
void           mu_query_sqlite_destroy  (MuQuerySQLite *query);

/** 
 * run a SQL query on the database, and return the resulting row
 * (if it is a SELECT query); you must call mu_msg_sqlite_next to
 * move to that first row
 * 
 * @param mgr a MuQuerySqliter instance, or NULL
 * @param sql the sql query to run
 * @err a GError ptr ptr to return an error (if any), or NULL
 * 
 * @return a new MuMsgSQLite object with the search query, or NULL in
 * case of error. to get the first result row, do mu_msg_sqlite_next
 * and get subsequent rows with the next call to mu_msg_sqlite_next
 * until that call returns FALSE
 */
MuMsgSQLite*      mu_query_sqlite_run      (MuQuerySQLite *query, 
					    const char *sql, GError **err);

/** 
 * run a SQL query on the database without returning rows
 * 
 * @param mgr a MuQuerySqliter instance, or NULL
 * @param sql the sql query to run
 * @err a GError ptr ptr to return an error (if any), or NULL
 * 
 * @return TRUE if the query succeeded, FALSE otherwise
 */
gboolean          mu_query_sqlite_exec     (MuQuerySQLite *query, 
					    const char *sql, GError **err);


#define MU_QUERY_STATS_TOP 5
struct _MuQueryStats {
	size_t   _msg_num;
	uint64_t _newest_msg_ids             [MU_QUERY_STATS_TOP];
	uint64_t _oldest_msg_ids             [MU_QUERY_STATS_TOP];
	uint64_t _biggest_msg_ids            [MU_QUERY_STATS_TOP];
	size_t   _avg_msg_size;    
	uint64_t _most_popular_sender_ids    [MU_QUERY_STATS_TOP]; 
	uint64_t _most_popular_recipient_ids [MU_QUERY_STATS_TOP];
};
typedef struct _MuQueryStats MuQueryStats;


/** 
 * get statistics about a table with messages; usually, this table will
 * be a temp view/table created as a subset of the message table. 
 *
 * @param query MuQuerySQLite instance
 * @param tmpview the name of the table/view to get stats for
 * @param stats ptr to a stats struct that will receive the result
 * 
 * @return TRUE if succeeded, FALSE otherwise
 */
gboolean          mu_query_sqlite_get_stats (MuQuerySQLite *query, 
					     const char* tmpview,
					     MuQueryStats* stats);


#endif /*__MU_QUERY_SQLITE_H__*/
