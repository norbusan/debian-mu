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
#include "mu/mu-msg-str.h"
#include "mu-msg-header-view.h"


/* 'private'/'protected' functions */
static void mu_msg_header_view_class_init (MuMsgHeaderViewClass *klass);
static void mu_msg_header_view_init       (MuMsgHeaderView *obj);
static void mu_msg_header_view_finalize   (GObject *obj);
/* list my signals  */
enum {
	/* MY_SIGNAL_1, */
	/* MY_SIGNAL_2, */
	LAST_SIGNAL
};

typedef struct _MuMsgHeaderViewPrivate MuMsgHeaderViewPrivate;
struct _MuMsgHeaderViewPrivate {
	GtkSizeGroup *_size_group;
};
#define MU_MSG_HEADER_VIEW_GET_PRIVATE(o)      (G_TYPE_INSTANCE_GET_PRIVATE((o), \
                                                MU_TYPE_MSG_HEADER_VIEW, \
                                                MuMsgHeaderViewPrivate))
/* globals */
static GtkVBoxClass *parent_class = NULL;

/* uncomment the following if you have defined any signals */
/* static guint signals[LAST_SIGNAL] = {0}; */

GType
mu_msg_header_view_get_type (void)
{
	static GType my_type = 0;
	if (!my_type) {
		static const GTypeInfo my_info = {
			sizeof(MuMsgHeaderViewClass),
			NULL,		/* base init */
			NULL,		/* base finalize */
			(GClassInitFunc) mu_msg_header_view_class_init,
			NULL,		/* class finalize */
			NULL,		/* class data */
			sizeof(MuMsgHeaderView),
			1,		/* n_preallocs */
			(GInstanceInitFunc) mu_msg_header_view_init,
			NULL
		};
		my_type = g_type_register_static (GTK_TYPE_VBOX,
		                                  "MuMsgHeaderView",
		                                  &my_info, 0);
	}
	return my_type;
}

static void
mu_msg_header_view_class_init (MuMsgHeaderViewClass *klass)
{
	GObjectClass *gobject_class;
	gobject_class = (GObjectClass*) klass;

	parent_class            = g_type_class_peek_parent (klass);
	gobject_class->finalize = mu_msg_header_view_finalize;

	g_type_class_add_private (gobject_class, sizeof(MuMsgHeaderViewPrivate));

	/* signal definitions go here, e.g.: */
/* 	signals[MY_SIGNAL_1] = */
/* 		g_signal_new ("my_signal_1",....); */
/* 	signals[MY_SIGNAL_2] = */
/* 		g_signal_new ("my_signal_2",....); */
/* 	etc. */
}


static void
add_header (MuMsgHeaderView *view, const gchar* field, const gchar* val)
{
	MuMsgHeaderViewPrivate *priv;
	GtkWidget *hbox, *field_label, *value_label;

	priv = MU_MSG_HEADER_VIEW_GET_PRIVATE(view);
	
	hbox        = gtk_hbox_new (FALSE, 12);
	field_label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(field_label), field);
	gtk_misc_set_alignment (GTK_MISC(field_label), 1.0, 0.0);
	gtk_size_group_add_widget (priv->_size_group, field_label);

	value_label = gtk_label_new (val);
	gtk_misc_set_alignment (GTK_MISC(value_label), 0.0, 0.0);

	gtk_box_pack_start (GTK_BOX(hbox), field_label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(hbox), value_label, TRUE, TRUE, 0);
	
	gtk_box_pack_start (GTK_BOX(view), hbox, FALSE, FALSE, 2);
	gtk_widget_show_all (hbox);
}


static void
clear_headers (MuMsgHeaderView *view)
{
//	gtk_container_foreach (GTK_CONTAINER(view),
//			       (GtkCallback)gtk_widget_destroy, NULL);
}


static void
mu_msg_header_view_init (MuMsgHeaderView *obj)
{
	MuMsgHeaderViewPrivate *priv;

	priv = MU_MSG_HEADER_VIEW_GET_PRIVATE(obj);
	priv->_size_group = gtk_size_group_new (GTK_SIZE_GROUP_BOTH);
}

static void
mu_msg_header_view_finalize (GObject *obj)
{		
	G_OBJECT_CLASS(parent_class)->finalize (obj);
}

GtkWidget*
mu_msg_header_view_new (void)
{
	return GTK_WIDGET(g_object_new(MU_TYPE_MSG_HEADER_VIEW, NULL));
}

gboolean
mu_msg_header_view_set_message  (MuMsgHeaderView *view, 
				 MuMsgGMime *msg)
{
	g_return_val_if_fail (MU_IS_MSG_HEADER_VIEW(view), FALSE);

	if (msg) {
		time_t date;

		clear_headers (view);
		add_header (view,"<b>From:</b>",
			    mu_msg_gmime_get_from(msg));
		add_header (view,"<b>Subject:</b>",
			    mu_msg_gmime_get_subject(msg));

		date = mu_msg_gmime_get_date(msg);
		if (date != 0)
			add_header (view,"<b>Date:</b>",
				    mu_msg_str_date_s(date));
 	} else 
		clear_headers(view);

	return TRUE;
}
