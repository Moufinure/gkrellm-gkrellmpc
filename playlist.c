/*
 * $Header: /cvsroot/gkrellmpc/playlist.c,v 1.22 2005/03/06 02:29:07 mina Exp $
 *
 * Holds all the function for playlist manipulation
 */

#include "globals.h"
#include "playlist.h"
#include "mpd.h"
#include "addlist.h"
#include <gdk/gdkkeysyms.h>

GtkWidget    * mpc_playlist = NULL;
gint           mpc_playlist_width = -1;
gint           mpc_playlist_height = 350;
gint           mpc_playlistversion = -1;
GtkListStore * mpc_playlist_store = NULL;
GtkWidget    * mpc_playlist_tree = NULL;
enum mpc_playlist_columns {
	  MPC_PLAYLIST_COLUMN_BOLD = 0
	, MPC_PLAYLIST_COLUMN_ID
	, MPC_PLAYLIST_COLUMN_ARTIST
	, MPC_PLAYLIST_COLUMN_TITLE
	, MPC_PLAYLIST_COLUMNS
};

/*
 * Creates (or activates) the playlist window
 */
void mpc_playlist_create(void) {
	GtkWidget * scrollwindow;
	GtkTreeSelection * selection;
	GtkWidget *vbox;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkWidget * buttonbar;
	GtkWidget * button;


	if (!mpc_playlist) {

		/*
		 * Create the playlist window
		 */
		mpc_playlist = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_wmclass(GTK_WINDOW(mpc_playlist), "playlist", "gkrellmpc");
		gtk_window_set_title(GTK_WINDOW(mpc_playlist), _("MPD PLAYLIST"));
		gtk_container_border_width (GTK_CONTAINER(mpc_playlist), 10);
		gtk_window_set_default_size(GTK_WINDOW(mpc_playlist), mpc_playlist_width, mpc_playlist_height);
		gtk_window_set_position(GTK_WINDOW(mpc_playlist), GTK_WIN_POS_CENTER);
		g_signal_connect(GTK_OBJECT(mpc_playlist), "configure_event", G_CALLBACK(mpc_playlist_configure_event), NULL);
		g_signal_connect(GTK_OBJECT(mpc_playlist), "delete_event", G_CALLBACK(mpc_playlist_delete_event), NULL);
		g_signal_connect(GTK_OBJECT(mpc_playlist), "destroy", G_CALLBACK(mpc_playlist_destroy_event), NULL);
		g_signal_connect(mpc_playlist, "key-release-event", G_CALLBACK(mpc_cb_playlist_key), NULL);

		/*
		 * Create the data store
		 */
		mpc_playlist_store = gtk_list_store_new(MPC_PLAYLIST_COLUMNS, G_TYPE_BOOLEAN, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING);

		/*
		 * Create playlist tree
		 */
		mpc_playlist_tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(mpc_playlist_store));
		gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(mpc_playlist_tree), TRUE);
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(mpc_playlist_tree));
		gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
		g_signal_connect(mpc_playlist_tree, "row-activated", G_CALLBACK(mpc_cb_playlist_row), NULL);
		g_signal_connect(mpc_playlist_tree, "key-release-event", G_CALLBACK(mpc_cb_playlist_key), NULL);

		/*
		 * Create the scrollwindow the playlist goes in
		 */
		scrollwindow = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_container_add (GTK_CONTAINER(scrollwindow), mpc_playlist_tree);

		/*
		 * Create the visual columns
		 */

		renderer = gtk_cell_renderer_text_new ();
		g_object_set(renderer
				, "weight", 700
				, "weight-set", FALSE
				, NULL
		);
		column = gtk_tree_view_column_new_with_attributes (_("Artist"), renderer
				, "weight-set", MPC_PLAYLIST_COLUMN_BOLD
				, "text", MPC_PLAYLIST_COLUMN_ARTIST
				, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(mpc_playlist_tree), column);

		renderer = gtk_cell_renderer_text_new ();
		g_object_set(renderer
				, "weight", 700
				, "weight-set", FALSE
				, NULL
		);
		column = gtk_tree_view_column_new_with_attributes (_("Title"), renderer
				, "weight-set", MPC_PLAYLIST_COLUMN_BOLD
				, "text", MPC_PLAYLIST_COLUMN_TITLE
				, NULL);
		gtk_tree_view_set_search_column(GTK_TREE_VIEW(mpc_playlist_tree), MPC_PLAYLIST_COLUMN_TITLE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(mpc_playlist_tree), column);

		/*
		 * Create the bottom buttons
		 */
		buttonbar = gtk_hbutton_box_new();

		button = gtk_button_new_from_stock(GTK_STOCK_ADD);
		g_signal_connect(GTK_OBJECT(button), "clicked", G_CALLBACK(mpc_cb_playlist_button_add), NULL);
		gtk_container_add(GTK_CONTAINER(buttonbar), button);

		button = gtk_button_new_from_stock(GTK_STOCK_REMOVE);
		g_signal_connect(GTK_OBJECT(button), "clicked", G_CALLBACK(mpc_cb_playlist_button_remove), NULL);
		gtk_container_add(GTK_CONTAINER(buttonbar), button);

		button = gtk_button_new_from_stock(GTK_STOCK_CLEAR);
		g_signal_connect(GTK_OBJECT(button), "clicked", G_CALLBACK(mpc_cb_playlist_button_clear), NULL);
		gtk_container_add(GTK_CONTAINER(buttonbar), button);

		button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
		g_signal_connect(GTK_OBJECT(button), "clicked", G_CALLBACK(mpc_cb_playlist_button_close), NULL);
		gtk_container_add(GTK_CONTAINER(buttonbar), button);


		/*
		 * And pack everything
		 */
		vbox = gtk_vbox_new(FALSE, 8);
		gtk_box_pack_start(GTK_BOX(vbox), scrollwindow, 1, 1, 0);
		gtk_box_pack_start(GTK_BOX(vbox), buttonbar, 0, 0, 0);
		gtk_container_add(GTK_CONTAINER(mpc_playlist), vbox);

		/*
		 * Update the playlist
		 */
		mpc_playlist_update();

		/*
		 * Finally show everything
		 */
		gtk_widget_show_all(mpc_playlist);
	}
	else {

		/*
		 * Make the playlist window the foreground window
		 */
		gtk_window_present(GTK_WINDOW(mpc_playlist));
	}
}

/*
 * Gets called when the window manager tries to close the playlist window
 */
gint mpc_playlist_delete_event(GtkContainer *container, GtkWidget *widget, gpointer user_data) {
	return(FALSE);
}

/*
 * Gets called when the playlist window is about to be destroyed
 */
void mpc_playlist_destroy_event(GtkWidget *widget, gpointer data) {
	/* Blow it up :) */
	gtk_list_store_clear(mpc_playlist_store);
	mpc_playlist = NULL;
	mpc_playlist_store = NULL;
	mpc_playlist_tree = NULL;
	mpc_id = -1;
}

/*
 * If the playlist window exists, updates it with the playlist from mpd
 */
gboolean mpc_playlist_update(void) {
	GPtrArray * list;
	GHashTable * hash;
	GtkTreeIter iter;
	int i;
	gint id;
	gchar * artist;
	gchar * name;
	gchar * title;
	
	if (!mpc_playlist) {
		/*
		 * There's no playlist, so there's nothing to do
		 */
		return(TRUE);
	}
	else {
		/*
		 * Get the playlist from mpd
		 */
		list = mpc_mpd_get_clumps("playlistinfo\n", FALSE);
		if (!list) {
			return (FALSE);
		}

		/*
		 * Populate it
		 */
		gtk_list_store_clear(mpc_playlist_store);
		for (i=0; i < list->len; i++) {

			hash = g_ptr_array_index(list, i);

			/* Add new row to store */
			gtk_list_store_append(mpc_playlist_store, &iter);

			/* Set local vars id, artist, name and title */
			id = g_strtod(g_hash_table_lookup(hash, "id"), NULL);
			artist = g_hash_table_lookup(hash, "artist");
			name = g_hash_table_lookup(hash, "name");
			title = g_hash_table_lookup(hash, "title");
			if (!title) title = g_hash_table_lookup(hash, "file");

			/* Set them in the store */
			gtk_list_store_set(mpc_playlist_store, &iter
					, MPC_PLAYLIST_COLUMN_BOLD, (id == mpc_id ? TRUE : FALSE)
					, MPC_PLAYLIST_COLUMN_ID, id
					, MPC_PLAYLIST_COLUMN_ARTIST, (artist ? artist : name ? name : "")
					, MPC_PLAYLIST_COLUMN_TITLE, title
					, -1);

			g_hash_table_destroy(hash);

		}

		g_ptr_array_free(list, FALSE);

		return (TRUE);
	}
}

/*
 * Called when the "clear" button is clicked in the playlist window
 */
void mpc_cb_playlist_button_clear (GtkButton *button, gpointer user_data) {
	mpc_mpd_do("clear\n");
}

/*
 * Called when the "remove" button is clicked in the playlist window
 */
void mpc_cb_playlist_button_remove (GtkButton *button, gpointer user_data) {
	GtkTreeSelection * selection;
	GArray * selected;
	gint i;
	gchar * command;

	/*
	 * Populate selected with the id of selected rows
	 */
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(mpc_playlist_tree));
	selected = g_array_new(FALSE, TRUE, sizeof(gint));
	gtk_tree_selection_selected_foreach(selection, mpc_cb_delete_foreach, selected);

	for (i=0; i < selected->len; i++) {
		command = g_strdup_printf("deleteid %d\n", g_array_index(selected, gint, i));
		mpc_mpd_do(command);
		g_free(command);
	}
	g_array_free(selected, FALSE);
}

/*
 * Gets called from mpc_cb_playlist_button_remove for each selected row
 * It's job is to add the id of that row's song into the given array
 */
void mpc_cb_delete_foreach (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data) {
	gint id;
	gtk_tree_model_get(GTK_TREE_MODEL(mpc_playlist_store), iter
			, MPC_PLAYLIST_COLUMN_ID, &id
			, -1);
	g_array_append_val(data, id);
}

/*
 * Called when the "add" button is clicked in the playlist window
 */
void mpc_cb_playlist_button_add (GtkButton *button, gpointer user_data) {
	mpc_addlist_create();
}

/*
 * Called when the "close" button is clicked in the playlist window
 */
void mpc_cb_playlist_button_close (GtkButton *button, gpointer user_data) {
	gtk_widget_destroy(mpc_playlist);
}

/*
 * Called when the user double-clicks/hits enter on a playlist row
 */
void mpc_cb_playlist_row (GtkTreeView *tree, GtkTreePath *path, GtkTreeViewColumn *col, gpointer user_data) {
	GtkTreeIter   iter;
	gint id;
	gchar * command;

	if (gtk_tree_model_get_iter(GTK_TREE_MODEL(mpc_playlist_store), &iter, path)) {
		gtk_tree_model_get(GTK_TREE_MODEL(mpc_playlist_store), &iter
				, MPC_PLAYLIST_COLUMN_ID, &id
				, -1);
		command = g_strdup_printf("playid %d\n", id);
		mpc_mpd_do(command);
		g_free(command);
	}
}

/*
 * Called when a user presses a key in the playlist window or tree
 */
gboolean mpc_cb_playlist_key (GtkWidget *widget, GdkEventKey *event, gpointer user_data) {

	if (widget == mpc_playlist) {

		/*
		 * Key pressed in main window
		 */
		switch (event->keyval) {
			case GDK_Escape:
				/* They pressed Escape - destroy the playlist */
				gtk_widget_destroy(mpc_playlist);
				return (TRUE);
		}
	}
	else if (widget == mpc_playlist_tree) {

		/*
		 * Key pressed in tree
		 */
		switch (event->keyval) {
			case GDK_Delete:
			case GDK_KP_Delete:
				/* They pressed delete - simulate a "Remove" button keypress */
				mpc_cb_playlist_button_remove(NULL, NULL);
				return (TRUE);
		}
	}

	/*
	 * If we've reached here tell GTK to send the key upwards
	 */
	return (FALSE);
}

/*
 * If the playlist window is open, it makes "bold" the song ID in mpc_id
 */
void mpc_playlist_update_bold (void) {
	GtkTreeIter iter;
	gint id;

	if (!mpc_playlist) {
		/* No playlist - nothing to do */
	}
	else {
		if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(mpc_playlist_store), &iter)) {
			/* Store is empty */
		}
		else {

			while (1) {

				/* iter points to an entry
				 * get id */
				gtk_tree_model_get(GTK_TREE_MODEL(mpc_playlist_store), &iter
						, MPC_PLAYLIST_COLUMN_ID, &id
						, -1);

				if (id == mpc_id) {
					/* We found the one that should be bold */
					gtk_list_store_set(mpc_playlist_store, &iter, MPC_PLAYLIST_COLUMN_BOLD, TRUE, -1);
				}
				else {
					/* Whatever this is should not be bold */
					gtk_list_store_set(mpc_playlist_store, &iter, MPC_PLAYLIST_COLUMN_BOLD, FALSE, -1);
				}

				if (!gtk_tree_model_iter_next(GTK_TREE_MODEL(mpc_playlist_store), &iter)) {
					/* No more entries in store */
					break;
				}
			}

		}
	}
}

gboolean mpc_playlist_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data) {
	/* Remember width & height */
	mpc_playlist_width = event->width;
	mpc_playlist_height = event->height;
	return(FALSE);
}
