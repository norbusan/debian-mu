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

#include <string.h>
#include <alloca.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <gmime/gmime.h>
#include <stdlib.h>
#include <ctype.h>

#include "mu/mu.h"
#include "mu-msg-gmime.h"

struct _MuMsgGMime {
	GMimeMessage    *_mime_msg;
	MuMsgFlags	_flags;
	char*		_html;
	char*		_text;	
	char*		_to;
	char*		_cc;
	size_t		_file_size;
	time_t		_timestamp;	
	char            *_path;
};

void 
mu_msg_gmime_destroy (MuMsgGMime *msg)
{
	if (msg) {
		if (G_IS_OBJECT(msg->_mime_msg))
			g_object_unref (msg->_mime_msg);
		
		g_free (msg->_html);
		g_free (msg->_text);
		g_free (msg->_path);
		g_free (msg->_to);
		g_free (msg->_cc);
		
		g_slice_free (MuMsgGMime, msg);
	} ;
}


static gboolean
init_file_metadata (MuMsgGMime* msg, const char* path)
{
	struct stat statbuf;

	if (access (path, R_OK) != 0) {
		g_warning ("%s: cannot read file %s: %s", 
			   __FUNCTION__, path, strerror(errno));
		return FALSE;
	}

	if (stat (path, &statbuf) < 0) {
		g_warning ("%s: cannot stat %s: %s", 
			   __FUNCTION__, path, strerror(errno));
		return FALSE;
	}
	
	if (!S_ISREG(statbuf.st_mode)) {
		g_warning ("%s: not a regular file: %s",
			   __FUNCTION__, path);
		return FALSE;
	}
	
	msg->_timestamp = statbuf.st_mtime;
	msg->_file_size = statbuf.st_size; 	
	msg->_path      = strdup (path);

	return TRUE;
}


static gboolean
init_mime_msg (MuMsgGMime *msg)
{
	FILE *file;
	GMimeStream *stream;
	GMimeParser *parser;

	g_debug ("opening mime msg for %s", mu_msg_gmime_get_path(msg));
	
	file = fopen (mu_msg_gmime_get_path(msg), "r");
	if (!file) {
		g_warning ("%s:cannot open %s: %s", 
			   __FUNCTION__, mu_msg_gmime_get_path(msg), 
			   strerror (errno));
		return FALSE;
	}

	stream = g_mime_stream_file_new (file);
	if (!stream) {
		g_warning ("%s: cannot create mime stream", __FUNCTION__);
		fclose (file);
		return FALSE;
	}
	
	parser = g_mime_parser_new_with_stream (stream);
	g_object_unref (stream);
	if (!parser) {
		g_warning ("%s: cannot create mime parser", __FUNCTION__);
		return FALSE;
	}

	msg->_mime_msg = g_mime_parser_construct_message (parser);
	g_object_unref (parser);
	if (!msg->_mime_msg) {
		g_warning ("%s: cannot create mime message", __FUNCTION__);
		return FALSE;
	}

	return TRUE;
}


MuMsgGMime*   
mu_msg_gmime_new (const char* filepath)
{
	MuMsgGMime *msg;
		
	g_return_val_if_fail (filepath, NULL);

	msg = g_slice_new0 (MuMsgGMime);
	if (!msg)
		return NULL;
	
	if (!init_file_metadata(msg, filepath)) {
		mu_msg_gmime_destroy (msg);
		return NULL;
	}

	if (!init_mime_msg(msg)) {
		mu_msg_gmime_destroy (msg);
		return NULL;
	}
		
	return msg;
}


const char*    
mu_msg_gmime_get_path  (MuMsgGMime *msg)
{
	g_return_val_if_fail (msg, NULL);

	return msg->_path;
}


const char*
mu_msg_gmime_get_subject (MuMsgGMime *msg)
{
	g_return_val_if_fail (msg, NULL);

	return g_mime_message_get_subject (msg->_mime_msg);
}

const char*
mu_msg_gmime_get_msgid (MuMsgGMime *msg)
{
	g_return_val_if_fail (msg, NULL);

	return g_mime_message_get_message_id (msg->_mime_msg);
}

const char*
mu_msg_gmime_get_refs (MuMsgGMime *msg)
{
	g_return_val_if_fail (msg, NULL);

	/* http://www.jwz.org/doc/threading.html */
	/* TODO:*/
	return g_mime_object_get_header (GMIME_OBJECT(msg->_mime_msg), 
					 "References");
}


const char*
mu_msg_gmime_get_user_agent (MuMsgGMime *msg)
{
	const char *m;
	GMimeObject *obj;

	g_return_val_if_fail (msg, NULL);
	obj = GMIME_OBJECT(msg->_mime_msg);
	
	m = g_mime_object_get_header (obj,"User-Agent");
	if (!m)
		m = g_mime_object_get_header (obj, "X-Mailer");
	
	return m;
}


const char*    
mu_msg_gmime_get_from (MuMsgGMime *msg)
{
	g_return_val_if_fail (msg, NULL);
	
	return g_mime_message_get_sender (msg->_mime_msg);
}


const char*
mu_msg_gmime_get_to (MuMsgGMime *msg)
{
	g_return_val_if_fail (msg, NULL);

	if (!msg->_to) {
		char *to;
		const InternetAddressList *recps;
		recps = g_mime_message_get_recipients (msg->_mime_msg,
						       GMIME_RECIPIENT_TYPE_TO);
		to = internet_address_list_to_string (recps, TRUE);
		if (to && strlen(to) == 0)
			g_free (to);
		else 
			msg->_to = to;
	}

	return msg->_to;
}

const char*
mu_msg_gmime_get_cc (MuMsgGMime *msg)
{
	g_return_val_if_fail (msg, NULL);

	if (!msg->_cc) {
		char *cc;
		const InternetAddressList *recps;
		recps = g_mime_message_get_recipients (msg->_mime_msg,
						       GMIME_RECIPIENT_TYPE_CC);
		
		cc = internet_address_list_to_string (recps, TRUE);
		if (cc && strlen(cc) == 0)
			g_free (cc);
		else 
			msg->_cc = cc;
	}

	return msg->_cc;
}


time_t
mu_msg_gmime_get_date (MuMsgGMime *msg)
{
	time_t t;

	g_return_val_if_fail (msg, 0);

	/* TODO: is the GMT-offset relevant? */
	g_mime_message_get_date(msg->_mime_msg, &t, NULL);
	
	return t;
}


static void
mu_g_mime_multipart_foreach_recursive (GMimeMultipart *multipart,
				       GMimePartFunc callback,
				       gpointer data)
{
	guint i;

	g_return_if_fail (GMIME_IS_MULTIPART(multipart));
	g_return_if_fail (callback != NULL);

	for (i = 0; i < g_mime_multipart_get_number(multipart); ++i) {
		GMimeObject *part;

		part = g_mime_multipart_get_part (multipart, i);
		callback (part, data);
		
		if (GMIME_IS_MULTIPART(part))
			mu_g_mime_multipart_foreach_recursive 
				(GMIME_MULTIPART(part), callback, data);

		g_object_unref (part);
	}
}

static void
mu_g_mime_message_foreach_part_recursive (GMimeMessage* message,
					  GMimePartFunc callback,
					  gpointer data)
{
	g_return_if_fail (GMIME_IS_MESSAGE(message));
	g_return_if_fail (callback != NULL);

	if (GMIME_IS_MULTIPART(message->mime_part))
		mu_g_mime_multipart_foreach_recursive 
			(GMIME_MULTIPART (message->mime_part),
			 callback, data);
	else
		callback (message->mime_part, data);
}


static gboolean
part_is_inline (GMimeObject *part)
{
	const char *disp;
	GMimeDisposition *dispo;
	gboolean result;
	const char *str;

	disp  = g_mime_part_get_content_disposition (GMIME_PART(part));
	if (!disp)
		return TRUE;
	
	dispo = g_mime_disposition_new (disp);
	str = g_mime_disposition_get (dispo);

	/* if it's not inline, it's an attachment */
	result = (str && (strcmp(str,GMIME_DISPOSITION_INLINE) == 0));
	g_mime_disposition_destroy (dispo);
	
	return result;
}
					  

static void
msg_cflags_cb (GMimeObject *part, MuMsgFlags *flags)
{
	if (GMIME_IS_PART(part)) 
		if ((*flags & MU_MSG_FLAG_HAS_ATTACH) == 0)
			if (!part_is_inline(part))
				*flags |= MU_MSG_FLAG_HAS_ATTACH;
}



static MuMsgFlags
get_content_flags (MuMsgGMime *msg)
{
	const GMimeContentType *ctype;
	MuMsgFlags flags = 0;
	GMimeObject *part;

	mu_g_mime_message_foreach_part_recursive 
		(msg->_mime_msg,(GMimePartFunc)msg_cflags_cb, 
		 &flags);

	/* note: signed or encrypted status for a message is determined by
	 *  the top-level mime-part
	 */
	if ((part = g_mime_message_get_mime_part(msg->_mime_msg))) {
		ctype = g_mime_object_get_content_type (part); 
		g_object_unref (part);
		if (ctype) {
			if (g_mime_content_type_is_type (ctype,"*", "signed")) 
				flags |= MU_MSG_FLAG_SIGNED;
			if (g_mime_content_type_is_type (ctype,"*", "encrypted")) 
				flags |= MU_MSG_FLAG_ENCRYPTED;
		}
	}

	return flags;
}


MuMsgFlags
mu_msg_gmime_get_flags (MuMsgGMime *msg)
{
	g_return_val_if_fail (msg, MU_MSG_FLAG_UNKNOWN);
	
	if (msg->_flags == MU_MSG_FLAG_UNKNOWN) {
		msg->_flags = 0;
		msg->_flags = mu_path_get_file_flags (mu_msg_gmime_get_path(msg));
		msg->_flags |= get_content_flags (msg);
	}
	
	return msg->_flags;
}


size_t
mu_msg_gmime_get_file_size (MuMsgGMime *msg)
{
	g_return_val_if_fail (msg, -1);

	return msg->_file_size;
}


static char*
to_lower (char *s)
{
	char *t = s;
	while (t&&*t) {
		t[0] = g_ascii_tolower(t[0]);
		++t;
	}
	return s;
}


static char*
get_prio_str (MuMsgGMime *msg)
{
	const char *str;
	GMimeObject *obj;

	obj = GMIME_OBJECT(msg->_mime_msg);

	str = g_mime_object_get_header (obj, "X-Priority");
	if (!str)
		str = g_mime_object_get_header (obj, "X-MSMail-Priority");
	if (!str)
		str = g_mime_object_get_header (obj, "Importance");
	if (!str)
		str = g_mime_object_get_header (obj, "Precedence");
	if (str) 
		return (to_lower(g_strdup(str)));
	else
		return NULL;
}


static MuMsgPriority
parse_prio_str (const char* priostr)
{
	int i;
	struct {
		const char*   _str;
		MuMsgPriority _prio;
	} str_prio[] = {
		{ "high", MU_MSG_PRIORITY_HIGH },
		{ "1",    MU_MSG_PRIORITY_HIGH },
		{ "2",    MU_MSG_PRIORITY_HIGH },
		
		{ "normal", MU_MSG_PRIORITY_NORMAL },
		{ "3",      MU_MSG_PRIORITY_NORMAL },

		{ "low",  MU_MSG_PRIORITY_LOW },
		{ "list", MU_MSG_PRIORITY_LOW },
		{ "bulk", MU_MSG_PRIORITY_LOW },
		{ "4",    MU_MSG_PRIORITY_LOW },
		{ "5",    MU_MSG_PRIORITY_LOW }
	};

	for (i = 0; i != G_N_ELEMENTS(str_prio); ++i)
		if (g_strstr_len (priostr, -1, str_prio[i]._str) != NULL)
			return str_prio[i]._prio;
	
	/* last-fm uses 'fm-user' as precedence */
	g_debug ("%s: unrecognized priority setting %s", __FUNCTION__, priostr);
	return MU_MSG_PRIORITY_NORMAL;
}


MuMsgPriority
mu_msg_gmime_get_priority (MuMsgGMime *msg)
{
	char* priostr;
	MuMsgPriority prio;

	g_return_val_if_fail (msg, 0);

	priostr = get_prio_str (msg);
	if (!priostr)
		return MU_MSG_PRIORITY_NORMAL;
	
	prio = parse_prio_str (priostr);

	g_free (priostr);

	return prio;
}



const char*
mu_msg_gmime_get_mailing_list (MuMsgGMime *msg)
{
	g_return_val_if_fail (msg, NULL);

	return g_mime_object_get_header (GMIME_OBJECT(msg->_mime_msg), 
					 "List-Id");
}



const char*     
mu_msg_gmime_get_header (MuMsgGMime *msg, const char* header)
{
	g_return_val_if_fail (msg, NULL);
	g_return_val_if_fail (header, NULL);

	return g_mime_object_get_header (GMIME_OBJECT(msg->_mime_msg), 
					 header);
}



time_t
mu_msg_gmime_get_timestamp (MuMsgGMime *msg)
{
	g_return_val_if_fail (msg, 0);
		
	return msg->_timestamp;
}

struct _GetBodyData {
	GMimeObject *_part;
	gboolean     _want_html;
};
typedef struct _GetBodyData GetBodyData;

static void
get_body_cb (GMimeObject *part, GetBodyData *data)
{
	const char *subtype;
	const char *disp;

	const GMimeContentType *ct;

	ct = g_mime_object_get_content_type (part);
	
	/* already found? */
	if (data->_part != NULL)
		return;

	/* is it right content type? */
	subtype = data->_want_html ? "html": "plain"; 
	if (!g_mime_content_type_is_type (ct, "text", subtype))
		return;
	
	/* is it not an attachment? */
	disp = g_mime_part_get_content_disposition (GMIME_PART(part));
	if (disp) {
		GMimeDisposition *dispo = g_mime_disposition_new (disp);
		const char *str = g_mime_disposition_get (dispo);
		if (str && (strcmp(str,GMIME_DISPOSITION_INLINE) != 0)) {
			g_mime_disposition_destroy (dispo);
			return; /* not inline, so it's not the body */
		}
		g_mime_disposition_destroy (dispo);
	}
	
	/* we found the body, so it seems */
	data->_part = part;
}	


/* turn \0-terminated buf into ascii (which is a utf8 subset);
 *   convert any non-ascii into '.'
 */
static void
asciify (char *buf)
{
	char *c;
	for (c = buf; c && *c; ++c)
		if (!isascii(*c))
			c[0] = '.';
}



/* NOTE: buffer will be *freed* or returned unchanged */
static char*
convert_to_utf8 (GMimePart *part, char *buffer)
{
	const GMimeContentType *ctype;
	const char* charset;
	GError *err = NULL;

	ctype   = g_mime_part_get_content_type (part);
	charset = g_mime_content_type_get_parameter (ctype,
						     "charset");
	if (charset) 
		charset = g_mime_charset_iconv_name (charset);

	if (charset) {
		char * utf8 = g_convert_with_fallback (buffer, -1, "UTF-8",
						       charset, ".", NULL, NULL,
						       &err);
		if (!utf8) {
			g_message ("%s: conversion failed from %s: %s", 
				   __FUNCTION__, charset,
				   err ? err ->message : "");
			if (err)
				g_error_free (err);
		} else {
			g_free (buffer);
			return utf8;
		}
	}

	/* hmmm.... no charset at all, or conversion failed; ugly hack:
	 *  replace all non-ascii chars with '.' instead... TODO: come up
	 * with something better */
	asciify (buffer);
	return buffer;
}


static char*
part_to_string (GMimePart *part, gboolean convert_utf8)
{
	GMimeDataWrapper *wrapper;
	GMimeStream *stream = NULL;
	
	ssize_t buflen, bytes;
	char *buffer = NULL;
	
	wrapper = g_mime_part_get_content_object (part);
	if (!wrapper) {
		g_warning ("failed to create data wrapper");
		goto cleanup;
	}

	stream = g_mime_stream_mem_new ();
	if (!stream) {
		g_warning ("failed to create mem stream");
		goto cleanup;
	}

	buflen = g_mime_data_wrapper_write_to_stream (wrapper, stream);
	if (buflen == 0) {
		g_debug ("empty buffer");
		goto cleanup;
	}
	
	buffer = (char*)malloc(buflen + 1);
	if (!buffer) {
		g_warning ("failed to allocate %d bytes", (int)buflen);
		goto cleanup;
	}	
	g_mime_stream_reset (stream);
	
	/* we read everything in one go */
	bytes = g_mime_stream_read (stream, buffer, buflen);
	if (bytes < 0) {
		free (buffer);
		buffer = NULL;
	} else
		buffer[bytes]='\0';

	/* convert_to_utf8 will free the old 'buffer' if needed */
	if (buffer && convert_utf8) 
		buffer = convert_to_utf8 (part, buffer);
	
cleanup:				
	if (stream)
		g_object_unref (G_OBJECT(stream));
	if (wrapper)
		g_object_unref (G_OBJECT(wrapper));
	
	return buffer;
}


static char*
mu_msg_gmime_get_body (MuMsgGMime *msg, gboolean html)
{
	GetBodyData data;

	memset (&data, 0, sizeof(GetBodyData));
	data._want_html = html;
	
	mu_g_mime_message_foreach_part_recursive (msg->_mime_msg,
						  (GMimePartFunc)get_body_cb,
						  &data);
	if (data._part) /* convert to UTF8 if it's text (not html) */
		return part_to_string (GMIME_PART(data._part), !html);
	else
		return NULL;
}

const char*
mu_msg_gmime_get_body_html (MuMsgGMime *msg)
{
	g_return_val_if_fail (msg, NULL);

	if (msg->_html)
		return msg->_html;
	else	
		return msg->_html = mu_msg_gmime_get_body (msg, TRUE);
}


const char*
mu_msg_gmime_get_body_text (MuMsgGMime *msg)
{
	g_return_val_if_fail (msg, NULL);
	
	if (msg->_text)
		return msg->_text;
	else	
		return msg->_text = mu_msg_gmime_get_body (msg, FALSE);
}



static int
mu_msg_gmime_get_contacts_from (MuMsgGMime *msg, MuMsgGMimeContactsCallback cb, 
				void *ptr)
{
	InternetAddressList *list, *cursor;

	/* we go through this whole excercise of trying to get a *list*
	 * of 'From:' address (usually there is only one...), because
	 * internet_address_parse_string has the nice side-effect of
	 * splitting in names and addresses for us */

	cursor = list = internet_address_parse_string (
		g_mime_message_get_sender (msg->_mime_msg));
	
	while (cursor) {
		MuMsgContact contact; /* stack allocated */
		InternetAddress *addr  = 
			internet_address_list_get_address (cursor);
		if (addr) {
			int result;
				    
			contact._name = internet_address_get_name (addr);
			contact._type = MU_MSG_CONTACT_TYPE_FROM;  
	
			/* we only support internet addresses;
			 * if we don't check, g_mime hits an assert
			 */
			contact._addr = (addr->type == 1) ?
				internet_address_get_addr (addr) : NULL;
	
		
			result = (cb)(&contact,ptr);
			
			/* note: don't unref addr here, as it's owned */
			/* by the list (at least that is what valgrind tells... */
			if ((result = (cb)(&contact,ptr)) != 0) { 
				/* callback tells us to stop */
				internet_address_list_destroy (list);
				return result;
			}
		}
		cursor = internet_address_list_next (cursor);
	}
	
	internet_address_list_destroy (list);	
	return 0;
}


int
mu_msg_gmime_get_contacts_foreach (MuMsgGMime *msg, 
				   MuMsgGMimeContactsCallback cb, 
				   void *ptr)
{
	int i, result;

	struct { 
		const char            *_gmime_type;
		MuMsgContactType       _type;
	} ctypes[] = {
		{GMIME_RECIPIENT_TYPE_TO,  MU_MSG_CONTACT_TYPE_TO},
		{GMIME_RECIPIENT_TYPE_CC,  MU_MSG_CONTACT_TYPE_CC},
		{GMIME_RECIPIENT_TYPE_BCC, MU_MSG_CONTACT_TYPE_BCC},
	};

	g_return_val_if_fail (cb && msg, -1);

	/* first, get the from address */
	if ((result = mu_msg_gmime_get_contacts_from (msg, cb, ptr)) != 0)
		return result; /* callback told us to stop */
	
	for (i = 0; i != sizeof(ctypes)/sizeof(ctypes[0]); ++i) {

		MuMsgContact contact; /* stack allocated */
		const InternetAddressList *cursor;
		
		cursor = g_mime_message_get_recipients 
			(msg->_mime_msg, ctypes[i]._gmime_type);
		
		while (cursor) {
			InternetAddress *addr  = 
				internet_address_list_get_address (cursor);
			if (addr) {

				contact._name = internet_address_get_name (addr);
				contact._type = ctypes[i]._type;  

				/* we only support internet addresses;
				 * if we don't check, g_mime hits an assert
				 */
				contact._addr = (addr->type == 1) ?
					internet_address_get_addr (addr) : NULL;
	
				result = (cb)(&contact,ptr);
	
				if (result != 0) /* callback tells us to stop */
					return result;
			}
			cursor = internet_address_list_next (cursor);
		}
	}
	return 0;
}

