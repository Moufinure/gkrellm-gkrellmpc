/*
 * $Header: /cvsroot/gkrellmpc/addlist.c,v 1.12 2005/03/06 02:29:07 mina Exp $
 *
 * Holds all the function for addlist manipulation
 */

#include "globals.h"
#include "addlist.h"
#include "mpd.h"
#include "url.h"
#include <gdk/gdkkeysyms.h>

GtkWidget    * mpc_addlist = NULL;
gint           mpc_addlist_width = -1;
gint           mpc_addlist_height = 400;
GtkWidget    * mpc_addlist_url;
GtkTreeStore * mpc_addlist_store = NULL;
GtkWidget    * mpc_addlist_tree = NULL;
enum mpc_addlist_columns {
	MPC_ADDLIST_COLUMN_ICON = 0
	, MPC_ADDLIST_COLUMN_RAWNAME
	, MPC_ADDLIST_COLUMN_DISPLAYNAME
	, MPC_ADDLIST_COLUMNS
};

/*
 * Creates (or activates) the addlist window
 */
void mpc_addlist_create(void) {
	GtkWidget * scrollwindow;
	GtkTreeSelection * selection;
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkWidget * buttonbar;
	GtkWidget * button;
	GtkWidget * bar;
	GtkWidget * url_label;
	GtkWidget * url_add;


	if (!mpc_addlist) {

		/*
		 * Create the addlist window
		 */
		mpc_addlist = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_wmclass(GTK_WINDOW(mpc_addlist), "playlistadd", "gkrellmpc");
		gtk_window_set_title(GTK_WINDOW(mpc_addlist), _("MPD ADD SONGS"));
		gtk_container_border_width (GTK_CONTAINER(mpc_addlist), 10);
		gtk_window_set_default_size(GTK_WINDOW(mpc_addlist), mpc_addlist_width, mpc_addlist_height);
		gtk_window_set_position(GTK_WINDOW(mpc_addlist), GTK_WIN_POS_CENTER);
		g_signal_connect(GTK_OBJECT(mpc_addlist), "configure_event", G_CALLBACK(mpc_addlist_configure_event), NULL);
		g_signal_connect(GTK_OBJECT(mpc_addlist), "delete_event", G_CALLBACK(mpc_addlist_delete_event), NULL);
		g_signal_connect(GTK_OBJECT(mpc_addlist), "destroy", G_CALLBACK(mpc_addlist_destroy_event), NULL);
		g_signal_connect(mpc_addlist, "key-release-event", G_CALLBACK(mpc_cb_addlist_key), NULL);

		/*
		 * Create URL label
		 */
		url_label = gtk_label_new("URL:");

		/*
		 * Create the URL entry box
		 */
		mpc_addlist_url = gtk_entry_new();

		/*
		 * Create the URL add button
		 */
		url_add = gtk_button_new_from_stock(GTK_STOCK_ADD);
		g_signal_connect(GTK_OBJECT(url_add), "clicked", G_CALLBACK(mpc_cb_addlist_button_add), "url");

		/*
		 * Create the bar
		 */
		bar = gtk_hseparator_new();

		/*
		 * Create the data store
		 */
		mpc_addlist_store = gtk_tree_store_new(MPC_ADDLIST_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

		/*
		 * Create addlist tree
		 */
		mpc_addlist_tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(mpc_addlist_store));
		gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(mpc_addlist_tree), TRUE);
		// gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(mpc_addlist_tree), FALSE);
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(mpc_addlist_tree));
		gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
		g_signal_connect(mpc_addlist_tree, "row-activated", G_CALLBACK(mpc_cb_addlist_row), NULL);

		/*
		 * Create the scrollwindow the addlist goes in
		 */
		scrollwindow = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_container_add (GTK_CONTAINER(scrollwindow), mpc_addlist_tree);

		/*
		 * Create the visual columns
		 */
		renderer = gtk_cell_renderer_pixbuf_new ();
		column = gtk_tree_view_column_new_with_attributes (NULL, renderer
				, "stock-id", MPC_ADDLIST_COLUMN_ICON
				, NULL);
		gtk_tree_view_column_set_sort_column_id (column, MPC_ADDLIST_COLUMN_ICON);
		gtk_tree_view_append_column(GTK_TREE_VIEW(mpc_addlist_tree), column);

		renderer = gtk_cell_renderer_text_new ();
		column = gtk_tree_view_column_new_with_attributes (_("Name"), renderer
				, "text", MPC_ADDLIST_COLUMN_DISPLAYNAME
				, NULL);
		gtk_tree_view_column_set_sort_column_id (column, MPC_ADDLIST_COLUMN_DISPLAYNAME);
		gtk_tree_view_set_search_column(GTK_TREE_VIEW(mpc_addlist_tree), MPC_ADDLIST_COLUMN_DISPLAYNAME);
		gtk_tree_view_append_column(GTK_TREE_VIEW(mpc_addlist_tree), column);

		/*
		 * Create the bottom buttons
		 */
		buttonbar = gtk_hbutton_box_new();

		button = gtk_button_new_from_stock(GTK_STOCK_ADD);
		g_signal_connect(GTK_OBJECT(button), "clicked", G_CALLBACK(mpc_cb_addlist_button_add), "list");
		gtk_container_add(GTK_CONTAINER(buttonbar), button);

		button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
		g_signal_connect(GTK_OBJECT(button), "clicked", G_CALLBACK(mpc_cb_addlist_button_close), NULL);
		gtk_container_add(GTK_CONTAINER(buttonbar), button);


		/*
		 * And pack everything
		 */
		hbox = gtk_hbox_new(FALSE, 8);
		gtk_box_pack_start(GTK_BOX(hbox), url_label, 0, 0, 0);
		gtk_box_pack_start(GTK_BOX(hbox), mpc_addlist_url, 1, 1, 0);
		gtk_box_pack_start(GTK_BOX(hbox), url_add, 0, 0, 0);
		vbox = gtk_vbox_new(FALSE, 8);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, 0, 0, 0);
		gtk_box_pack_start(GTK_BOX(vbox), bar, 0, 0, 0);
		gtk_box_pack_start(GTK_BOX(vbox), scrollwindow, 1, 1, 0);
		gtk_box_pack_start(GTK_BOX(vbox), buttonbar, 0, 0, 0);
		gtk_container_add(GTK_CONTAINER(mpc_addlist), vbox);

		/*
		 * Update the addlist
		 */
		mpc_addlist_update();

		/*
		 * Finally show everything
		 */
		gtk_widget_show_all(mpc_addlist);
	}
	else {

		/*
		 * Make the addlist window the foreground window
		 */
		gtk_window_present(GTK_WINDOW(mpc_addlist));
	}
}

/*
 * Called when a user presses a key in the addlist window
 */
gboolean mpc_cb_addlist_key (GtkWidget *widget, GdkEventKey *event, gpointer user_data) {

	switch (event->keyval) {
		case GDK_Escape:
			/* They pressed Escape - destroy the addlist */
			gtk_widget_destroy(mpc_addlist);
			return (TRUE);
	}

	/*
	 * If we've reached here tell GTK to send the key upwards
	 */
	return (FALSE);
}

/*
 * Gets called when the addlist window is about to be destroyed
 */
void mpc_addlist_destroy_event(GtkWidget *widget, gpointer data) {
	/* Blow it up :) */
	gtk_tree_store_clear(mpc_addlist_store);
	mpc_addlist = NULL;
	mpc_addlist_store = NULL;
	mpc_addlist_tree = NULL;
}

/*
 * Gets called when the window manager tries to close the addlist window
 */
gint mpc_addlist_delete_event(GtkContainer *container, GtkWidget *widget, gpointer user_data) {
	return(FALSE);
}

/*
 * Called when the user double-clicks/hits enter on a addlist row
 */
void mpc_cb_addlist_row (GtkTreeView *tree, GtkTreePath *path, GtkTreeViewColumn *col, gpointer user_data) {
	GtkTreeIter   iter;
	gchar * name;
	gchar * command;

	if (gtk_tree_model_get_iter(GTK_TREE_MODEL(mpc_addlist_store), &iter, path)) {
		gtk_tree_model_get(GTK_TREE_MODEL(mpc_addlist_store), &iter
				, MPC_ADDLIST_COLUMN_RAWNAME, &name
				, -1);
		command = g_strdup_printf("add \"%s\"\n", name);
		mpc_mpd_do(command);
		g_free(command);
	}
}

/*
 * Called when the "add" button is clicked in the addlist window
 */
void mpc_cb_addlist_button_add (GtkButton *button, gpointer user_data) {
	GtkTreeSelection * selection;
	const gchar * oldurl = NULL;
	gchar * newurl = NULL;
	gchar * command = NULL;
	GtkWidget * errordialog;

	if (strcmp(user_data, "url") == 0) {
		/*
		 * They clicked on the ADD next to the url
		 */
		oldurl = gtk_entry_get_text(GTK_ENTRY(mpc_addlist_url)); 
		if (oldurl && strlen(oldurl) > 0) {
			newurl = mpc_url_parse(oldurl);
			if (newurl) {
				command = g_strdup_printf("add \"%s\"\n", newurl);
				mpc_mpd_do(command);
				g_free(command);

				g_free(newurl);
			}
			else {
				/*
				 * Show them an error
				 */
				errordialog = gtk_message_dialog_new (GTK_WINDOW(mpc_addlist), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("URL could not be added"));
				gtk_dialog_run(GTK_DIALOG(errordialog));
				gtk_widget_destroy(errordialog);
			}
		}
	}
	else {
		/*
		 * They clicked on ADD under the list
		 * loop over selected rows calling mpc_cb_add_foreach for each
		 */
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(mpc_addlist_tree));
		gtk_tree_selection_selected_foreach(selection, mpc_cb_add_foreach, NULL);
	}

}

/*
 * Gets called from mpc_cb_addlist_button_add for each selected row
 * It's job is to send the "add" command to mpd for the given row
 */
void mpc_cb_add_foreach (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data) {
	gchar * rawname;
	gchar * command;

	gtk_tree_model_get(GTK_TREE_MODEL(mpc_addlist_store), iter
			, MPC_ADDLIST_COLUMN_RAWNAME, &rawname
			, -1);

	command = g_strdup_printf("add \"%s\"\n", rawname);
	mpc_mpd_do(command);
	g_free(command);

}


/*
 * Called when the "close" button is clicked in the addlist window
 */
void mpc_cb_addlist_button_close (GtkButton *button, gpointer user_data) {
	gtk_widget_destroy(mpc_addlist);
}

/*
 * If the addlist window exists, updates it with the addlist from mpd
 */
gboolean mpc_addlist_update(void) {
	GPtrArray * list;
	GHashTable * hash;
	gchar * file;
	gchar * directory;
	gint i;
	GtkTreeIter iter;
	GPtrArray * iterpath;
	GtkTreeIter * tempiter;
	GtkTreeIter * parentiter = NULL;
	gint level = 0;
	gchar * p;
	gchar ** parts;
	gchar ** tempparts;
	gchar * nicename = NULL;

	if (!mpc_addlist) {
		/* Nothing to do */
	}
	else {

		/* Get list of all files form mpd */
		list = mpc_mpd_get_clumps("listall\n", TRUE);
		if (!list) {
			return (FALSE);
		}

		gtk_tree_store_clear(mpc_addlist_store);
		iterpath = g_ptr_array_new();

		for (i=0; i < list->len; i++) {
			hash = g_ptr_array_index(list, i);

			/* Populate local vars file and directory */
			file = g_hash_table_lookup(hash, "file");
			directory = g_hash_table_lookup(hash, "directory");

			/* Update parentiter */
			if (directory) {

				level = 1;
				p = directory;
				while (*(p++)) {
					if (*p == '/') {
						level++;
					}
				}

				while (iterpath->len >= level) {
					tempiter = g_ptr_array_index(iterpath, iterpath->len - 1);
					g_free(tempiter);
					g_ptr_array_remove_index(iterpath, iterpath->len - 1);
				}

				if (iterpath->len)
					parentiter = g_ptr_array_index(iterpath, iterpath->len - 1);
				else
					parentiter = NULL;
			}

			/* Set nicename */
			parts = g_strsplit((directory ? directory : file), "/", 0);
			tempparts = parts;
			while (tempparts[0]) {
				nicename = tempparts[0];
				tempparts++;
			}

			/* Add new row to store */
			gtk_tree_store_append(mpc_addlist_store, &iter, parentiter);

			/* Set vars in new row */
			gtk_tree_store_set(mpc_addlist_store, &iter
					, MPC_ADDLIST_COLUMN_ICON, (directory ? GTK_STOCK_OPEN : GTK_STOCK_NEW)
					, MPC_ADDLIST_COLUMN_RAWNAME, (directory ? directory : file)
					, MPC_ADDLIST_COLUMN_DISPLAYNAME, nicename
					, -1);

			g_strfreev(parts);

			/* Update parentiter */
			if (directory) {
				tempiter = g_malloc(sizeof(iter));
				memcpy(tempiter, &iter, sizeof(iter));
				g_ptr_array_add(iterpath, tempiter);
				parentiter = tempiter;
			}

			g_hash_table_destroy(hash);
		}

		g_ptr_array_free(iterpath, TRUE);
		g_ptr_array_free(list, FALSE);

	}

	return (TRUE);
}

gboolean mpc_addlist_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data) {
	/* Remember width & height */
	mpc_addlist_width = event->width;
	mpc_addlist_height = event->height;
	return(FALSE);
}
