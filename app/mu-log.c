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

#include <stdio.h>

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include "mu-log.h"

struct _MuLog {
	int _fd;               /* log file descriptor */
	int _own;              /* close _fd with log_destroy? */
};


MuLog*
mu_log_new_with_fd (int fd, int doclose)
{
	MuLog *mulog;

	mulog = (MuLog*)malloc(sizeof(MuLog));
	if (!mulog) {
		fprintf (stderr, "%s: out of memory\n", __FUNCTION__);
		return NULL;
	}

	mulog->_fd     = fd;
	mulog->_own    = doclose; /* if we now own the fd, close it in _destroy */
	
	return mulog;
}


static void
try_close (int fd)
{
	if (close (fd) < 0) 
		fprintf (stderr, "%s: close() of fd %d failed: %s\n",
			 __FUNCTION__, fd, strerror(errno));
}


MuLog*
mu_log_new_with_file  (const char* file, int append)
{
	MuLog *mulog;
	int fd = 0;
	
	if (file) {
		fd = open (file,
			   O_WRONLY|O_CREAT|(append ? O_APPEND : O_TRUNC),
			   00600);
		if (fd < 0) {
			fprintf (stderr, "%s: open() of '%s' failed: %s\n",
				 __FUNCTION__, file, strerror(errno));
			return NULL;
		}
	}
			   
	mulog = mu_log_new_with_fd (fd, 1);
	if (!mulog)
		try_close (fd);

	return mulog;
}


void 
mu_log_destroy (MuLog *mulog)
{
	if (!mulog)
		return;

	if (mulog->_own)
		try_close (mulog->_fd);

	free (mulog);
}


const char*
pfx (GLogLevelFlags level)
{
	switch (level) {
	case G_LOG_LEVEL_WARNING:   return  "WARN";
	case G_LOG_LEVEL_ERROR :    return  "ERR ";
	case G_LOG_LEVEL_DEBUG:     return  "DBG ";
	case G_LOG_LEVEL_CRITICAL : return  "CRIT";
	case G_LOG_LEVEL_MESSAGE:   return  "MSG ";
	case G_LOG_LEVEL_INFO :     return  "INFO";
	default:                    return  "LOG "; 
	}
}

void
mu_log_write (MuLog *log, const char* domain, GLogLevelFlags level, 
	      const gchar *msg)
{
	time_t now;
	ssize_t len;

	/* log lines will be truncated at 255 chars */
	char buf [255], timebuf [32];
	
	g_return_if_fail (log);

	/* get the time/date string */
	now = time(NULL);
	strftime (timebuf, sizeof(timebuf), "%F %T", localtime(&now));
		
	/* now put it all together */
	len = snprintf (buf, sizeof(buf), "%s [%s] [%s] %s\n", timebuf, 
			pfx(level), domain, msg);
	
	len = write (log->_fd, buf, len);
	if (len < 0)
		fprintf (stderr, "%s: failed to write to log: %s\n",
			 __FUNCTION__,  strerror(errno));

	/* for serious errors, log them to stderr as well */
	if ((level & G_LOG_LEVEL_ERROR) ||
	    (level & G_LOG_LEVEL_CRITICAL) || 
	    (level & G_LOG_LEVEL_WARNING))
		g_printerr ("error: %s\n", msg);
}
