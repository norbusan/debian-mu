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

#ifndef __MU_EXPR_H__
#define __MU_EXPR_H__

#include <glib.h>

#include "mu-expr-type.h"
#include "mu-expr-error.h"
#include "mu-expr-crit.h"
#include "mu-expr-val.h"

G_BEGIN_DECLS

/*
 * a MuExprList is a list of MuExpr (mu-expression); 
 * _not_ to be confused with a list of strings (which
 * might be parsed into a MuExprList
 */
typedef GSList MuExprList;

struct _MuExpr;
typedef struct _MuExpr MuExpr;

MuExpr *    mu_expr_new     (const char* expr, GError **err);
void        mu_expr_destroy (MuExpr *expr);

MuExprType  mu_expr_type (MuExpr *expr);

MuExprVal * mu_expr_get_val  (MuExpr *expr);
MuExprCrit* mu_expr_get_crit (MuExpr *expr);

MuExprList*  mu_expr_list_new     (GSList *expr_strs, GError **err);
void         mu_expr_list_destroy (MuExprList *exprs, gboolean shallow);

G_END_DECLS


#endif /*__MU_EXPR_H__*/
