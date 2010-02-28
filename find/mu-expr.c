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

#include <glib.h>
#include <string.h>

#include "mu-expr.h"

struct _MuExpr {
	MuExprCrit *_crit; /* criteria, the part before the ':' */
	MuExprVal  *_val;  /* value, the part after the ':' */
};

MuExpr *
mu_expr_new (const char* str, GError **err)
{
	MuExpr *expr;
	MuExprVal *val;
	MuExprCrit *crit;

	char *colon;
	char *valstr, *critstr;

	g_return_val_if_fail (str, NULL);

	/* the first ':' separates crit and val */

	colon = strchr (str, ':');
	if (!colon) {
		g_set_error (err, 0,
			     MU_EXPR_ERROR_NO_COLON,
			     "expected: expression with colon, but got: %s",
			     str);
		return NULL;
	}

	if (colon == str) { /* expr starts with colon -- not good */
		g_set_error (err, 0,
			     MU_EXPR_ERROR_NO_CRITERIA,
			     "expected: expression with criteria, but got: %s",
			     str);
		return NULL;
	}

	if (colon[1] == '\0') { /* expr end with colon -- not good either */
		g_set_error (err, 0,
			     MU_EXPR_ERROR_NO_VALUE,
			     "expected: expression with value, but got: %s",
			     str);
		return NULL;
	}

	critstr = g_strndup (str, colon - str);
	crit = mu_expr_crit_new (critstr, err);
	g_free(critstr);
	if (!crit)
		return NULL;

	valstr  = g_strdup  (colon + 1);

	/* the first criterion determines the expression type */
	val = mu_expr_val_new (mu_expr_crit_types(crit)[0],valstr, err);
	
	g_free (valstr);
	if (!val)
		return NULL;

	expr = g_new0(MuExpr,1);
	expr->_crit = crit;
	expr->_val  = val;

	return expr;
}


void
mu_expr_destroy (MuExpr *expr)
{
	if (expr) {
		mu_expr_crit_destroy (expr->_crit);
		mu_expr_val_destroy  (expr->_val);
		g_free (expr);
	}
}


MuExprType
mu_expr_type (MuExpr *expr)
{
	g_return_val_if_fail (expr, -1);

	return mu_expr_crit_types(expr->_crit)[0];

}

MuExprVal*
mu_expr_get_val  (MuExpr *expr)
{
	g_return_val_if_fail (expr, NULL);

	return expr->_val;

}


MuExprCrit*
mu_expr_get_crit (MuExpr *expr)
{
	g_return_val_if_fail (expr, NULL);

	return expr->_crit;
}


MuExprList*
mu_expr_list_new (GSList *expr_strs, GError **err)
{
	GSList *lst, *cursor;

	if (!expr_strs)
		return NULL;

	cursor = expr_strs;
	for (cursor = expr_strs, lst=NULL; cursor; cursor=cursor->next) {
		MuExpr* expr;
		expr = mu_expr_new ((const char*)cursor->data, err);
		if (!expr) {
			mu_expr_list_destroy (lst, FALSE);
			return NULL;
		} else
			lst = g_slist_append (lst, expr);
	}

	return lst;
}



void
mu_expr_list_destroy (MuExprList *exprs, gboolean shallow)
{
	if (exprs) {
		if (!shallow)
			g_slist_foreach (exprs, (GFunc)mu_expr_destroy, NULL);

		g_slist_free (exprs);
	}
}
