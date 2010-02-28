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

#ifndef __MU_LOG_H__
#define __MU_LOG_H__

#include <glib.h>

struct _MuLog;
typedef struct _MuLog MuLog;

MuLog* mu_log_new_with_fd    (int fd, int doclose);
MuLog* mu_log_new_with_file  (const char* file, int append);
void   mu_log_destroy        (MuLog *mulog);
void   mu_log_write          (MuLog *log, const char* domain, 
			      GLogLevelFlags level, 
			      const gchar *msg);

/*
 * suffix of the log file (name will be '<appname>.log'
 */
#define MU_LOG_SUFFIX "log"

#endif /*__MU_LOG_H__*/
