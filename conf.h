/*
 * $Header: /cvsroot/gkrellmpc/conf.h,v 1.1 2004/12/30 20:49:37 mina Exp $
 */

#ifndef _CONF_H
#define _CONF_H

void mpc_create_plugin_tab (GtkWidget *);
void mpc_apply_plugin_config ();
void mpc_save_plugin_config (FILE *);
void mpc_load_plugin_config (gchar *);

#endif
