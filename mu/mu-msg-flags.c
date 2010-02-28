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

#include <glib.h>
#include "mu-msg-flags.h"

static struct {
	char       kar;
	MuMsgFlags flag;
	gboolean   file_flag;
} FLAG_CHARS[] = {
	{'D', MU_MSG_FLAG_DRAFT, TRUE},
	{'F', MU_MSG_FLAG_FLAGGED, TRUE},
	{'N', MU_MSG_FLAG_NEW, TRUE},
	{'P', MU_MSG_FLAG_PASSED, TRUE},
	{'R', MU_MSG_FLAG_REPLIED, TRUE},
	{'S', MU_MSG_FLAG_SEEN, TRUE},
	{'T', MU_MSG_FLAG_TRASHED, TRUE},
	{'a', MU_MSG_FLAG_HAS_ATTACH, FALSE},
	{'s', MU_MSG_FLAG_SIGNED, FALSE},
	{'x', MU_MSG_FLAG_ENCRYPTED, FALSE}
};


MuMsgFlags
mu_msg_flags_from_str (const char* str)
{
	MuMsgFlags flags = 0;	
	while (str[0]) {
		int i;
		MuMsgFlags oneflag = MU_MSG_FLAG_UNKNOWN;
		for (i = 0; i != G_N_ELEMENTS(FLAG_CHARS); ++i) {
			if (str[0] == FLAG_CHARS[i].kar) {
				oneflag = FLAG_CHARS[i].flag;
				break;
			}
		}
		if (oneflag == MU_MSG_FLAG_UNKNOWN)
			return MU_MSG_FLAG_UNKNOWN;
		else
			flags |= oneflag;

		++str;
	}
 
	return flags;
}

MuMsgFlags
mu_msg_flags_from_char (char c)
{
	char str[2];

	str[0] = c;
	str[1] = '\0';

	return mu_msg_flags_from_str (str);
}


const char*
mu_msg_flags_to_str_s  (MuMsgFlags flags)
{
	int i = 0, j = 0;
	static char buf[G_N_ELEMENTS(FLAG_CHARS) + 1];
	
	for (i = 0; i != G_N_ELEMENTS(FLAG_CHARS); ++i)
		if (flags & FLAG_CHARS[i].flag)
			buf[j++] = FLAG_CHARS[i].kar;
	buf[j] = '\0';
	return buf;
}


gboolean
mu_msg_flags_is_file_flag (MuMsgFlags flag)
{
	int i = 0;
	
	for (i = 0; i != G_N_ELEMENTS(FLAG_CHARS); ++i)
		if (flag == FLAG_CHARS[i].flag)
			return FLAG_CHARS[i].file_flag;

	return FALSE;
}
