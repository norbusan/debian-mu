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

#ifndef __MU_INDEX_RUNNER_H__
#define __MU_INDEX_RUNNER_H__

#include <gtk/gtk.h>
#include "index/mu-index.h"

G_BEGIN_DECLS

/* convenience macros */
#define MU_TYPE_INDEX_RUNNER             (mu_index_runner_get_type())
#define MU_INDEX_RUNNER(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),MU_TYPE_INDEX_RUNNER,MuIndexRunner))
#define MU_INDEX_RUNNER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),MU_TYPE_INDEX_RUNNER,GtkVBox))
#define MU_IS_INDEX_RUNNER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),MU_TYPE_INDEX_RUNNER))
#define MU_IS_INDEX_RUNNER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),MU_TYPE_INDEX_RUNNER))
#define MU_INDEX_RUNNER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),MU_TYPE_INDEX_RUNNER,MuIndexRunnerClass))

typedef struct _MuIndexRunner      MuIndexRunner;
typedef struct _MuIndexRunnerClass MuIndexRunnerClass;

struct _MuIndexRunner {
	 GtkVBox parent;
};

struct _MuIndexRunnerClass {
	GtkVBoxClass parent_class;
};

/* member functions */
GType        mu_index_runner_get_type    (void) G_GNUC_CONST;
GtkWidget*   mu_index_runner_new         (MuIndex *index, const char* maildir);
MuResult     mu_index_runner_start       (MuIndexRunner *runner);
void         mu_index_runner_stop        (MuIndexRunner *runner);
G_END_DECLS

#endif /* __MU_INDEX_RUNNER_H__ */

