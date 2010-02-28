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
#include "mu-expr-type.h"

enum _MuExprCat {
	MU_EXPR_CAT_STRING,
	MU_EXPR_CAT_NUMBER,
	MU_EXPR_CAT_INTERVAL,
	MU_EXPR_CAT_NOSEARCH
};
typedef enum _MuExprCat MuExprCat;

struct _MuExprTypeInfo {
	MuExprType    _type;
	char          _char;
	const char*   _dbfield;
	MuExprCat     _cat;
};
typedef struct _MuExprTypeInfo MuExprTypeInfo;

static const MuExprTypeInfo MU_EXPR_TYPE_INFO[] = {
	{ MU_EXPR_TYPE_TO,	  't',    "m.recipients",   MU_EXPR_CAT_STRING},
        { MU_EXPR_TYPE_CC,	  'c',    "m.cc",           MU_EXPR_CAT_STRING},
        { MU_EXPR_TYPE_FROM,      'f',    "m.sender",       MU_EXPR_CAT_STRING},
        { MU_EXPR_TYPE_SUBJECT,   's',    "m.subject",      MU_EXPR_CAT_STRING},
	{ MU_EXPR_TYPE_MSG_ID,    'm',    "m.msg_id",       MU_EXPR_CAT_STRING},
        { MU_EXPR_TYPE_XAPIAN,    'x',    "NULL AS dummy2", MU_EXPR_CAT_STRING},
        { MU_EXPR_TYPE_FLAGS,     'F',    "m.flags",        MU_EXPR_CAT_NUMBER},
	{ MU_EXPR_TYPE_DATE,      'd',    "m.mdate",        MU_EXPR_CAT_INTERVAL},
	{ MU_EXPR_TYPE_PATH,      'P',    "m.mpath",        MU_EXPR_CAT_STRING},
	{ MU_EXPR_TYPE_SIZE,      'z',    "m.msize",        MU_EXPR_CAT_INTERVAL},
	{ MU_EXPR_TYPE_PRIORITY,  'p',    "m.priority",     MU_EXPR_CAT_NUMBER},
	{ MU_EXPR_TYPE_TIMESTAMP, 'T',    "m.tstamp",       MU_EXPR_CAT_NOSEARCH},  
	{ MU_EXPR_TYPE_ID,        'i',    "m.id",           MU_EXPR_CAT_NOSEARCH},
};


MuExprType
mu_expr_type_from_char (char c)
{
	int i;
	for (i = 0; i != G_N_ELEMENTS(MU_EXPR_TYPE_INFO); ++i)
		if (c == MU_EXPR_TYPE_INFO[i]._char &&
		    MU_EXPR_TYPE_INFO[i]._cat != MU_EXPR_CAT_NOSEARCH)
			return MU_EXPR_TYPE_INFO[i]._type;
	
	return MU_EXPR_TYPE_NONE;
}


const char*
mu_expr_type_db_field (MuExprType type)
{
	int i;
	g_return_val_if_fail(mu_expr_type_is_valid(type), NULL);

	for (i = 0; i != sizeof(MU_EXPR_TYPE_INFO)/sizeof(MU_EXPR_TYPE_INFO[0]); ++i)
		if (type == MU_EXPR_TYPE_INFO[i]._type)
			return MU_EXPR_TYPE_INFO[i]._dbfield;

	g_return_val_if_reached (NULL);
}


static gboolean
expr_type_has_cat (MuExprType type, MuExprCat cat)
{
	int i;	
	for (i = 0; i != sizeof(MU_EXPR_TYPE_INFO)/sizeof(MU_EXPR_TYPE_INFO[0]); ++i)
		if (MU_EXPR_TYPE_INFO[i]._type == type)
			return MU_EXPR_TYPE_INFO[i]._cat == cat;
	return FALSE;

}

gboolean
mu_expr_type_is_string (MuExprType type)
{
	g_return_val_if_fail (mu_expr_type_is_valid(type), FALSE);
	return expr_type_has_cat (type, MU_EXPR_CAT_STRING);
}

gboolean
mu_expr_type_is_number (MuExprType type)
{
	g_return_val_if_fail (mu_expr_type_is_valid(type), FALSE);
	return expr_type_has_cat (type, MU_EXPR_CAT_NUMBER);
}

gboolean
mu_expr_type_is_interval (MuExprType type)
{
	g_return_val_if_fail (mu_expr_type_is_valid(type), FALSE);
	return expr_type_has_cat (type, MU_EXPR_CAT_INTERVAL);
}
