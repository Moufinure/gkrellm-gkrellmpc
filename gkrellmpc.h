/*
 * $Header: /cvsroot/gkrellmpc/gkrellmpc.h,v 1.17 2005/01/02 20:39:52 mina Exp $
 */

#ifndef _GKRELLMPC_H
#define _GKRELLMPC_H

void mpc_disable_plugin ();
void mpc_update_plugin ();
void mpc_create_plugin (GtkWidget *, gint);
GkrellmMonitor * gkrellm_init_plugin();

void mpc_cb_button_prev();
void mpc_cb_button_play_pause();
void mpc_cb_button_stop();
void mpc_cb_button_next();
void mpc_cb_button_eject();

void mpc_stack_images (GkrellmPiximage *, GkrellmPiximage *);

gint mpc_panel_expose_event(GtkWidget *, GdkEventExpose *);
void mpc_update_songname (gchar *);
void mpc_update_label (gchar *);
void mpc_sync_with_mpd();

void mpc_update_volume_position(GkrellmKrell *, gint);
gint mpc_cb_panel_motion(GtkWidget *, GdkEventMotion *, gpointer);
gint mpc_cb_panel_release(GtkWidget *, GdkEventButton *, gpointer);
gint mpc_cb_panel_press(GtkWidget *, GdkEventButton *, gpointer);
gint mpc_cb_panel_scroll(GtkWidget *, GdkEventScroll *, gpointer);

void mpc_cb_panel_dragdatareceived (GtkWidget *widget, GdkDragContext *dc, gint x, gint y, GtkSelectionData *data, guint info, guint t, gpointer userdata);
gboolean mpc_cb_panel_dragmotion (GtkWidget *widget, GdkDragContext *dc, gint x, gint y, guint t, gpointer data);

#endif
