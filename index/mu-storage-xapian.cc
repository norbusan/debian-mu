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

#include <xapian.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /*HAVE_CONFIG_H*/

#include "msg/mu-msg-gmime.h"
#include "mu-storage-xapian.h"

static bool transaction_begin (MuStorageXapian *storage);
static bool transaction_commit (MuStorageXapian *storage);
static bool transaction_rollback (MuStorageXapian *storage);

struct _MuStorageXapian {
	Xapian::WritableDatabase *_db;

	/* transaction handling */
	bool   _in_transaction;
	int    _processed;
	size_t _transaction_size;
};

MuStorageXapian*
mu_storage_xapian_new  (const char* path)
{
	MuStorageXapian *storage;
	
	g_return_val_if_fail (path, NULL);
	
	try {
		storage = g_new0(MuStorageXapian,1);
		storage->_db = new Xapian::WritableDatabase
			(path,Xapian::DB_CREATE_OR_OPEN);

		/* keep count of processed docs */
		storage->_transaction_size = 1000; /* default */
		storage->_in_transaction = false;
		storage->_processed = 0;

		g_message ("%s: opened %s", __FUNCTION__, path);
		return storage;

	} catch (const Xapian::Error &err) {
		delete storage->_db;
		g_free (storage);
		g_warning ("%s: caught xapian exception '%s' (%s)",
			   __FUNCTION__, err.get_msg().c_str(),
			   err.get_error_string());
		return NULL;
	} catch (...) {
		delete storage->_db;
		g_free (storage);
		g_warning ("%s: caught exception", __FUNCTION__);
		return NULL;
	}
}

void
mu_storage_xapian_tune (MuStorageXapian *storage,
			unsigned int transaction_size)
{
	g_return_if_fail (storage);

	if (transaction_size) {
		g_message ("tune: setting xapian transaction size to %u",
			   transaction_size);
		storage->_transaction_size = transaction_size;
	}
}



void
mu_storage_xapian_destroy (MuStorageXapian *storage)
{
	if (!storage)
		return;

	try { 	
		if (storage->_in_transaction)
			transaction_commit (storage);

		delete storage->_db;
		g_free (storage);

	} catch (const Xapian::Error &err) {
		g_free (storage);
		g_warning ("%s: caught xapian exception '%s' (%s)", 
			   __FUNCTION__, err.get_msg().c_str(), 
			   err.get_error_string());
	} catch (...) {
		g_free (storage);
		g_warning ("%s: caught exception", __FUNCTION__);
	}
}

static bool
transaction_begin (MuStorageXapian *storage)
{
	if (storage->_in_transaction) {
		g_warning ("%s: already in a transaction", __FUNCTION__);
		return false;
	}
	
	try {
		g_debug ("%s: starting xapian transaction", 
			 __FUNCTION__);
		storage->_db->begin_transaction();
		storage->_in_transaction = true;

		return true;

	} catch (const Xapian::Error &err) {
		g_warning ("%s: caught xapian exception '%s' (%s)", 
			   __FUNCTION__, err.get_msg().c_str(), 
			   err.get_error_string());
	} catch (...) {
		g_warning ("%s: caught exception", __FUNCTION__);
	}

	return false;
}


static bool
transaction_commit (MuStorageXapian *storage)
{
	if (!storage->_in_transaction) {
		g_warning ("%s: not in a tranction", __FUNCTION__);
		return false;
	}

	try {
		g_debug ("%s: commiting xapian transaction", 
			 __FUNCTION__);
		storage->_in_transaction = false;
		storage->_db->commit_transaction();

		return true;

	} catch (const Xapian::Error &err) {
		g_warning ("%s: caught xapian exception '%s' (%s)", 
			   __FUNCTION__, err.get_msg().c_str(), 
			   err.get_error_string());
	} catch (...) {
		g_warning ("%s: caught exception", __FUNCTION__);
	}

	return false;
}


static bool
transaction_rollback (MuStorageXapian *storage)
{
	if (!storage->_in_transaction) {
		g_warning ("%s: not in a tranction", __FUNCTION__);
		return false;
	}

	try {
		g_debug ("%s: rolling back xapian transaction", 
			 __FUNCTION__);
	
		storage->_in_transaction = false;
		storage->_db->cancel_transaction();

		return true;

	} catch (const Xapian::Error &err) {
		g_warning ("%s: caught xapian exception '%s' (%s)", 
			   __FUNCTION__, err.get_msg().c_str(), 
			   err.get_error_string());
	} catch (...) {
		g_warning ("%s: caught exception", __FUNCTION__);
	}

	return false;
}


static void
index_if_not_null (Xapian::TermGenerator& termgen, const char* str)
{
	if (str)
		termgen.index_text (str);
}

MuResult
mu_storage_xapian_store (MuStorageXapian *storage, int64_t msgid,
			 MuMsgGMime *msg)
{
	char msgid_buf[24];

	g_return_val_if_fail (storage, MU_ERROR);
	g_return_val_if_fail (msg, MU_ERROR);

	g_snprintf (msgid_buf, 24, (sizeof(int*)==4)?"%lld":"%ld", msgid);
	/* Note, C99's PRId64 should expand to either lld (32-bit machines) or
	 * ld (64-bit machines); but it's not defined everywhere... */

	try {
		const char* body;
		Xapian::Document newdoc;
		Xapian::TermGenerator termgen;

		/* start transaction if needed */
		if (!storage->_in_transaction && !transaction_begin (storage)) {
			g_warning ("%s: failed to start transaction", __FUNCTION__);
			return MU_ERROR;
		}

		newdoc.add_value(MU_XAPIAN_MSG_PATH_ID, mu_msg_gmime_get_path(msg));
		newdoc.add_value(MU_XAPIAN_SQLITE_MESSAGE_ID, msgid_buf);

		index_if_not_null (termgen,mu_msg_gmime_get_subject (msg));
		index_if_not_null (termgen,mu_msg_gmime_get_from (msg));
		index_if_not_null (termgen,mu_msg_gmime_get_to (msg));
		
		termgen.set_document(newdoc);
		
		body = NULL;
		if (!(mu_msg_gmime_get_flags(msg) & MU_MSG_FLAG_ENCRYPTED)) 
			body = mu_msg_gmime_get_body_text(msg);
		if (body)
			termgen.index_text(body);
		else 
			g_message ("%s: no readable text body found for %s",
				   __FUNCTION__, mu_msg_gmime_get_path(msg));
		/* not a bug; the message may be encrypted,
		 * may have a html-only body, or simply have
		 * body at all */
		Xapian::docid id (storage->_db->add_document (newdoc));
		if (id == 0) {
			g_warning ("%s: failed to store document data", 
				   __FUNCTION__);
			transaction_rollback (storage);
			return MU_OK;
		}
		g_debug ("%s: stored document with id %u",__FUNCTION__, 
			 (unsigned int)id);
		
		if ((++storage->_processed % storage->_transaction_size) == 0)
			if (!transaction_commit (storage)) {
				g_warning ("%s: commit failed", __FUNCTION__);
				return MU_ERROR;
			}

		return MU_OK;

	} catch (const Xapian::Error &err) {
		g_warning ("%s: caught xapian exception '%s' (%s)", 
			   __FUNCTION__, err.get_msg().c_str(), 
			   err.get_error_string());

	} catch (...) {
		g_warning ("%s: caught exception", __FUNCTION__);
	}

	return MU_ERROR;
}


MuResult
mu_storage_xapian_cleanup (MuStorageXapian *storage, const char* msgpath)
{
	g_return_val_if_fail (storage, MU_ERROR);
	g_return_val_if_fail (msgpath, MU_ERROR);

	try {
		/* this seems like an akward to get the specific doc;
		   defining a range... is there another way? */
		Xapian::Query::Query q (Xapian::Query::OP_VALUE_RANGE,
					MU_XAPIAN_MSG_PATH_ID,
					msgpath,msgpath);
		Xapian::Enquire enq(*storage->_db);
		Xapian::MSet matches;

		enq.set_query(q);
		matches = enq.get_mset(0, 1); 
		
		transaction_begin (storage);
		for (Xapian::MSet::const_iterator ci = matches.begin();
		     ci != matches.end(); ++ci) {
			Xapian::docid id (ci.get_document().get_docid());
			g_message ("xapian: deleting document %u", (guint)id);
			storage->_db->delete_document(id);
		}		
		transaction_commit (storage);
		return MU_OK;

	} catch (const Xapian::Error &err) {
		g_warning ("%s: caught xapian exception '%s' (%s)", 
			   __FUNCTION__, err.get_msg().c_str(), 
			   err.get_error_string());
		
		if (storage->_in_transaction)
			transaction_rollback (storage);

		return MU_ERROR;
	} catch (...) {
		g_warning ("%s: caught exception", __FUNCTION__);
		
		if (storage->_in_transaction)
			transaction_rollback (storage);

		return MU_ERROR;
	}


}
