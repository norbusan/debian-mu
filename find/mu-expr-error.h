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

#ifndef __MU_EXPR_ERROR_H__
#define __MU_EXPR_ERROR_H__

enum _MuExprError {
	MU_EXPR_ERROR_BOOKMARK_NOT_FOUND,
	MU_EXPR_ERROR_DATE_OUT_OF_RANGE,
	MU_EXPR_ERROR_INVALID_CRITERION,
	MU_EXPR_ERROR_INVALID_ESCAPE_SEQ,
	MU_EXPR_ERROR_INVALID_DATE,
	MU_EXPR_ERROR_INVALID_FLAG,
	MU_EXPR_ERROR_INVALID_INTERVAL,
	MU_EXPR_ERROR_INVALID_PRIORITY,
	MU_EXPR_ERROR_INVALID_SIZE,
	MU_EXPR_ERROR_MIXED_TYPE_CRITERIA,
	MU_EXPR_ERROR_NO_COLON,
	MU_EXPR_ERROR_NO_CRITERIA,
	MU_EXPR_ERROR_NO_VALUE,
	MU_EXPR_ERROR_UNTERMINATED_QUOTES,

	/* TODO: move these elsewhere: */
	MU_EXPR_ERROR_BOOKMARKS_FILE,
	MU_EXPR_ERROR_XAPIAN_INIT,
	MU_EXPR_ERROR_SQLITE_INIT
};



#endif /*__MU_EXPR_ERROR_H__*/
