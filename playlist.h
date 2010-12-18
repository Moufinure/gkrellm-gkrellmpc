/*
 * $Header: /cvsroot/gkrellmpc/playlist.h,v 1.10 2005/03/06 02:29:07 mina Exp $
 */

#ifndef _PLAYLIST_H
#define _PLAYLIST_H

void mpc_playlist_create(void);
gboolean mpc_playlist_configure_event(GtkWidget *, GdkEventConfigure *, gpointer);
gint mpc_playlist_delete_event(GtkContainer *, GtkWidget *, gpointer);
gboolean mpc_playlist_update(void);
void mpc_cb_playlist_button_clear (GtkButton *, gpointer);
void mpc_cb_playlist_button_remove (GtkButton *, gpointer);
void mpc_cb_delete_foreach (GtkTreeModel *, GtkTreePath *, GtkTreeIter *, gpointer);
void mpc_cb_playlist_button_add (GtkButton *, gpointer);
void mpc_cb_playlist_button_close (GtkButton *, gpointer);
void mpc_cb_playlist_row (GtkTreeView *, GtkTreePath *, GtkTreeViewColumn *, gpointer);
gboolean mpc_cb_playlist_key (GtkWidget *, GdkEventKey *, gpointer);
void mpc_playlist_update_bold (void);
void mpc_playlist_destroy_event(GtkWidget *, gpointer);

#endif
