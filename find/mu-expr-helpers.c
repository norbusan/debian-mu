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

#include "mu-expr-helpers.h"
#include "mu-expr-type.h"
#include "mu-expr-error.h"


GSList *
mu_expr_helpers_strlist_from_args (int argc, char *argv[])
{
	GSList *lst;
	int i;

	g_return_val_if_fail (argc >= 0, NULL);
	if (argc == 0)
		return NULL;
	g_return_val_if_fail (argv, NULL);

	/* we prepend args in opposite direction;
	 * prepending is faster
	 */
	for (i = argc - 1, lst = NULL; i >= 0; --i) {
		if (!argv[i])
			continue;		
		lst = g_slist_prepend (lst, g_strdup(argv[i]));
	}
	return lst;
}


GSList *
mu_expr_helpers_strlist_from_str (const char* str, GError **err)
{
	GSList *lst;
	GString *word;
	gboolean quoted;
	
	g_return_val_if_fail (str, NULL);

	word = g_string_sized_new (strlen(str));
	quoted = FALSE;
	lst = NULL;
	while (str && *str) {
		switch (*str) {
		case ' ':
			if (!quoted) {
				if (word->len) {
					lst = g_slist_append (lst, 
							       g_strdup(word->str)); 
					g_string_erase(word, 0, -1);
				}
			} else
				g_string_append_c (word, ' ');
			break;
		
		case '\'': 
			quoted = !quoted; 
			break;
	
		case '\\':
			++str;
			if (*str!='\\' && *str != '\'') {
				g_set_error (err, 0, MU_EXPR_ERROR_INVALID_ESCAPE_SEQ,
					     "invalid escape sequence");
				g_string_free (word, TRUE);
				mu_expr_helpers_strlist_free (lst);
				return NULL;
			} 
			g_string_append_c (word, *str); break;
			break;
			
		default: 
			g_string_append_c (word, *str); 
			break;
		
		}
		++str;
	}
		
	if (quoted) {
		g_set_error (err, 0, MU_EXPR_ERROR_UNTERMINATED_QUOTES,
			     "unterminated quotes in expression");
		g_string_free (word, TRUE);
		mu_expr_helpers_strlist_free (lst);
		return NULL;
	}
	
	if (word->len)
		lst = g_slist_append (lst, g_strdup(word->str));

	g_string_free (word, TRUE);

	return lst;
}


void
mu_expr_helpers_strlist_free (GSList *lst)
{
	g_slist_foreach (lst, (GFunc)g_free, NULL);
	g_slist_free (lst);
}



gboolean  
mu_expr_helpers_sortfields_valid (const char *str)
{
	if (!str)
		return TRUE; /* NULL is valid */
	
	while (str[0]) {
		if (mu_expr_type_from_char(str[0]) == MU_EXPR_TYPE_NONE)
			return FALSE;
		++str;
	}

	return TRUE;
}
