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

#include <glib/gi18n.h>
#include <string.h>

#include "mu-index-runner.h"


/* 'private'/'protected' functions */
static void mu_index_runner_class_init (MuIndexRunnerClass *klass);
static void mu_index_runner_init       (MuIndexRunner *obj);
static void mu_index_runner_finalize   (GObject *obj);

/* list my signals  */
enum {
	/* MY_SIGNAL_1, */
	/* MY_SIGNAL_2, */
	LAST_SIGNAL
};

typedef struct _MuIndexRunnerPrivate MuIndexRunnerPrivate;
struct _MuIndexRunnerPrivate {
	MuIndex        *_index;
	gchar*          _maildir;
	guint           _total;

	GtkProgressBar *_progress;
	GtkLabel       *_label;
	MuResult        _result;
};
#define MU_INDEX_RUNNER_GET_PRIVATE(o)      (G_TYPE_INSTANCE_GET_PRIVATE((o), \
                                             MU_TYPE_INDEX_RUNNER, \
                                             MuIndexRunnerPrivate))
/* globals */
static GtkVBoxClass *parent_class = NULL;

/* uncomment the following if you have defined any signals */
/* static guint signals[LAST_SIGNAL] = {0}; */

GType
mu_index_runner_get_type (void)
{
	static GType my_type = 0;
	if (!my_type) {
		static const GTypeInfo my_info = {
			sizeof(MuIndexRunnerClass),
			NULL,		/* base init */
			NULL,		/* base finalize */
			(GClassInitFunc) mu_index_runner_class_init,
			NULL,		/* class finalize */
			NULL,		/* class data */
			sizeof(MuIndexRunner),
			1,		/* n_preallocs */
			(GInstanceInitFunc) mu_index_runner_init,
			NULL
		};
		my_type = g_type_register_static (GTK_TYPE_VBOX,
		                                  "MuIndexRunner",
		                                  &my_info, 0);
	}
	return my_type;
}

static void
mu_index_runner_class_init (MuIndexRunnerClass *klass)
{
	GObjectClass *gobject_class;
	gobject_class = (GObjectClass*) klass;

	parent_class            = g_type_class_peek_parent (klass);
	gobject_class->finalize = mu_index_runner_finalize;

	g_type_class_add_private (gobject_class, sizeof(MuIndexRunnerPrivate));

	/* signal definitions go here, e.g.: */
/* 	signals[MY_SIGNAL_1] = */
/* 		g_signal_new ("my_signal_1",....); */
/* 	signals[MY_SIGNAL_2] = */
/* 		g_signal_new ("my_signal_2",....); */
/* 	etc. */
}


static void
mu_index_runner_init (MuIndexRunner *obj)
{
 	MuIndexRunnerPrivate *priv = MU_INDEX_RUNNER_GET_PRIVATE(obj);

	priv->_index = NULL;
	priv->_maildir = NULL;
	priv->_result = MU_OK;

	priv->_progress = GTK_PROGRESS_BAR(gtk_progress_bar_new());
	priv->_label    = GTK_LABEL(gtk_label_new ("Indexing..."));

	gtk_box_pack_start (GTK_BOX(obj), GTK_WIDGET(priv->_progress), 
			    TRUE, FALSE, 6);
	gtk_box_pack_start (GTK_BOX(obj), GTK_WIDGET(priv->_label), 
			    TRUE, FALSE, 6);

	gtk_widget_show (GTK_WIDGET(priv->_progress));
	gtk_widget_show (GTK_WIDGET(priv->_label));

	
}

static void
mu_index_runner_finalize (GObject *obj)
{	
	g_free (MU_INDEX_RUNNER_GET_PRIVATE(obj)->_maildir);

	G_OBJECT_CLASS(parent_class)->finalize (obj);
}


GtkWidget*
mu_index_runner_new (MuIndex *index, const char* maildir)
{
	GtkWidget *w;

	g_return_val_if_fail (index, NULL);
	g_return_val_if_fail (maildir, NULL);

	w = GTK_WIDGET(g_object_new(MU_TYPE_INDEX_RUNNER, NULL));	

	MU_INDEX_RUNNER_GET_PRIVATE(w)->_index   = index;
	MU_INDEX_RUNNER_GET_PRIVATE(w)->_maildir = g_strdup(maildir);
	
	return w;
}


static MuResult
index_stats_callback (MuIndexStats *stats, MuIndexRunner *runner)
{	
	MuIndexRunnerPrivate *priv;
	priv = MU_INDEX_RUNNER_GET_PRIVATE(runner);

	gtk_progress_bar_pulse (priv->_progress);

	return priv->_result;

}

static MuResult
index_run_callback (MuIndexStats *stats, MuIndexRunner *runner)
{	
	MuIndexRunnerPrivate *priv;

	priv = MU_INDEX_RUNNER_GET_PRIVATE(runner);

	if (priv->_result != MU_OK)
		return priv->_result;
	
	if (priv->_total) {

		gdouble frac;
		gchar *str;
		
		frac = (gdouble)stats->_processed / (gdouble)priv->_total;
		str = g_strdup_printf ("%.1f%%", frac*100);

		gtk_progress_bar_set_fraction (priv->_progress, frac);
		gtk_progress_bar_set_text (priv->_progress, str);

		g_free (str);

	} else 
		gtk_progress_bar_pulse (priv->_progress);

	while (g_main_context_pending (NULL))
		g_main_context_iteration (NULL, TRUE);

	return MU_OK;
}


static void
runner_count (MuIndexRunner *runner)
{
	MuIndexRunnerPrivate *priv;
	MuIndexStats stats;
	gchar *str;
	MuResult result;
	
	g_return_if_fail (MU_IS_INDEX_RUNNER(runner));
	priv = MU_INDEX_RUNNER_GET_PRIVATE(runner);

	priv->_result = MU_OK;

	/* counting */
	gtk_label_set_text (priv->_label, _("Counting messages"));
	memset (&stats, 0, sizeof(MuIndexStats));
	result = mu_index_stats (priv->_index, priv->_maildir, &stats, 
				 (MuIndexCallback)index_stats_callback,
				 runner);

	priv->_total = stats._processed;
	str = g_strdup_printf (_("Found %d messages"), priv->_total);
	gtk_label_set_text (priv->_label, str);
	g_free (str);
}



static void
runner_cleanup (MuIndexRunner *runner)
{
	MuIndexRunnerPrivate *priv;
	MuIndexStats stats;
	gchar *str;
	MuResult result;
	
	g_return_if_fail (MU_IS_INDEX_RUNNER(runner));
	priv = MU_INDEX_RUNNER_GET_PRIVATE(runner);

	/* cleanup */
	gtk_label_set_text (priv->_label, "Cleaning up...");
	memset (&stats, 0, sizeof(MuIndexStats));
	result = mu_index_cleanup (priv->_index, &stats, 
				   (MuIndexCallback)index_stats_callback,
				   runner);
	
	str = g_strdup_printf (_("Cleaned up %d message(s) from database"), 
			       stats._cleaned_up);
	gtk_label_set_text (priv->_label, str);
	g_free (str);
}


static MuResult
runner_index (MuIndexRunner *runner)
{
	MuIndexRunnerPrivate *priv;
	MuIndexStats stats;
	gchar *str;
	MuResult result;
	
	g_return_val_if_fail (MU_IS_INDEX_RUNNER(runner), MU_ERROR);
	priv = MU_INDEX_RUNNER_GET_PRIVATE(runner);

	/* index */
	str = g_strdup_printf (_("Indexing %s and its children"), 
			       priv->_maildir); 
	gtk_label_set_text (priv->_label, str);
	g_free (str);
	memset (&stats, 0, sizeof(MuIndexStats));
	result = mu_index_run (priv->_index, priv->_maildir, FALSE, &stats, 
		      (MuIndexCallback)index_run_callback, 
		      runner);
	if (result != MU_OK) 
		return result;
	
	gtk_progress_bar_set_fraction (priv->_progress, 1.0);
	gtk_progress_bar_set_text (priv->_progress, "100%");

	str = g_strdup_printf (_("Indexing finished.\n"
				 "Added %d message(s), updated %d"), 
			       stats._added, stats._updated); 
	gtk_label_set_text (priv->_label, str);
	g_free (str);
	
	return MU_OK;
}



MuResult
mu_index_runner_start (MuIndexRunner *runner)
{
	MuIndexRunnerPrivate *priv;

	g_return_val_if_fail (MU_IS_INDEX_RUNNER(runner), MU_ERROR);

	priv = MU_INDEX_RUNNER_GET_PRIVATE(runner);
	priv->_result = MU_OK;

	runner_count (runner);
	if (priv->_result != MU_OK)
		return priv->_result;

	runner_cleanup (runner);
	if (priv->_result != MU_OK)
		return priv->_result;
	
	return runner_index (runner);
}


void
mu_index_runner_stop  (MuIndexRunner *runner)
{
	g_return_if_fail (MU_IS_INDEX_RUNNER(runner));
	MU_INDEX_RUNNER_GET_PRIVATE(runner)->_result = MU_STOP;
}
