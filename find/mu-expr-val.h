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

#ifndef __MU_EXPR_VAL_H__
#define __MU_EXPR_VAL_H__

#include <glib.h>
#include "mu-expr-type.h"

G_BEGIN_DECLS

struct _MuExprVal;
typedef struct _MuExprVal MuExprVal;

enum _MuExprPred {
        MU_EXPR_PRED_EXACT,
        MU_EXPR_PRED_LIKE,
	MU_EXPR_PRED_INTERVAL
};
typedef enum _MuExprPred MuExprPred;

MuExprVal*   mu_expr_val_new       (MuExprType type, const char* str,
				    GError **err);
void         mu_expr_val_destroy   (MuExprVal *val);

int          mu_expr_val_int     (MuExprVal *val);
int          mu_expr_val_int_neg (MuExprVal *val);

int          mu_expr_val_lower   (MuExprVal *val);
int          mu_expr_val_upper   (MuExprVal *val);
const char*  mu_expr_val_str     (MuExprVal *val);

gboolean     mu_expr_val_neg      (MuExprVal *val);
MuExprPred   mu_expr_val_pred     (MuExprVal *val);

G_END_DECLS

#endif /*__MU_EXPR_VAL_H__*/
