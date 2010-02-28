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

#ifndef __MU_EXPR_TYPE_H__
#define __MU_EXPR_TYPE_H__

#include <glib.h>

enum _MuExprType {
        MU_EXPR_TYPE_TO,
        MU_EXPR_TYPE_CC,
        MU_EXPR_TYPE_FROM,
        MU_EXPR_TYPE_SUBJECT,
        MU_EXPR_TYPE_MSG_ID,
        MU_EXPR_TYPE_XAPIAN,
        MU_EXPR_TYPE_FLAGS,
        MU_EXPR_TYPE_DATE,
        MU_EXPR_TYPE_SIZE,
	MU_EXPR_TYPE_PATH,
	MU_EXPR_TYPE_PRIORITY,
	MU_EXPR_TYPE_TIMESTAMP,
	MU_EXPR_TYPE_ID,

	MU_EXPR_TYPE_NUM,
	MU_EXPR_TYPE_NONE
};
typedef unsigned char MuExprType;

#define mu_expr_type_is_valid(t)  ((t)<MU_EXPR_TYPE_NUM)

MuExprType mu_expr_type_from_char (char c);
const char* mu_expr_type_db_field (MuExprType type);

gboolean mu_expr_type_is_string (MuExprType type);
gboolean mu_expr_type_is_number (MuExprType type);
gboolean mu_expr_type_is_interval (MuExprType type);

#endif /*__MU_EXPR_TYPE_H__*/
