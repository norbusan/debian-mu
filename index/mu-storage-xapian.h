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

#ifndef __MU_STORAGE_XAPIAN_H__
#define __MU_STORAGE_XAPIAN_H__

#include <glib.h>
#include <inttypes.h>

#include "msg/mu-msg-gmime.h"

G_BEGIN_DECLS

/*
 * index numbers for _add_value;
 */
enum MuStorageXapianValueIDs {
	MU_XAPIAN_SQLITE_MESSAGE_ID  = 0, /* the id field of the message in sqlite */ 
	MU_XAPIAN_MSG_PATH_ID        = 1  /* on-disk path to the msg */ 
};


struct _MuStorageXapian;
typedef struct _MuStorageXapian MuStorageXapian;

MuStorageXapian*  mu_storage_xapian_new     (const char* path);
void              mu_storage_xapian_tune    (MuStorageXapian *storage,
					     unsigned int transaction_size);

void              mu_storage_xapian_destroy (MuStorageXapian *storage);
MuResult	  mu_storage_xapian_store   (MuStorageXapian *storage,
					     int64_t msgid,
					     MuMsgGMime *msg);
MuResult          mu_storage_xapian_cleanup (MuStorageXapian *storage, 
					     const char* msgpath);
G_END_DECLS

#endif /*__MU_STORAGE_XAPIAN_H__*/
