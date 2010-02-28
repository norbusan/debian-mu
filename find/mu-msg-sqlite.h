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

#ifndef __MU_MSG_SQLITE_H__
#define __MU_MSG_SQLITE_H__

#include <glib.h>
#include "mu/mu-msg.h"    /* for MuMsgPriority */

struct _MuMsgSQLite;
typedef struct _MuMsgSQLite MuMsgSQLite;

void             mu_msg_sqlite_destroy           (MuMsgSQLite *row);
gboolean         mu_msg_sqlite_next              (MuMsgSQLite *row, 
						  GError **err);
int              mu_msg_sqlite_get_id            (MuMsgSQLite *row);
const char*      mu_msg_sqlite_get_msgid         (MuMsgSQLite *row);
const char*      mu_msg_sqlite_get_path          (MuMsgSQLite *row);
size_t           mu_msg_sqlite_get_size          (MuMsgSQLite *row);  
time_t           mu_msg_sqlite_get_timestamp     (MuMsgSQLite *row);  
time_t           mu_msg_sqlite_get_date          (MuMsgSQLite *row);  

const char*      mu_msg_sqlite_get_from          (MuMsgSQLite *row);
const char*      mu_msg_sqlite_get_to            (MuMsgSQLite *row);
const char*      mu_msg_sqlite_get_cc            (MuMsgSQLite *row);
const char*      mu_msg_sqlite_get_subject       (MuMsgSQLite *row);
MuMsgFlags       mu_msg_sqlite_get_flags         (MuMsgSQLite *row);
MuMsgPriority    mu_msg_sqlite_get_priority      (MuMsgSQLite *row);

gchar*      mu_msg_sqlite_to_string (MuMsgSQLite *row, const char* rowformat);

#endif /*__MU_MSG_SQLITE_H__*/
