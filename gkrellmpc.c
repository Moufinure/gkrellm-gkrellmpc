
/*
 * $Header: /cvsroot/gkrellmpc/gkrellmpc.c,v 1.64 2005/03/06 02:39:21 mina Exp $
 *
 * Main functions for GkrellMPC as per the gkrellm API doc
 */

#include "globals.h"
#include "gkrellmpc.h"
#include "mpd.h"
#include "conf.h"
#include "playlist.h"
#include "url.h"

#include "pixmaps/prev.xpm"
#include "pixmaps/prev_clicked.xpm"
#include "pixmaps/play_pause.xpm"
#include "pixmaps/play_pause_clicked.xpm"
#include "pixmaps/stop.xpm"
#include "pixmaps/stop_clicked.xpm"
#include "pixmaps/next.xpm"
#include "pixmaps/next_clicked.xpm"
#include "pixmaps/eject.xpm"
#include "pixmaps/eject_clicked.xpm"

#if !GKRELLM_CHECK_VERSION(2,2,0)
#define gkrellm_gdk_string_width    gdk_string_width
#endif

GkrellmMonitor mpc_plugin_mon = {
	PLUGIN_NAME,            /* Name, for config tab.        */
	0,                      /* Id,  0 if a plugin           */
	mpc_create_plugin,      /* The create_plugin() function */
	mpc_update_plugin,      /* The update_plugin() function */
	mpc_create_plugin_tab,  /* The create_plugin_tab() config function */
	mpc_apply_plugin_config,/* The apply_plugin_config() function      */

	mpc_save_plugin_config, /* The save_plugin_config() function  */
	mpc_load_plugin_config, /* The load_plugin_config() function  */
	PLUGIN_NAME,            /* config keyword                     */

	NULL,                   /* Undefined 2  */
	NULL,                   /* Undefined 1  */
	NULL,                   /* Undefined 0  */

	PLUGIN_PLACEMENT,       /* Insert plugin before this monitor.       */
	NULL,                   /* Handle if a plugin, filled in by GKrellM */
	NULL                    /* path if a plugin, filled in by GKrellM   */
};
GkrellmTicks * mpc_ticker = NULL;
gint           mpc_plugin_style_id = 0;
GkrellmDecal * mpc_label_decal = NULL;
gchar        * mpc_label = NULL;
GkrellmDecal * mpc_songname_decal = NULL;
gchar        * mpc_songname = NULL;
gint           mpc_id = -1;
GkrellmKrell * mpc_volume_krell = NULL;
gint           mpc_volume = 0;
gboolean       mpc_volume_inmotion = FALSE;
GkrellmPanel * mpc_panel = NULL;
GkrellmDecal * mpc_status_decal = NULL;
GkrellmKrell * mpc_pos_krell = NULL;
gint           mpc_pos = 0;
gboolean       mpc_pos_inmotion = FALSE;
GtkTooltips  * mpc_tooltip = NULL;

/*
 * Entrance handler gkrellm automatically calls
 */
GkrellmMonitor * gkrellm_init_plugin() {
#ifdef ENABLE_NLS
        bind_textdomain_codeset(PACKAGE, "UTF8");
#endif

	/*
	 * Initialize non-constant globals
	 */
	mpc_ticker = gkrellm_ticks();
	mpc_plugin_style_id = gkrellm_add_chart_style(&mpc_plugin_mon, PLUGIN_STYLE_NAME);
	mpc_label = g_strdup(_("MPD"));
	mpc_songname = g_strdup(_("Not connected"));
	mpc_conf_hostname = g_strdup("localhost");

	return &mpc_plugin_mon;
}

/*
 * Gets called to create the plugin area
 */
void mpc_create_plugin (GtkWidget *vbox, gint first_create) {
	GkrellmStyle         * style;
	GkrellmTextstyle     * textstyle;
	GkrellmStyle         * sliderstyle;
	GtkTargetEntry         dragtargets[3];

	GkrellmDecal * volume_decal = NULL;
	GkrellmDecal * pos_decal = NULL;

	static GkrellmPiximage   *prev_image;
	static GkrellmPiximage   *prev_clicked_image;
	gint   prev_image_width;

	static GkrellmPiximage   *play_pause_image;
	static GkrellmPiximage   *play_pause_clicked_image;
	gint   play_pause_image_width;

	static GkrellmPiximage   *stop_image;
	static GkrellmPiximage   *stop_clicked_image;
	gint   stop_image_width;

	static GkrellmPiximage   *next_image;
	static GkrellmPiximage   *next_clicked_image;
	gint   next_image_width;

	static GkrellmPiximage   *eject_image;
	static GkrellmPiximage   *eject_clicked_image;
	gint   eject_image_width;

	gint t;
	gint l;
	gint w;

	style = gkrellm_meter_style(mpc_plugin_style_id);
	sliderstyle = gkrellm_krell_slider_style();
	textstyle = gkrellm_meter_textstyle(mpc_plugin_style_id);
	w = gkrellm_chart_width();

	if (first_create) {
		/* Create the global panel var */
		mpc_panel = gkrellm_panel_new0();
		
	}

	t = 3;

	/* Create the label decal */
	mpc_label_decal = gkrellm_create_decal_text(mpc_panel, "NO MPD ERROR PAUSED STOPPED 0123456789:", textstyle, style, -1, t, -1);
	gkrellm_draw_decal_text(mpc_panel, mpc_label_decal, mpc_label, -1);

	/* Create the status decal */
	mpc_status_decal = gkrellm_create_decal_pixmap(mpc_panel, gkrellm_decal_misc_pixmap(), gkrellm_decal_misc_mask(), N_MISC_DECALS, style, 0, t);
	mpc_status_decal->x = w - mpc_status_decal->w;
	gkrellm_draw_decal_pixmap(mpc_panel, mpc_status_decal, (mpc_mpd ? D_MISC_LED1 : D_MISC_LED0));

	/* Update t */
	t += mpc_label_decal->h > mpc_status_decal->h ? mpc_label_decal->h : mpc_status_decal->h;
	t += 2;

	/* Create the song name text decal */
	mpc_songname_decal = gkrellm_create_decal_text(mpc_panel, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", textstyle, style, -1, t, -1);
	gkrellm_draw_decal_text(mpc_panel, mpc_songname_decal, mpc_songname, -1);

	/* Update t */
	t += mpc_songname_decal->h;
	t += 2;

	/* Create the vol text decal */
	volume_decal = gkrellm_create_decal_text(mpc_panel, _("Vol:"), textstyle, style, -1, t, 0);
	gkrellm_draw_decal_text(mpc_panel, volume_decal, _("Vol:"), -1);

	/* Create the volume krell */
        gkrellm_set_style_slider_values_default(sliderstyle, t, volume_decal->w + 3, 0);
	mpc_volume_krell = gkrellm_create_krell(mpc_panel, gkrellm_krell_slider_piximage(), sliderstyle);
	gkrellm_monotonic_krell_values(mpc_volume_krell, FALSE);
	gkrellm_set_krell_full_scale(mpc_volume_krell, 100, 1);

	/* Update t */
	t += volume_decal->h > mpc_volume_krell->h_frame ? volume_decal->h : mpc_volume_krell->h_frame;
	t += 2;

	/* Create the pos text decal */
	pos_decal = gkrellm_create_decal_text(mpc_panel, _("Pos:"), textstyle, style, -1, t, 0);
	gkrellm_draw_decal_text(mpc_panel, pos_decal, _("Pos:"), -1);

	/* Create the pos krell */
        gkrellm_set_style_slider_values_default(sliderstyle, t, pos_decal->w + 3, 0);
	mpc_pos_krell = gkrellm_create_krell(mpc_panel, gkrellm_krell_slider_piximage(), sliderstyle);
	gkrellm_monotonic_krell_values(mpc_pos_krell, FALSE);
	gkrellm_set_krell_full_scale(mpc_pos_krell, 100, 1);

	/* Update t */
	t += pos_decal->h > mpc_pos_krell->h_frame ? pos_decal->h : mpc_pos_krell->h_frame;
	t += 2;

	/* Load the buttons into memory  and calculate their widths */
	gkrellm_load_piximage("prev", mpc_xpm_prev0, &prev_image, PLUGIN_STYLE_NAME);
	gkrellm_load_piximage("prev_clicked", mpc_xpm_prev1, &prev_clicked_image, PLUGIN_STYLE_NAME);
	mpc_stack_images(prev_image, prev_clicked_image);
	prev_image_width = gdk_pixbuf_get_width(prev_image->pixbuf);

	gkrellm_load_piximage("play_pause", mpc_xpm_playpause0, &play_pause_image, PLUGIN_STYLE_NAME);
	gkrellm_load_piximage("play_pause_clicked", mpc_xpm_playpause1, &play_pause_clicked_image, PLUGIN_STYLE_NAME);
	mpc_stack_images(play_pause_image, play_pause_clicked_image);
	play_pause_image_width = gdk_pixbuf_get_width(play_pause_image->pixbuf);

	gkrellm_load_piximage("stop", mpc_xpm_stop0, &stop_image, PLUGIN_STYLE_NAME);
	gkrellm_load_piximage("stop_clicked", mpc_xpm_stop1, &stop_clicked_image, PLUGIN_STYLE_NAME);
	mpc_stack_images(stop_image, stop_clicked_image);
	stop_image_width = gdk_pixbuf_get_width(stop_image->pixbuf);

	gkrellm_load_piximage("next", mpc_xpm_next0, &next_image, PLUGIN_STYLE_NAME);
	gkrellm_load_piximage("next_clicked", mpc_xpm_next1, &next_clicked_image, PLUGIN_STYLE_NAME);
	mpc_stack_images(next_image, next_clicked_image);
	next_image_width = gdk_pixbuf_get_width(next_image->pixbuf);

	gkrellm_load_piximage("eject", mpc_xpm_eject0, &eject_image, PLUGIN_STYLE_NAME);
	gkrellm_load_piximage("eject_clicked", mpc_xpm_eject1, &eject_clicked_image, PLUGIN_STYLE_NAME);
	mpc_stack_images(eject_image, eject_clicked_image);
	eject_image_width = gdk_pixbuf_get_width(eject_image->pixbuf);

	/* Place the buttons in the center */
	l = (w - prev_image_width - play_pause_image_width - stop_image_width - next_image_width - (eject_image_width/4) - eject_image_width) / 2;
	if (l < 0) {
		/* gkrellm is not wide enough for our buttons - start at 0 anyways then */
		l = 0;
	}
	
	gkrellm_make_scaled_button(mpc_panel, prev_image, mpc_cb_button_prev, NULL, FALSE, FALSE, 2, 0, 1, l, t, 12, 9);
	l += prev_image_width;

	gkrellm_make_scaled_button(mpc_panel, play_pause_image, mpc_cb_button_play_pause, NULL, FALSE, FALSE, 2, 0, 1, l, t, 12, 9);
	l += play_pause_image_width;

	gkrellm_make_scaled_button(mpc_panel, stop_image, mpc_cb_button_stop, NULL, FALSE, FALSE, 2, 0, 1, l, t, 12, 9);
	l += stop_image_width;

	gkrellm_make_scaled_button(mpc_panel, next_image, mpc_cb_button_next, NULL, FALSE, FALSE, 2, 0, 1, l, t, 12, 9);
	l += next_image_width;

	l += (eject_image_width/4);

	gkrellm_make_scaled_button(mpc_panel, eject_image, mpc_cb_button_eject, NULL, FALSE, FALSE, 2, 0, 1, l, t, 12, 9);
	l += next_image_width;

	/* Create the tooltip */
	mpc_tooltip = gtk_tooltips_new();

	/* Finalize the panel */
	gkrellm_panel_configure(mpc_panel, NULL, style);
	gkrellm_panel_create(vbox, &mpc_plugin_mon, mpc_panel);

	if (first_create) {
		/*
		 * Connect to the panel signals
		 */
		g_signal_connect(G_OBJECT(mpc_panel->drawing_area), "expose_event", G_CALLBACK(mpc_panel_expose_event), NULL);
		g_signal_connect(G_OBJECT(mpc_panel->drawing_area), "button_press_event", G_CALLBACK(mpc_cb_panel_press), NULL);
		g_signal_connect(G_OBJECT(mpc_panel->drawing_area), "button_release_event", G_CALLBACK(mpc_cb_panel_release), NULL);
		g_signal_connect(G_OBJECT(mpc_panel->drawing_area), "motion_notify_event", G_CALLBACK(mpc_cb_panel_motion), NULL);
		g_signal_connect(G_OBJECT(mpc_panel->drawing_area), "scroll_event", G_CALLBACK(mpc_cb_panel_scroll), NULL);
		
		/*
		 * Setup the panel to receive drops for URLS
		 */
		dragtargets[0].target = "text/plain";
		dragtargets[0].flags = 0;
		dragtargets[0].info = 0;
		dragtargets[1].target = "text/uri-list";
		dragtargets[1].flags = 0;
		dragtargets[1].info = 1;
		dragtargets[2].target = "STRING";
		dragtargets[2].flags = 0;
		dragtargets[2].info = 2;
		gtk_drag_dest_set(vbox, GTK_DEST_DEFAULT_DROP, dragtargets, 3, GDK_ACTION_COPY);
		g_signal_connect(G_OBJECT(vbox), "drag_motion", G_CALLBACK(mpc_cb_panel_dragmotion), NULL);
		g_signal_connect(G_OBJECT(vbox), "drag_data_received", G_CALLBACK(mpc_cb_panel_dragdatareceived), NULL);
	}

	/* Tell gkrellm to call us back on destruction */
	gkrellm_disable_plugin_connect(&mpc_plugin_mon, mpc_disable_plugin);

}

/*
 * Gets called to update the plugin area
 */
void mpc_update_plugin () {
	gint		w_scroll, w_decal;
	static gint	x_scroll;

	/* Try to connect to mpd */
	if (!mpc_mpd && mpc_ticker->ten_second_tick) {
		mpc_mpd_connect();
	}

	if (mpc_ticker->second_tick) {
		mpc_sync_with_mpd();
	}

	/* Update the label */
	gkrellm_draw_decal_text(mpc_panel, mpc_label_decal, mpc_label, -1);

	/* Scroll the song name */
	w_decal = mpc_songname_decal->w;
	w_scroll = gkrellm_gdk_string_width(mpc_songname_decal->text_style.font, mpc_songname);
	x_scroll -= mpc_conf_scrollspeed;
	if (x_scroll <= -w_scroll)
		x_scroll = w_decal;
	mpc_songname_decal->x_off = x_scroll;
	gkrellm_draw_decal_text(mpc_panel, mpc_songname_decal, mpc_songname, x_scroll);
	
	/* Update the volume krell */
	gkrellm_update_krell(mpc_panel, mpc_volume_krell, (gint)mpc_volume);

	/* Update the pos krell */
	gkrellm_update_krell(mpc_panel, mpc_pos_krell, (gint)mpc_pos);

	/* Update the panel */
	gkrellm_draw_panel_layers(mpc_panel);

}

/*
 * Gets called when the plugin is disabled
 */
void mpc_disable_plugin () {
	if (mpc_playlist)
		gtk_widget_destroy(mpc_playlist);
	if (mpc_addlist)
		gtk_widget_destroy(mpc_addlist);
	mpc_mpd_disconnect();
	mpc_url_cleanup();
}

/*
 * Gets called back when the PREV button is pressed
 */
void mpc_cb_button_prev() {
	mpc_mpd_do("previous\n");
}

/*
 * Gets called back when the PLAY/PAUSE button is pressed
 */
void mpc_cb_button_play_pause() {
	GHashTable * status;
	gchar * state;

	status = mpc_mpd_get("status\n");

	if (!status) {
		/* Cannot get status - so cannot issue appropriate command - just try to play */
		mpc_mpd_do("play\n");
	}
	else {
		state = g_hash_table_lookup(status, "state");
		if (strcmp(state, "play") == 0) {
			/* MPD is playing - issue pause command */
			mpc_mpd_do("pause\n");
		}
		else {
			/* MPD is not playing - issue play command */
			mpc_mpd_do("play\n");
		}
		g_hash_table_destroy(status);
	}

}

/*
 * Gets called back when the STOP button is pressed
 */
void mpc_cb_button_stop() {
	mpc_mpd_do("stop\n");
}

/*
 * Gets called back when the NEXT button is pressed
 */
void mpc_cb_button_next() {
	mpc_mpd_do("next\n");
}

/*
 * Gets called back when the EJECT button is pressed
 */
void mpc_cb_button_eject() {
	mpc_playlist_create();
}

/*
 * Stolen shamelessly from gkrellmms
 */
void mpc_stack_images (GkrellmPiximage *out_image, GkrellmPiximage *in_image) {
	GdkPixbuf *pixbuf, *src_pixbuf;
	gint w, h;

	src_pixbuf = out_image->pixbuf;
	w = gdk_pixbuf_get_width(src_pixbuf);
	h = gdk_pixbuf_get_height(src_pixbuf);

	pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, gdk_pixbuf_get_has_alpha(src_pixbuf), 8, w, 2 * h);
	gdk_pixbuf_copy_area(src_pixbuf, 0, 0, w, h, pixbuf, 0, 0);
	gdk_pixbuf_copy_area(in_image->pixbuf, 0, 0, w, h, pixbuf, 0, h);
	g_object_unref(out_image->pixbuf);
	out_image->pixbuf = pixbuf;
}

/*
 * Stolen shamelessly from gkrellm's demo2.c
 */
gint mpc_panel_expose_event(GtkWidget *widget, GdkEventExpose *ev) {
	gdk_draw_pixmap(widget->window, widget->style->fg_gc[GTK_WIDGET_STATE (widget)], mpc_panel->pixmap, ev->area.x, ev->area.y, ev->area.x, ev->area.y, ev->area.width, ev->area.height);
	return FALSE;
}

/*
 * Updates the global label var to the one supplied
 * taking care of free()ing etcetera
 */
void mpc_update_label (gchar * newlabel) {
	if (mpc_label) {
		free (mpc_label);
	}
	if (!newlabel) {
		newlabel = "";
	}
	mpc_label = g_strdup(newlabel);
}

/*
 * Updates the global songname var to the one supplied
 * taking care of free()ing etcetera
 */
void mpc_update_songname (gchar * newname) {
	if (mpc_songname) {
		free (mpc_songname);
	}
	if (!newname) {
		newname = "";
	}
	mpc_songname = g_strdup(newname);
}

/*
 * Queries mpd for status/songname
 * updates global vars with the results
 */
void mpc_sync_with_mpd() {
	GHashTable * status;
	GHashTable * currentsong;
	gchar * temp;
	gchar * state;
	gchar * time;
	gchar * artist;
	gchar * title;
	gchar * name;
	gint    oldid;
	gchar * file;
	gchar * newlabel = NULL;
	gchar * newsongname = NULL;
	gint newplaylistversion;
	gchar ** parts;
	gint timesec;
	gchar * tiptext = NULL;

	status = mpc_mpd_get("status\n");
	currentsong = mpc_mpd_get("currentsong\n");

	if (!mpc_mpd) {
		mpc_update_label(_("NO MPD"));
		mpc_update_songname("");
		gtk_tooltips_set_tip(mpc_tooltip, mpc_panel->drawing_area, _("MPD is not running"), NULL);
	}
	else if (!status || !currentsong) {
		mpc_update_label(_("MPD ERROR"));
		mpc_update_songname("");
		mpc_mpd_disconnect();
		gtk_tooltips_set_tip(mpc_tooltip, mpc_panel->drawing_area, _("Error communicating with MPD"), NULL);
	}
	else {

		/* Update global volume */
		if (!mpc_volume_inmotion) mpc_volume = (gint)g_strtod(g_hash_table_lookup(status, "volume"), NULL);

		time = g_hash_table_lookup(status, "time");
		/* Update pos */
		if (!mpc_pos_inmotion) {
			if (time) {
				parts = g_strsplit(time, ":", 2);
				if (g_strtod(parts[1], NULL) == 0)
					mpc_pos = 100;
				else
					mpc_pos = 100 * g_strtod(parts[0], NULL) / g_strtod(parts[1], NULL);
				g_strfreev(parts);
			}
			else {
				mpc_pos = 0;
			}
		}

		/* Update global id */
		oldid = mpc_id;
		temp = g_hash_table_lookup(currentsong, "id");
		if (temp)
			mpc_id = g_strtod(temp, NULL);
		else
			mpc_id = -2;

		if (oldid != mpc_id)
			mpc_playlist_update_bold();

		/* Update global label and songname */

		file = g_hash_table_lookup(currentsong, "file");
		artist = g_hash_table_lookup(currentsong, "artist");
		title = g_hash_table_lookup(currentsong, "title");
		name = g_hash_table_lookup(currentsong, "name");
		state = g_hash_table_lookup(status, "state");

		if (file) {
			tiptext = g_strdup_printf(
				_("Artist: %s\nTitle: %s\nFile: %s")
				, artist ? artist : _("N/A")
				, title ? title : _("N/A")
				,  file
			);
		}
		else {
			tiptext = g_strdup_printf(_("Empty playlist"));
		}
		gtk_tooltips_set_tip(mpc_tooltip, mpc_panel->drawing_area, tiptext, NULL);
		g_free(tiptext);

		if (strcmp(state, "stop") == 0) {
			newlabel = g_strdup(_("MPD STOPPED"));
			newsongname = g_strdup("");
		}
		else {
			if (strcmp(state, "play") == 0) {
				parts = g_strsplit(time, ":", 2);
				timesec = g_strtod(parts[0], NULL);
				g_strfreev(parts);

				newlabel = g_strdup_printf(_("MPD %02d:%02d"), (int)((int)timesec/60), (int)((int)timesec % 60));
			}
			else if (strcmp(state, "pause") == 0) {
				newlabel = g_strdup(_("MPD PAUSED"));
			}

			if (title && *title) {
				if (artist && *artist) {
					newsongname = g_strjoin(": ", artist, title, NULL);
				}
				else if (name && *name) {
					newsongname = g_strjoin(": ", name, title, NULL);
				}
				else {
					newsongname = g_strdup(title);
				}
			}
			else {
				newsongname = g_strdup(file);
			}

		}

		mpc_update_label(newlabel);
		mpc_update_songname(newsongname);

		free(newlabel);
		free(newsongname);

		/* Update global playlistversion */
		newplaylistversion = g_strtod(g_hash_table_lookup(status, "playlist"), NULL);
		if (newplaylistversion != mpc_playlistversion) {
			if (mpc_playlist_update()) {
				mpc_playlistversion = newplaylistversion;
			}
		}

	}

	/*
	 * Cleanup
	 */
	if (status) {
		g_hash_table_destroy(status);
	}
	if (currentsong) {
		g_hash_table_destroy(currentsong);
	}

}

/*
 * Stolen shamelessly from demo4
 */
void mpc_update_volume_position(GkrellmKrell *k, gint x_ev) {
	gint	x;
	gchar * command;
	gint newvolume;

	/* Consider only x_ev values that are inside the dynamic range of the
	|  krell, ie from k->x0 to k->x0 + k->w_scale.  The krell left and right
	|  margins determined this range when the slider krell was created.
	|  I also set the krell full_scale to be w_scale so I can simply update
	|  the krell position to the x position within w_scale.
	*/
	x = x_ev - mpc_volume_krell->x0;
	if (x < 0)
		x = 0;
	if (x > mpc_volume_krell->w_scale)
		x = mpc_volume_krell->w_scale;

	/*
	 * Set newvolume (between 0 and 100)
	 */
	newvolume = ((float)x / mpc_volume_krell->w_scale) * 100;

	/*
	 * Tell mpd
	 */
	command = g_strdup_printf("setvol %d\n", newvolume);
	if (mpc_mpd_do(command)) {
		/*
		 * Set the volume and update the slider
		 */
		mpc_volume = newvolume;
		gkrellm_update_krell(mpc_panel, mpc_volume_krell, mpc_volume);
		gkrellm_draw_panel_layers(mpc_panel);
	}
	g_free(command);

}

void mpc_update_pos_position(GkrellmKrell *k, gint x_ev) {
	gint	x;
	gchar * command;
	gint newpos, seektime;
	GHashTable * status;
	gchar * state;
	gchar * time;
	gchar * song;
	gchar ** parts;

	status = mpc_mpd_get("status\n");
	if (!status)
		return;

	state = g_hash_table_lookup(status, "state");

	if (strcmp(state, "play") != 0) {
		/* Do nothing if we're not currently playing */
	}
	else {

		x = x_ev - mpc_pos_krell->x0;
		if (x < 0)
			x = 0;
		if (x > mpc_pos_krell->w_scale)
			x = mpc_pos_krell->w_scale;

		/*
		 * Set newpos (between 0 and 100)
		 */
		newpos = ((float)x / mpc_pos_krell->w_scale) * 100;

		/*
		 * Tell mpd
		 */
		time = g_hash_table_lookup(status, "time");
		song = g_hash_table_lookup(status, "song");
		parts = g_strsplit(time, ":", 2);
		seektime = g_strtod(parts[1], NULL) * newpos / 100;
		g_strfreev(parts);

		command = g_strdup_printf("seek %s %d\n", song, seektime);
		if (mpc_mpd_do(command)) {
			/*
			 * Set the pos and update the slider
			 */
			mpc_pos = newpos;
			gkrellm_update_krell(mpc_panel, mpc_pos_krell, mpc_pos);
			gkrellm_draw_panel_layers(mpc_panel);
		}
		g_free(command);
	}

	g_hash_table_destroy(status);
}

  /* To make a krell work like a slider, we need to update the krell position
  |  as a function of mouse position.
 * Stolen shamelessly from demo4
  */
gint mpc_cb_panel_motion(GtkWidget *widget, GdkEventMotion *ev, gpointer data) {
	GdkModifierType state;

	if (mpc_volume_inmotion) {
		/* Check if button is still pressed, in case missed button_release
		*/
		state = ev->state;
		if (!(state & GDK_BUTTON1_MASK))
			mpc_volume_inmotion = FALSE;
		else
			mpc_update_volume_position(mpc_volume_krell, (gint) ev->x);
	} else if (mpc_pos_inmotion) {
		/* Check if button is still pressed, in case missed button_release
		*/
		state = ev->state;
		if (!(state & GDK_BUTTON1_MASK))
			mpc_pos_inmotion = FALSE;
		else
			mpc_update_pos_position(mpc_pos_krell, (gint) ev->x);
	}

	return TRUE;
}

gint mpc_cb_panel_release(GtkWidget *widget, GdkEventButton *ev, gpointer data) {
	if (mpc_volume_inmotion)
		mpc_volume_inmotion = FALSE;
	if (mpc_pos_inmotion)
		mpc_pos_inmotion = FALSE;
	return TRUE;
}

gint mpc_cb_panel_press(GtkWidget *widget, GdkEventButton *ev, gpointer data) {
	GkrellmKrell *k;
	gchar *cliptext;
	GtkClipboard * clipboard;

	if (ev->button == 2) {
		if (mpc_conf_middleclick == MPC_MIDDLECLICK_ACTION_DROP) {
			clipboard = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
			cliptext = gtk_clipboard_wait_for_text(clipboard);
			if (cliptext) {
				mpc_url_drop(cliptext);
				g_free(cliptext);
			}
		}
		else {
			mpc_cb_button_play_pause();
		}
		return TRUE;
	}
	else if (ev->button == 3) {
		if (mpc_conf_rightclick == MPC_RIGHTCLICK_ACTION_PLAYLIST)
			mpc_playlist_create();
		else
			gkrellm_open_config_window(&mpc_plugin_mon);
		return TRUE;
	}

    /* Check if button was pressed within the space taken
    |  up by the slider krell.
    */
	mpc_volume_inmotion = FALSE;
	k = mpc_volume_krell;
	if (   ev->x > k->x0 && ev->x <= k->x0 + k->w_scale
	    && ev->y >= k->y0 && ev->y <= k->y0 + k->h_frame
	   )
		mpc_volume_inmotion = TRUE;

	mpc_pos_inmotion = FALSE;
	k = mpc_pos_krell;
	if (   ev->x > k->x0 && ev->x <= k->x0 + k->w_scale
	    && ev->y >= k->y0 && ev->y <= k->y0 + k->h_frame
	   )
		mpc_pos_inmotion = TRUE;

	if (mpc_volume_inmotion)
		mpc_update_volume_position(mpc_volume_krell, (gint) ev->x);

	if (mpc_pos_inmotion)
		mpc_update_pos_position(mpc_pos_krell, (gint) ev->x);

	return TRUE;
}

gint mpc_cb_panel_scroll(GtkWidget *widget, GdkEventScroll *ev, gpointer data) {
	gint newvolume = 0;
	gint newposition = 0;
	gchar * time;
	gchar * song;
	GHashTable * status;
	gchar ** parts;
	gchar * command;

	switch (ev->direction) {
		case GDK_SCROLL_UP:
		case GDK_SCROLL_RIGHT:
			if (mpc_conf_wheelaction == MPC_WHEEL_ACTION_VOLUME)
				newvolume = mpc_volume + mpc_conf_wheelamount;
			else
				newposition = mpc_pos + mpc_conf_wheelamount;
			break;
		case GDK_SCROLL_DOWN:
		case GDK_SCROLL_LEFT:
			if (mpc_conf_wheelaction == MPC_WHEEL_ACTION_VOLUME)
				newvolume = mpc_volume - mpc_conf_wheelamount;
			else
				newposition = mpc_pos - mpc_conf_wheelamount;
			break;
	}

	if (newvolume < 0)
		newvolume = 0;
	if (newvolume > 100)
		newvolume = 100;
	if (newposition < 0)
		newposition = 0;
	if (newposition > 100)
		newposition = 100;

	if (mpc_conf_wheelaction == MPC_WHEEL_ACTION_VOLUME && newvolume != mpc_volume) {
		/* Volume was changed */
		command = g_strdup_printf("setvol %d\n", newvolume);
		if (mpc_mpd_do(command)) {
			/* MPD took it - set volume and update panel */
			mpc_volume = newvolume;
			gkrellm_update_krell(mpc_panel, mpc_volume_krell, mpc_volume);
			gkrellm_draw_panel_layers(mpc_panel);
		}
		g_free(command);
	}
	else if (mpc_conf_wheelaction == MPC_WHEEL_ACTION_POSITION && newposition != mpc_pos) {
		/* Position was changed */

		/* We need some stats from current song to prepare command */
		status = mpc_mpd_get("status\n");
		if (!status)
			return TRUE;

		time = g_hash_table_lookup(status, "time");
		song = g_hash_table_lookup(status, "song");
		if (time && song) {
			parts = g_strsplit(time, ":", 2);
			command = g_strdup_printf("seek %s %d\n", song, (int)(g_strtod(parts[1], NULL) * newposition / 100));
			g_strfreev(parts);

			if (mpc_mpd_do(command)) {
				/* MPD took it - set position and update panel */
				mpc_pos = newposition;
				gkrellm_update_krell(mpc_panel, mpc_pos_krell, mpc_pos);
				gkrellm_draw_panel_layers(mpc_panel);
			}
			g_free(command);
		}

		g_hash_table_destroy(status);
	}

	return TRUE;
}

/*
 * Gets called back when a drop happens on the panel
 */
void mpc_cb_panel_dragdatareceived (GtkWidget *widget, GdkDragContext *dc, gint x, gint y, GtkSelectionData *data, guint info, guint t, gpointer userdata) {

	if (data->length) {
		mpc_url_drop(data->data);
	}

	gtk_drag_finish(dc, TRUE, FALSE, t);
}

/*
 * Gets called when the drag motion occurs over the panel
 */
gboolean mpc_cb_panel_dragmotion (GtkWidget *widget, GdkDragContext *dc, gint x, gint y, guint t, gpointer data) {

	/*
	 * Anywhere in the panel is a viable drop area - don't bother checking coordinates or such
	 */
	gdk_drag_status(dc, GDK_ACTION_COPY, t);

	return(TRUE);
}
