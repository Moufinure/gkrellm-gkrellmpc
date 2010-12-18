/*
 * $Header: /cvsroot/gkrellmpc/conf.c,v 1.9 2005/03/05 22:27:27 mina Exp $
 *
 * Holds all the functions for the configuration tab
 */

#include "globals.h"
#include "conf.h"
#include "mpd.h"
#include "gkrellmpc.h"

GtkWidget    * mpc_confwidget_hostname = NULL;
GtkWidget    * mpc_confwidget_port = NULL;
GtkWidget    * mpc_confwidget_scrollspeed = NULL;
GtkWidget    * mpc_confwidget_wheelaction = NULL;
GtkWidget    * mpc_confwidget_wheelamount = NULL;
GtkWidget    * mpc_confwidget_rightclick = NULL;
GtkWidget    * mpc_confwidget_middleclick = NULL;
GtkWidget    * mpc_confwidget_drop = NULL;
gint           mpc_conf_scrollspeed = 3;
MPC_WHEEL_ACTIONS mpc_conf_wheelaction = MPC_WHEEL_ACTION_VOLUME;
gint           mpc_conf_wheelamount = 10;
gchar        * mpc_conf_hostname = NULL;
gint           mpc_conf_port = 6600;
MPC_RIGHTCLICK_ACTIONS mpc_conf_rightclick = MPC_RIGHTCLICK_ACTION_CONFIG;
MPC_MIDDLECLICK_ACTIONS mpc_conf_middleclick = MPC_MIDDLECLICK_ACTION_PAUSE;
MPC_DROP_ACTIONS mpc_conf_drop = MPC_DROP_ACTION_ADDPLAY;

/*
 * Gets called when it's time to create the configuration tab
 */
void mpc_create_plugin_tab (GtkWidget *tab) {
	GtkWidget *notebook;
	GtkWidget *table;
	GtkWidget *vbox;
        gchar *tmpstr;

	notebook = gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
	gtk_box_pack_start(GTK_BOX(tab), notebook, TRUE, TRUE, 0);

	/* Config tab */

	table = gtk_table_new(7, 2, FALSE);

	vbox = gkrellm_gtk_framed_notebook_page(notebook, _("Configuration"));

	gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(_("MPD hostname:")), 0, 1, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(table), (mpc_confwidget_hostname = gtk_entry_new()), 1, 2, 0, 1);
	if (mpc_conf_hostname) {
		gtk_entry_set_text(GTK_ENTRY(mpc_confwidget_hostname), mpc_conf_hostname);
	}

	gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(_("MPD port:")), 0, 1, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(table), (mpc_confwidget_port = gtk_spin_button_new_with_range(1,65535,1)), 1, 2, 1, 2);
	if (mpc_conf_port) {
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(mpc_confwidget_port), mpc_conf_port);
	}

	gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(_("Scrolling speed:")), 0, 1, 2, 3);
	gtk_table_attach_defaults(GTK_TABLE(table), (mpc_confwidget_scrollspeed = gtk_spin_button_new_with_range(1,25,1)), 1, 2, 2, 3);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(mpc_confwidget_scrollspeed), mpc_conf_scrollspeed);

	gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(_("Mouse wheel:")), 0, 1, 3, 4);
	mpc_confwidget_wheelaction = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(mpc_confwidget_wheelaction), _("Adjusts volume"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(mpc_confwidget_wheelaction), _("Adjusts position"));
	gtk_table_attach_defaults(GTK_TABLE(table), mpc_confwidget_wheelaction, 1, 2, 3, 4);
	gtk_combo_box_set_active(GTK_COMBO_BOX(mpc_confwidget_wheelaction), mpc_conf_wheelaction);

	gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(_("Mouse wheel adjustment:")), 0, 1, 4, 5);
	gtk_table_attach_defaults(GTK_TABLE(table), (mpc_confwidget_wheelamount = gtk_spin_button_new_with_range(0,100,1)), 1, 2, 4, 5);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(mpc_confwidget_wheelamount), mpc_conf_wheelamount);

	gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(_("Right click:")), 0, 1, 5, 6);
	mpc_confwidget_rightclick = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(mpc_confwidget_rightclick), _("Opens configuration"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(mpc_confwidget_rightclick), _("Opens playlist"));
	gtk_table_attach_defaults(GTK_TABLE(table), mpc_confwidget_rightclick, 1, 2, 5, 6);
	gtk_combo_box_set_active(GTK_COMBO_BOX(mpc_confwidget_rightclick), mpc_conf_rightclick);

	gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(_("Middle click:")), 0, 1, 6, 7);
	mpc_confwidget_middleclick = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(mpc_confwidget_middleclick), _("Pauses"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(mpc_confwidget_middleclick), _("Simulates drop from clipboard"));
	gtk_table_attach_defaults(GTK_TABLE(table), mpc_confwidget_middleclick, 1, 2, 6, 7);
	gtk_combo_box_set_active(GTK_COMBO_BOX(mpc_confwidget_middleclick), mpc_conf_middleclick);

	gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(_("Drag-drop URL into panel will:")), 0, 1, 7, 8);
	mpc_confwidget_drop = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(mpc_confwidget_drop), _("Clear playlist, add URL and play it"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(mpc_confwidget_drop), _("Add URL and play it"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(mpc_confwidget_drop), _("Just add URL"));
	gtk_table_attach_defaults(GTK_TABLE(table), mpc_confwidget_drop, 1, 2, 7, 8);
	gtk_combo_box_set_active(GTK_COMBO_BOX(mpc_confwidget_drop), mpc_conf_drop);

	gtk_box_pack_start(GTK_BOX(vbox), table, TRUE, TRUE, 0);

	/* Help tab */

	vbox = gkrellm_gtk_framed_notebook_page(notebook, _("Help"));

	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(_("For help please visit the support forums at the MPD web site:\nhttp://www.musicpd.org\n\nAlternatively email the author at webmaster@topfx.com")), TRUE, TRUE, 0);

	/* About tab */

        tmpstr = g_strdup_printf(_("About %s"), PLUGIN_NAME);
	vbox = gkrellm_gtk_framed_notebook_page(notebook, tmpstr);
        g_free(tmpstr);

        tmpstr = g_strdup_printf(_("Plugin version %s\n\nWritten by Mina Naguib <webmaster@topfx.com>\nhttp://www.topfx.com\n\nCopyright (C) 2004-2005 Mina Naguib\nReleased under the GPL license version 2"), VERSION);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(tmpstr), TRUE, TRUE, 0);
        g_free(tmpstr);
}

/*
 * Gets called to apply the plugin config
 */
void mpc_apply_plugin_config () {
	gboolean needdisconnect = FALSE;
	gchar * newhostname;
	gint newport;

	newhostname = g_strdup(gtk_entry_get_text(GTK_ENTRY(mpc_confwidget_hostname)));
	newport = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(mpc_confwidget_port));

	if (strcmp(newhostname, mpc_conf_hostname) != 0 || newport != mpc_conf_port) {
		needdisconnect = TRUE;
	}

	if (mpc_conf_hostname) {
		g_free(mpc_conf_hostname);
	}
	mpc_conf_hostname = newhostname;
	mpc_conf_port = newport;

	if (needdisconnect) {
		if (mpc_addlist)
			gtk_widget_destroy(mpc_addlist);
		mpc_playlistversion = -1;
		mpc_mpd_disconnect();
		mpc_sync_with_mpd();
	}

	mpc_conf_scrollspeed = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(mpc_confwidget_scrollspeed));
	mpc_conf_wheelaction = gtk_combo_box_get_active(GTK_COMBO_BOX(mpc_confwidget_wheelaction));
	mpc_conf_wheelamount = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(mpc_confwidget_wheelamount));
	mpc_conf_rightclick = gtk_combo_box_get_active(GTK_COMBO_BOX(mpc_confwidget_rightclick));
	mpc_conf_middleclick = gtk_combo_box_get_active(GTK_COMBO_BOX(mpc_confwidget_middleclick));
	mpc_conf_drop = gtk_combo_box_get_active(GTK_COMBO_BOX(mpc_confwidget_drop));
}

/*
 * Gets called to save the plugin config to a filehandle
 */
void mpc_save_plugin_config (FILE * fh) {

	fprintf(fh, "%s hostname %s\n", PLUGIN_NAME, mpc_conf_hostname);
	fprintf(fh, "%s port %d\n", PLUGIN_NAME, mpc_conf_port);
	fprintf(fh, "%s scrollspeed %d\n", PLUGIN_NAME, mpc_conf_scrollspeed);
	fprintf(fh, "%s wheelaction %d\n", PLUGIN_NAME, mpc_conf_wheelaction);
	fprintf(fh, "%s wheelamount %d\n", PLUGIN_NAME, mpc_conf_wheelamount);
	fprintf(fh, "%s rightclick %d\n", PLUGIN_NAME, mpc_conf_rightclick);
	fprintf(fh, "%s middleclick %d\n", PLUGIN_NAME, mpc_conf_middleclick);
	fprintf(fh, "%s drop %d\n", PLUGIN_NAME, mpc_conf_drop);
}

/*
 * Gets called to load a single config value from a line
 */
void mpc_load_plugin_config (gchar * line) {
	gchar * keyword;
	gchar * value;
	gint len;

	len = strlen(line);
	keyword = g_malloc(len+1);
	value = g_malloc(len+1);

	if (sscanf(line, "%31s %[^\n]", keyword, value) == 2) {
		if (strcmp(keyword, "hostname") == 0) {
			if (mpc_conf_hostname) free(mpc_conf_hostname);
			mpc_conf_hostname = g_strdup(value);
		}
		else if (strcmp(keyword, "port") == 0) {
			mpc_conf_port = g_ascii_strtod(value, NULL);
		}
		else if (strcmp(keyword, "scrollspeed") == 0) {
			mpc_conf_scrollspeed = g_ascii_strtod(value, NULL);
		}
		else if (strcmp(keyword, "wheelaction") == 0) {
			mpc_conf_wheelaction = g_ascii_strtod(value, NULL);
		}
		else if (strcmp(keyword, "wheelamount") == 0) {
			mpc_conf_wheelamount = g_ascii_strtod(value, NULL);
		}
		else if (strcmp(keyword, "rightclick") == 0) {
			mpc_conf_rightclick = g_ascii_strtod(value, NULL);
		}
		else if (strcmp(keyword, "middleclick") == 0) {
			mpc_conf_middleclick = g_ascii_strtod(value, NULL);
		}
		else if (strcmp(keyword, "drop") == 0) {
			mpc_conf_drop = g_ascii_strtod(value, NULL);
		}
	}

	free(keyword);
	free(value);
}
