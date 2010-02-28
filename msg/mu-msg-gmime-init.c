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

#include <gmime/gmime.h>
#include "mu-msg-gmime-init.h"

static gboolean _initialized = FALSE;

void 
mu_msg_gmime_init  (void)
{
	if (!_initialized) {
		g_mime_init(0);
		_initialized = TRUE;
	}
}


void
mu_msg_gmime_uninit (void)
{
	if (_initialized) {
		g_mime_shutdown();
		_initialized = FALSE;
	}	
}  
