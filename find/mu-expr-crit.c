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

#include "mu-expr-crit.h"

struct _MuExprCrit {
	unsigned char _types[MU_EXPR_TYPE_NUM];
};


MuExprCrit* 
mu_expr_crit_new (const char* str, GError **err)
{
	MuExprCrit *crit;
	int i;
	gboolean non_string_type_seen;

	g_return_val_if_fail (str, NULL);

	crit = g_new0(MuExprCrit, 1);
	crit->_types[0] = MU_EXPR_TYPE_NONE;

	non_string_type_seen = FALSE;
	
	i = 0;
	while (str[0]) {
		MuExprType t;

		t = mu_expr_type_from_char(str[0]);		
		if (t == MU_EXPR_TYPE_NONE) {
			g_set_error (err, 0,
				     MU_EXPR_ERROR_INVALID_CRITERION,
				     "expected: criterion char, but got: %c", str[0]);
			g_free (crit);
			return NULL;
		}
		
		if (!mu_expr_type_is_string(t) && i > 0) {
			g_set_error (err, 0,
				     MU_EXPR_ERROR_INVALID_CRITERION,
				     "non-string criteria cannot be combined");
			g_free (crit);
			return NULL;
		} 

		if (mu_expr_type_is_string(t)) {
			if (non_string_type_seen) {
				g_set_error (err, 0,
					     MU_EXPR_ERROR_MIXED_TYPE_CRITERIA,
					     "cannot combine string and "
					     "non-string criteria (%c)", str[0]);
				g_free (crit);
				return NULL;
			}
		} else
			non_string_type_seen = TRUE;

		crit->_types[i] = t;
		
		++str;
		++i;
	}

	crit->_types[i] = MU_EXPR_TYPE_NONE; /* close the list */

	
	return crit;
}

void
mu_expr_crit_destroy (MuExprCrit *crit)
{
	if (crit)
		g_free (crit);
}


const MuExprType*
mu_expr_crit_types (MuExprCrit *crit)
{						
	g_return_val_if_fail (crit, NULL);
	return crit->_types;
}
