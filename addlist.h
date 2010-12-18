/*
 * $Header: /cvsroot/gkrellmpc/addlist.h,v 1.4 2005/03/06 02:29:07 mina Exp $
 */

#ifndef _ADDLIST_H
#define _ADDLIST_H

void mpc_addlist_create(void);
gboolean mpc_cb_addlist_key (GtkWidget *, GdkEventKey *, gpointer);
gboolean mpc_addlist_configure_event(GtkWidget *, GdkEventConfigure *, gpointer);
void mpc_addlist_destroy_event(GtkWidget *, gpointer);
gint mpc_addlist_delete_event(GtkContainer *, GtkWidget *, gpointer);
void mpc_cb_addlist_row (GtkTreeView *, GtkTreePath *, GtkTreeViewColumn *, gpointer);
void mpc_cb_addlist_button_add (GtkButton *, gpointer);
void mpc_cb_addlist_button_close (GtkButton *, gpointer);
gboolean mpc_addlist_update(void);
void mpc_cb_add_foreach (GtkTreeModel *, GtkTreePath *, GtkTreeIter *, gpointer);

#endif
