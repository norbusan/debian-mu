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

#ifndef __MU_STORAGE_SQLITE_H__
#define __MU_STORAGE_SQLITE_H__

#include <time.h>
#include <inttypes.h>
#include "msg/mu-msg-gmime.h"

/* opaque structure */
struct _MuStorageSQLite;
typedef struct _MuStorageSQLite MuStorageSQLite;

/** 
 * create a new Storage instance
 * 
 * @param dbpath path where to store the (sqlite3) database; if it
 * does not exist yet, it will be created
 * 
 * @return a new Storage instance, or NULL in case of error
 */
MuStorageSQLite* mu_storage_sqlite_new (const char *dbpath);

/** 
 * destroy a storage instance
 * 
 * @param storage a MuStorageSQLite instance, or NULL
 */
void mu_storage_sqlite_destroy (MuStorageSQLite *storage);


/** 
 * set some performance tuning parameters. parameters outside the range are
 * ignored 
 *
 * @param storage a MuStorageSQLite instance
 * @param transaction_size the new transaction size >0, ie. the number 
 * of messages that are updated/inserted within a database transaction
 * @param synchronous set PRAGMA synchronous [0..2] 
 * (http://www.sqlite.org/pragma.html)
 * @param temp_store set PRAGMA temp_store  [0..2] 
 * http://www.sqlite.org/pragma.html)
 */
void
mu_storage_sqlite_tune (MuStorageSQLite *storage, 
			unsigned int transaction_size,
			unsigned int synchronous, 
			unsigned int temp_store);
/** 
 * store the information about a message in the storage db
 * 
 * @param storage a valid MuStorageSQLite instance
 * @param info a MuMsgGMime structure with information about a message
 * 
 * @return the message id of the message which was added or updated,
 * or -1 in case of error; see logs for details of what exactly
 * went wrong
 */
int64_t mu_storage_sqlite_store (MuStorageSQLite *storage, MuMsgGMime *msg);


/** 
 * retrieve the time of the last update for message with filename, or 0 if none 
 * were ever done (or the last update was 01-01-1970...)
 * 
 * @param storage a valid MuStorageSQLite instance
 * @param fname filename for some file
 * 
 * @return the last update time_t, or 0 in case of error or if no update
 * was ever done.
 */
time_t mu_storage_sqlite_message_timestamp (MuStorageSQLite *storage, 
					    const char *fname);


/**
 * callback function for mu_storage_sqlite_cleanup
 * 
 */
typedef MuResult (*MuStorageSQLiteCleanupCallback) (MuMsgStatus status,
						    const char* path, void *data);

/** 
 * remove messages from the sqlite database for which there is no
 * corresponding file on the file system
 * 
 * @param storage a valid MuStorageSQLite instance 
 * @param cb a callback function that will be called for each message
 * that is to be deleted 
 * @param data user pointer for the callback function
 * 
 * @return MU_OK if all went OK, MU_ERROR in case of an error, or
 * MU_STOP in case th the user stopped the operation
 */
MuResult mu_storage_sqlite_cleanup (MuStorageSQLite *storage, 
				    MuStorageSQLiteCleanupCallback cb, void *data);

#endif /*__MU_STORAGE_SQLITE_H__*/
