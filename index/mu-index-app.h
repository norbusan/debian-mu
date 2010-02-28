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

#ifndef __MU_INDEX_APP_H__
#define __MU_INDEX_APP_H__

#include <glib.h>

#include "mu-index.h"

gboolean mu_index_app_init   (int *argcp, char***argvp, GError **err);
gboolean mu_index_app_uninit (void);

MuIndex* mu_index_app_get_index (GError **err);
gboolean mu_index_app_quiet (void);
const char* mu_index_app_get_maildir(void);
const gboolean mu_index_app_caught_signal (void);

#endif /*__MU_INDEX_APP_H__*/
