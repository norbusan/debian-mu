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

#include <alloca.h>
#include <dirent.h>
#include <errno.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <glib/gstdio.h>
#include <string.h>

#include "mu/mu.h"
#include "mu/mu-msg-flags.h"

#include "mu-path.h"

static MuResult process_dir (const char* path, gboolean sort_inodes,
			     MuWalkCallback cb, void *data);

/*
 * is this a 'new' msg or a 'cur' msg?; if new, we return
 * (in info) a ptr to the info part 
 */
enum _MsgType {
	MSG_TYPE_CUR,
	MSG_TYPE_NEW,
	MSG_TYPE_OTHER
};
typedef enum _MsgType MsgType;


MsgType
check_msg_type (const char* path, char **info)
{
	char *dir, *file; 
	MsgType mtype;

	/* try to find the info part */
	/* note that we can use either the ':' or '!' as separator;
	 * the former is the official, but as it does not work on e.g. VFAT
	 * file systems, some Maildir implementations use the latter instead
	 * (or both). For example, Tinymail/modest does this. The python
	 * documentation at http://docs.python.org/lib/mailbox-maildir.html
	 * mentions the '!' as well as a 'popular choice'
	 */
	dir =  g_path_get_dirname (path);
	file = g_path_get_basename (path);
	
	if (!(*info = strrchr(file, ':')))
		*info = strrchr (file, '!'); /* Tinymail */
	if (*info)
		++(*info); /* skip the ':' or '!' */
	
	if (g_str_has_suffix (dir, G_DIR_SEPARATOR_S "cur")) {
		if (!*info)
			g_message ("'cur' file, but no info part: %s", path);
		mtype = MSG_TYPE_CUR;
	} else if (g_str_has_suffix (dir, G_DIR_SEPARATOR_S "new")) {
		if (*info)
			g_message ("'new' file, ignoring info part: %s", path);
		mtype = MSG_TYPE_NEW;
	} else 
		mtype = MSG_TYPE_OTHER; /* file has been added explicitly as
					   a single message */
	if (*info)
		*info = g_strdup (*info);

	g_free (dir);
	g_free (file);
	
	return mtype;
}

MuMsgFlags
mu_path_get_file_flags (const char* path)
{
	MuMsgFlags flags;
	MsgType mtype;
	char *info = NULL;

	g_return_val_if_fail (path, MU_MSG_FLAG_UNKNOWN);
	g_return_val_if_fail (!g_str_has_suffix(path,G_DIR_SEPARATOR_S),
			      MU_MSG_FLAG_UNKNOWN);
	
	mtype = check_msg_type (path, &info);
	
	/* we ignore any flags for a new message */
	if (mtype == MSG_TYPE_NEW) {
		g_free (info);
		return MU_MSG_FLAG_NEW;
	}
	
	flags = 0;
	if (mtype == MSG_TYPE_CUR || mtype == MSG_TYPE_OTHER) {
		char *cursor = info;
		/* only support the "2," format */
		if (cursor && cursor[0]=='2' && cursor[1]==',') {
			cursor += 2; /* jump past 2, */
			while (*cursor) {
				MuMsgFlags oneflag = 
					mu_msg_flags_from_char (*cursor);
				/* ignore anything but file flags */
				if (mu_msg_flags_is_file_flag(oneflag))
					flags |= oneflag;			
				++cursor;
			}
		}
	} 
	g_free (info);
	return flags;
}

gboolean
mu_path_symlink_message (const char* srcpath, const char* targetmdir)
{
	MuMsgFlags flags;
	char *targetpath;
	int result;
	char *filename;

	g_return_val_if_fail (srcpath, FALSE);
	g_return_val_if_fail (targetmdir, FALSE);
		
	/* do a lot of checking...; of course checking is racy,
	 * but we can give user better feedback than a failing
	 * symlink() call */

	if (access (srcpath, F_OK) != 0) {
		g_warning ("%s: not found '%s': %s", __FUNCTION__, srcpath,
			   strerror(errno));
		return FALSE;
	}
	
	/* source checking */
	if (!g_file_test (srcpath, G_FILE_TEST_IS_REGULAR)) {
		g_warning ("%s: not a valid source: %s", __FUNCTION__, srcpath);
		return FALSE;
	}
	
	flags = mu_path_get_file_flags (srcpath);
	if (flags == MU_MSG_FLAG_UNKNOWN) {
		g_warning ("%s: not a valid source message: %s", __FUNCTION__, 
			   srcpath);
		return FALSE;
	}

	/* target checking  */
	if (!g_file_test (targetmdir, G_FILE_TEST_IS_DIR)) {
		g_warning ("%s: not a valid target: %s", __FUNCTION__, targetmdir);
		return FALSE;
	}

	/* not sure if symlink(2) behaves like the cmdline utility;
	 * ie, 'ln -s foo bar/' creates the bar/foo symlink, without
	 * explicit foo in target. the symlink(2) manpage is silent,
	 * on this, so we take no risk
	 */
	filename   = g_path_get_basename (srcpath);
	targetpath = g_strdup_printf ("%s%c%s%c%s", targetmdir, 
				      G_DIR_SEPARATOR, 
				      (flags&MU_MSG_FLAG_NEW) ? "new":"cur",
				      G_DIR_SEPARATOR, filename);
	g_free (filename);

	result = symlink (srcpath, targetpath); 
	if (result < 0)
		g_warning ("%s: symlink failed '%s'->'%s': %s",
			   __FILE__, srcpath, targetpath, strerror(errno));
	g_free (targetpath);

	return result == 0;
}

gboolean
mu_path_make_maildir (const char* path, mode_t mode)
{
	int i;
	const char* subs[] = { "cur", "new", "tmp" };

	g_return_val_if_fail (path, FALSE);

	if (!path) {
		errno = EINVAL;
		return FALSE;
	}
	
	/* first create with 0700, otherwise we can't create subdirs...*/
	if (mkdir (path, 0700) < 0)
		return FALSE;
	
	for (i = 0; i != sizeof(subs)/sizeof(subs[0]); ++i) {
		int retval;
		char *fullpath = 
			g_strdup_printf ("%s%c%s", path,G_DIR_SEPARATOR,subs[i]);
		retval = mkdir (fullpath, mode);
		g_free (fullpath);
		if (retval < 0)
			return FALSE;
	}
	
	/* now give the parent dir the real mode */
	if (chmod (path, mode) < 0) 
		return FALSE;
	
	return TRUE;
}




static MuResult 
process_file (const char* fullpath, MuWalkCallback cb, void *data)
{
	MuResult result;
	struct stat statbuf;
	
	if (!cb)
		return MU_OK;

	if (access(fullpath, R_OK) != 0) {
		g_warning ("cannot access %s: %s", fullpath, strerror(errno));
		return MU_ERROR;
	}
	
	if (stat (fullpath, &statbuf) != 0) {
		g_warning ("cannot stat %s: %s", fullpath, strerror(errno));
		return MU_ERROR;
	}

	result = (cb)(fullpath,statbuf.st_mtime,data);
	
	switch (result) {
	case MU_OK:
		return result;
	case MU_STOP:
		g_message ("%s: callback told us to stop", __FUNCTION__);
		return result;
	default:
		g_warning ("%s: failed %d in callback (%s)",  
			   __FUNCTION__, result, fullpath);
		return MU_ERROR;
	}
}


/* determine if path is a maildir leaf-dir; ie. if it's 'cur' or 'new'
 * (we're ignoring 'tmp' for obvious reasons)
 */
static gboolean
is_maildir_new_or_cur (const char *path)
{
	size_t len;
	const char *sfx;

	/* path is the full path; it cannot possibly be shorter
	 * than 4 for a maildir (/cur or /new)
	 */
	if (!path||(len = strlen(path)) < 4)
		return FALSE;
	
	sfx = &path[len - 4];

	if (sfx[0] != G_DIR_SEPARATOR) /* small optimization */
		return FALSE;
	else
		return (strcmp (sfx + 1, "cur") == 0 || 
			strcmp (sfx + 1, "new") == 0);
}

static MuResult
process_dir_entry (const char* path,struct dirent *entry,
		   gboolean sort_inodes,
		   MuWalkCallback cb, void *data)
{
	char* fullpath;
	
	/* ignore dot-files, but not dot-dirs except for '.' and '..' 
	 *  not sure if it's guaranteed to never get . and .. from
	 * readdir; manpage is silent on this 
	 */
	if (entry->d_name[0] == '.') {
		if (entry->d_type == DT_REG) 
			return MU_OK;
		if (entry->d_name[1] == '\0')
			return MU_OK; /* ignore directory '.' */
		if (entry->d_name[1] == '.' && entry->d_name[2] == '\0')
			return MU_OK; /* ignore directory '..' */
	}

	fullpath = (char*)alloca(strlen(path) + 1 + strlen(entry->d_name) + 1);
	sprintf (fullpath, "%s%c%s", path, G_DIR_SEPARATOR, entry->d_name);
	
	switch (entry->d_type) {
	case DT_REG:
		/* we only want files in cur/ and new/ */
		if (!is_maildir_new_or_cur (path)) {
			g_debug ("'%s' is not a leaf dir", path);
			return MU_OK; 
		}
		return process_file (fullpath, cb, data);
		
	case DT_DIR:
		return process_dir (fullpath, sort_inodes, cb, data);
	
	default:
		return MU_OK; /* ignore other types */
	}
}

static struct dirent* 
dirent_copy (struct dirent *entry)
{
	struct dirent *d = g_slice_new (struct dirent);
	/* NOTE: simply memcpy'ing sizeof(struct dirent) bytes will
	 * give memory errors*/
	return (struct dirent*)memcpy (d, entry, entry->d_reclen);
}

static void
dirent_destroy (struct dirent *entry)
{
	g_slice_free(struct dirent, entry);
}

static gint
dirent_cmp (struct dirent *d1, struct dirent *d2)
{
	return d1->d_ino - d2->d_ino;
}


/* the next two functions, process_dir_inode_order, and process_dir,
 * are two strategies to scan a dir; the latter one scans the dir in
 * the order of direntries that readdir presents; the former one first
 * sorts the direntries in ascending order of inode, which could be faster when using 
 * ext3 with dir_index enables (tune2fs), as suggested:
 * http://does-not-exist.org/mail-archives/mutt-dev/msg00205.html
 */
static MuResult
process_dir_inode_order (DIR *dir, const char* path, MuWalkCallback cb, void *data)
{
	MuResult result = MU_OK;
	GList *lst, *c;
	struct dirent *entry;

	lst = NULL;
	while ((entry = readdir (dir)))
		lst = g_list_prepend (lst, dirent_copy(entry));
	
	c = lst = g_list_sort (lst, (GCompareFunc)dirent_cmp);
	
	for (c = lst; c && result == MU_OK; c = c->next) 
		result = process_dir_entry (path, (struct dirent*)c->data, 
					    TRUE, cb, data);

	g_list_foreach (lst, (GFunc)dirent_destroy, NULL);
	g_list_free (lst);
	
	return result;
}

static MuResult
process_dir_random_order (DIR *dir, const char* path, MuWalkCallback cb, 
			  void *data)
{
	struct dirent *entry;
	MuResult result = MU_OK;
	
	while ((entry = readdir (dir))) {
		result = process_dir_entry (path, entry, FALSE, 
					    cb, data);
		if (result != MU_OK)
			break;
	}
	
	return result;
}

static MuResult
process_dir (const char* path, gboolean sort_inodes,
	     MuWalkCallback cb, void *data)
{
	MuResult result;
	DIR* dir = opendir (path);		
	if (!dir) {
		g_warning ("failed to open %s: %s", path, strerror(errno));
		return MU_ERROR;
	}
	
	rewinddir (dir);
	if (sort_inodes)
		result = process_dir_inode_order (dir, path, cb, data);
	else
		result = process_dir_random_order (dir, path, cb, data);
	
	closedir (dir);
	return result;
}

MuResult
mu_path_walk_maildir (const char *path, gboolean sort_inodes,
		      MuWalkCallback cb, void *data)
{
	struct stat statbuf;

	g_return_val_if_fail (path && cb, MU_ERROR);


	if (!cb)
		return MU_OK;
	
	if (access(path, R_OK) != 0) {
		g_warning ("cannot access %s: %s", path, strerror(errno));
		return MU_ERROR;
	}
	
	if (stat (path, &statbuf) != 0) {
		g_warning ("cannot stat %s: %s", path, strerror(errno));
		return MU_ERROR;
	}

	g_message ("starting dir scan on %s, sort inodes: %s",
		   path, sort_inodes ? "yes" : "no");
	
	if ((statbuf.st_mode & S_IFMT) == S_IFREG)
		return process_file (path, cb, data);

	if ((statbuf.st_mode & S_IFMT) == S_IFDIR) 
		return process_dir (path, sort_inodes, cb, data);

	g_warning ("%s: unsupported file type for %s", __FUNCTION__, path);
	return MU_ERROR;
}


