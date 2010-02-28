/* mu-msg-text-view.c */

/* insert (c)/licensing information) *//*
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

#include "msg/mu-msg-gmime.h"
#include "mu-msg-text-view.h"

/* 'private'/'protected' functions */
static void mu_msg_text_view_class_init (MuMsgTextViewClass *klass);
static void mu_msg_text_view_init       (MuMsgTextView *obj);
static void mu_msg_text_view_finalize   (GObject *obj);
/* list my signals  */
enum {
	/* MY_SIGNAL_1, */
	/* MY_SIGNAL_2, */
	LAST_SIGNAL
};

typedef struct _MuMsgTextViewPrivate MuMsgTextViewPrivate;
struct _MuMsgTextViewPrivate {
	GtkTextView *_body;
	MuMsgGMime  *_msg_gmime;

};
#define MU_MSG_TEXT_VIEW_GET_PRIVATE(o)      (G_TYPE_INSTANCE_GET_PRIVATE((o), \
                                              MU_TYPE_MSG_TEXT_VIEW, \
                                              MuMsgTextViewPrivate))
/* globals */
static GtkVBoxClass *parent_class = NULL;

/* uncomment the following if you have defined any signals */
/* static guint signals[LAST_SIGNAL] = {0}; */

GType
mu_msg_text_view_get_type (void)
{
	static GType my_type = 0;
	if (!my_type) {
		static const GTypeInfo my_info = {
			sizeof(MuMsgTextViewClass),
			NULL,		/* base init */
			NULL,		/* base finalize */
			(GClassInitFunc) mu_msg_text_view_class_init,
			NULL,		/* class finalize */
			NULL,		/* class data */
			sizeof(MuMsgTextView),
			1,		/* n_preallocs */
			(GInstanceInitFunc) mu_msg_text_view_init,
			NULL
		};
		my_type = g_type_register_static (GTK_TYPE_VBOX,
		                                  "MuMsgTextView",
		                                  &my_info, 0);
	}
	return my_type;
}

static void
mu_msg_text_view_class_init (MuMsgTextViewClass *klass)
{
	GObjectClass *gobject_class;
	gobject_class = (GObjectClass*) klass;

	parent_class            = g_type_class_peek_parent (klass);
	gobject_class->finalize = mu_msg_text_view_finalize;

	g_type_class_add_private (gobject_class, sizeof(MuMsgTextViewPrivate));

	/* signal definitions go here, e.g.: */
/* 	signals[MY_SIGNAL_1] = */
/* 		g_signal_new ("my_signal_1",....); */
/* 	signals[MY_SIGNAL_2] = */
/* 		g_signal_new ("my_signal_2",....); */
/* 	etc. */
}

static void
mu_msg_text_view_init (MuMsgTextView *obj)
{
 	MuMsgTextViewPrivate *priv = MU_MSG_TEXT_VIEW_GET_PRIVATE(obj);

	priv->_body = GTK_TEXT_VIEW(gtk_text_view_new ());

	gtk_text_view_set_editable (priv->_body, FALSE);
	gtk_text_view_set_cursor_visible (priv->_body, FALSE);
	gtk_text_view_set_wrap_mode (priv->_body, GTK_WRAP_WORD_CHAR);

	gtk_box_pack_start (GTK_BOX(obj), 
			    GTK_WIDGET(priv->_body), TRUE, TRUE, 5);
}

static void
mu_msg_text_view_finalize (GObject *obj)
{
	G_OBJECT_CLASS(parent_class)->finalize (obj);
}

GtkWidget*
mu_msg_text_view_new (void)
{
	return GTK_WIDGET(g_object_new(MU_TYPE_MSG_TEXT_VIEW, NULL));
}

gboolean
mu_msg_text_view_set_message (MuMsgTextView *view, MuMsgGMime *msg)
{
	MuMsgTextViewPrivate *priv;

	g_return_val_if_fail (MU_IS_MSG_TEXT_VIEW (view), FALSE);
	
	
	priv = MU_MSG_TEXT_VIEW_GET_PRIVATE(view);

	priv->_msg_gmime = NULL;
	
	if (msg) {
		priv->_msg_gmime = msg;
		gtk_text_buffer_set_text 
			(gtk_text_view_get_buffer(priv->_body),
			 mu_msg_gmime_get_body_text(priv->_msg_gmime), -1);
	} else 
		gtk_text_buffer_set_text 
			(gtk_text_view_get_buffer(priv->_body), "", -1);

	return TRUE;
}

