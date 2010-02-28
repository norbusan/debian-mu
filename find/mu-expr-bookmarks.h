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

#ifndef __MU_EXPR_BOOKMARKS_H__
#define __MU_EXPR_BOOKMARKS_H__

/*
 * pseudo-MuExprs can contain bookmarks, such as 'B:foo_bar'
 * while preprocessing pseudo-MuExprs into proper MuExprs, these
 * bookmarks are resolved.
 */
struct _MuExprBookmarks;
typedef struct _MuExprBookmarks MuExprBookmarks;

/** 
 * get a new bookmark handler, which loads the bookmarks file and
 * allows for resolving them. free with mu_expr_bookmarks_destroy
 *
 * @param bookmarksfile file containing bookmarks
 * @param err receives error information (if there is any)
 * 
 * @return a new MuExpBookmarks instance, or NULL in case of error.
 */
MuExprBookmarks *mu_expr_bookmarks_new (const char* bookmarksfile,
					GError **err);

/** 
 * destroy a MuExprBookmarks instance
 * 
 * @param bookmarks a valid MuExprBookmarks instance, or NULL
 */
void  mu_expr_bookmarks_destroy (MuExprBookmarks *bookmarks);

/** 
 * resolve a bookmark. Note that this resolves bookmarks, not bookmark
 * expressions; ie. it expecte 'foo_bar', not 'B:foo_bar'
 * 
 * @param bookmarks a valid MuExprBookmarks instance
 * @param str a bookmark to resolve
 * 
 * @return the resolved bookmark as newly allocated string, or NULL if
 * it could not be resolved. Free with g_free
 */
gchar*    mu_expr_bookmarks_resolve (MuExprBookmarks *bookmarks, 
				     const char* str);


#endif /*__MU_EXPR_BOOKMARKS_H__*/
