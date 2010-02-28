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

#include <glib.h>
#include <glib/gstdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <stdlib.h>

#include "mu-expr-error.h"
#include "mu-expr-bookmarks.h"


struct _MuExprBookmarks {
	GKeyFile *_keyfile;
};

#define MU_EXPR_BOOKMARKS_EXPR "expr"

MuExprBookmarks *
mu_expr_bookmarks_new (const char* bookmarkfile, GError **err)
{
	MuExprBookmarks *bookmarks;
	GError *myerr = NULL;

	if (!bookmarkfile)
		return NULL; /* not an error */

	if (g_access(bookmarkfile,R_OK) != 0) {
		g_set_error(err, 0, MU_EXPR_ERROR_BOOKMARKS_FILE,
			    "failed to read %s: %s", bookmarkfile, 
			    strerror(errno));
		return NULL;
	}

	bookmarks = (MuExprBookmarks*)malloc(sizeof(MuExprBookmarks));
	if (!bookmarks)
		return NULL;

	bookmarks->_keyfile = g_key_file_new ();
	if (!g_key_file_load_from_file (bookmarks->_keyfile, bookmarkfile, 0, 
					&myerr)) {
		g_set_error(err, 0, MU_EXPR_ERROR_BOOKMARKS_FILE,
			    "failed to read %s: %s", bookmarkfile, 
			    myerr->message);
		g_error_free (myerr);
		free (bookmarks);
		return NULL;
	}
			
	return bookmarks;
}

void
mu_expr_bookmarks_destroy (MuExprBookmarks *bookmarks)
{
	if (!bookmarks)
		return;

	g_key_file_free (bookmarks->_keyfile);
	free (bookmarks);
}


gchar*
mu_expr_bookmarks_resolve (MuExprBookmarks *bookmarks, 
			   const gchar* str)
{
	g_return_val_if_fail (bookmarks, NULL);
	g_return_val_if_fail (str, NULL);
	
	return g_key_file_get_string (bookmarks->_keyfile,
				      str, MU_EXPR_BOOKMARKS_EXPR,
				      NULL);
}
