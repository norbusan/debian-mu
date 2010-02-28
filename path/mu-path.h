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

#ifndef __MU_PATH_H__
#define __MU_PATH_H__

#include <glib.h>
#include <sys/types.h> /* for mode_t */

#include "mu/mu.h"          /* for MuResult */
#include "mu/mu-msg-flags.h" 

/** 
 * get the Maildir flags from a mailfile. The flags are as specified
 * in http://cr.yp.to/proto/maildir.html, plus MU_MSG_FLAG_NEW for
 * new messages, ie the ones that live in new/. The flags are
 * logically OR'ed. Note that the file does not have to exist; the
 * flags are based on the name only. 
 *
 * @param pathname of a mailfile; it does not have to refer to an
 * actual message
 * 
 * @return the flags, or MU_MSG_FILE_FLAG_UNKNOWN in case of error
 */
MuMsgFlags mu_path_get_file_flags (const char* pathname);


/** 
 * create a symbolic link to a mailfile at @param srcpath. Note that
 * the srcpath is the full path to the source message, while the
 * @param targetmdir is a the path to an existing maildir
 * 
 * @param srcpath symlink to an existing message
 * @param targetmdir path to an existing maildir 
 * 
 * @return true if it succeeded, false otherwise. errno might contain
 * more information
 */
gboolean    mu_path_symlink_message   (const char* srcpath, 
				       const char* targetmdir);

/** 
 * create a new maildir
 * 
 * @param path path to the new maildir 
 * @param mode file mode for the
 * new maildir. '0700' is a good choice.
 * 
 * @return TRUE if it succeeded, FALSE otherwise. errno may have more
 * information.
 */
gboolean    mu_path_make_maildir   (const char *path, mode_t mode);


/** 
 * MuWalkCallback -- callback function for mu_path_walk_maildir; see the
 * documentation there.
 */
typedef MuResult (*MuWalkCallback) (const char* fullpath,
				    time_t timestamp,
				    void *user_data);

/** 
 * start a recursive scan of a maildir; for each file found, we call
 * callback with the path (with the Maildir path of scanner_new as
 * root), the filename, the timestamp (mtime) of the file,and the
 * *data pointer, for user data.  dot-files are ignored, as well as
 * files outside cur/ and new/ dirs and unreadable files; however,
 * dotdirs are visited (ie. '.dotdir/cur'), so this enables Maildir++.
 * (http://www.inter7.com/courierimap/README.maildirquota.html,
 *  search for 'Mission statement')
 *
 * mu_path_walk_maildir wills stop if the callback returns something
 * != MU_OK. For example, it can return
 * MU_STOP to stop the scan , or some error.
 * 
 * @param path the maildir path to scan
 * @param inode_order if TRUE, sort directories by inode order,
 * which should be faster on ext3 with dir_index (see tune2fs)
 * @param cb the callback function
 * @param data user data pointer
 * 
 * @return a scanner result; MU_OK if everything went ok, 
 * MU_STOP if we want to stop, or MU_ERROR in
 * case of error
 */
MuResult mu_path_walk_maildir (const char *path, 
			       gboolean inode_order,
			       MuWalkCallback cb, 
			       void *data);


#endif /*__MU_PATH_H__*/

