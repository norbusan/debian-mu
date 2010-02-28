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


#include "mu-msg-view.h"
/* include other impl specific header files */

/* 'private'/'protected' functions */
static void mu_msg_view_class_init (MuMsgViewClass *klass);
static void mu_msg_view_init       (MuMsgView *obj);
static void mu_msg_view_finalize   (GObject *obj);
/* list my signals  */
enum {
	/* MY_SIGNAL_1, */
	/* MY_SIGNAL_2, */
	LAST_SIGNAL
};

typedef struct _MuMsgViewPrivate MuMsgViewPrivate;
struct _MuMsgViewPrivate {
	MuMsgHeaderView *_header;
	MuMsgTextView   *_body;
};
#define MU_MSG_VIEW_GET_PRIVATE(o)      (G_TYPE_INSTANCE_GET_PRIVATE((o), \
                                         MU_TYPE_MSG_VIEW, \
                                         MuMsgViewPrivate))
/* globals */
static GtkVBoxClass *parent_class = NULL;

/* uncomment the following if you have defined any signals */
/* static guint signals[LAST_SIGNAL] = {0}; */

GType
mu_msg_view_get_type (void)
{
	static GType my_type = 0;
	if (!my_type) {
		static const GTypeInfo my_info = {
			sizeof(MuMsgViewClass),
			NULL,		/* base init */
			NULL,		/* base finalize */
			(GClassInitFunc) mu_msg_view_class_init,
			NULL,		/* class finalize */
			NULL,		/* class data */
			sizeof(MuMsgView),
			1,		/* n_preallocs */
			(GInstanceInitFunc) mu_msg_view_init,
			NULL
		};
		my_type = g_type_register_static (GTK_TYPE_VBOX,
		                                  "MuMsgView",
		                                  &my_info, 0);
	}
	return my_type;
}

static void
mu_msg_view_class_init (MuMsgViewClass *klass)
{
	GObjectClass *gobject_class;
	gobject_class = (GObjectClass*) klass;

	parent_class            = g_type_class_peek_parent (klass);
	gobject_class->finalize = mu_msg_view_finalize;

	g_type_class_add_private (gobject_class, sizeof(MuMsgViewPrivate));

	/* signal definitions go here, e.g.: */
/* 	signals[MY_SIGNAL_1] = */
/* 		g_signal_new ("my_signal_1",....); */
/* 	signals[MY_SIGNAL_2] = */
/* 		g_signal_new ("my_signal_2",....); */
/* 	etc. */
}

static void
mu_msg_view_init (MuMsgView *obj)
{
 	MuMsgViewPrivate *priv;
	GtkWidget *scrollwin;

	priv = MU_MSG_VIEW_GET_PRIVATE(obj); 
	
	priv->_header = MU_MSG_HEADER_VIEW(mu_msg_header_view_new ());
	priv->_body   = MU_MSG_TEXT_VIEW(mu_msg_text_view_new ());

	scrollwin = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scrollwin),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_scrolled_window_add_with_viewport 
		(GTK_SCROLLED_WINDOW(scrollwin), GTK_WIDGET(priv->_body));

	gtk_box_pack_start (GTK_BOX(obj), GTK_WIDGET(priv->_header), FALSE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX(obj), scrollwin, TRUE, TRUE, 2);
}

static void
mu_msg_view_finalize (GObject *obj)
{
/* 	free/unref instance resources here */
	G_OBJECT_CLASS(parent_class)->finalize (obj);
}

GtkWidget*
mu_msg_view_new (void)
{
	return GTK_WIDGET(g_object_new(MU_TYPE_MSG_VIEW, NULL));
}

gboolean
mu_msg_view_set_message  (MuMsgView* view, MuMsgGMime *msg)
{
	g_return_val_if_fail (MU_IS_MSG_VIEW(view), FALSE);
	
	if (!mu_msg_header_view_set_message 
	    (MU_MSG_VIEW_GET_PRIVATE(view)->_header, msg))
		return FALSE;
	
	if (!mu_msg_text_view_set_message 
	    (MU_MSG_VIEW_GET_PRIVATE(view)->_body, msg))
		return FALSE;

	return TRUE;
}

MuMsgHeaderView*
mu_msg_view_get_header (MuMsgView* view)
{
	g_return_val_if_fail (MU_IS_MSG_VIEW(view), NULL);
	return MU_MSG_VIEW_GET_PRIVATE(view)->_header;
}


MuMsgTextView*
mu_msg_view_get_body   (MuMsgView* view)
{
	g_return_val_if_fail (MU_IS_MSG_VIEW(view), NULL);
	return MU_MSG_VIEW_GET_PRIVATE(view)->_body;
}

