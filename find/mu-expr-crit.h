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

#ifndef __MU_EXPR_CRIT_H__
#define __MU_EXPR_CRIT_H__

#include <glib.h>
#include "mu-expr-type.h"
#include "mu-expr-error.h"

struct _MuExprCrit;
typedef struct _MuExprCrit MuExprCrit;

MuExprCrit* mu_expr_crit_new     (const char* str, GError **err);
void        mu_expr_crit_destroy (MuExprCrit *crit);

const MuExprType* mu_expr_crit_types (MuExprCrit *crit);

#endif /*__MU_EXPR_CRIT_H__*/
