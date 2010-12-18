/*
 * $Header: /cvsroot/gkrellmpc/url.c,v 1.3 2005/01/03 21:47:27 mina Exp $
 *
 * Holds all the functions for url manpilation in in gkrellmpc
 */

#include "globals.h"
#include "url.h"
#include "mpd.h"
#include <curl/curl.h>

CURL * mpc_curl = NULL;
gchar * mpc_url_contenttype = NULL;
gchar * mpc_url_content = NULL;

/*
 * Call once to initialize libcurl
 */
void mpc_url_init (void) {
	mpc_url_cleanup();
	mpc_curl = curl_easy_init();
}

/*
 * Call once to de-initialize libcurl
 */
void mpc_url_cleanup (void) {
	if (mpc_curl) {
		curl_easy_cleanup(mpc_curl);
		mpc_curl = NULL;
	}
}

/*
 * Give it a URL
 * It'll reply back with (a possibly different) URL suitable for adding to MPD
 * or NULL if your original URL was garbage
 * returned URL should be freed
 */
gchar * mpc_url_parse (const gchar * url) {
	gchar * retval = NULL;
	gchar ** lines = NULL;
	gchar ** templines = NULL;
	gchar ** parts = NULL;
	double bytesin = 0;
	mpc_url_init();

	/* Prepare globals */
	if (mpc_url_contenttype) {
		g_free(mpc_url_contenttype);
		mpc_url_contenttype = NULL;
	}
	if (mpc_url_content) {
		g_free(mpc_url_content);
		mpc_url_content = NULL;
	}

	/* Fire up curl */
	curl_easy_setopt(mpc_curl, CURLOPT_URL, url);
	curl_easy_setopt(mpc_curl, CURLOPT_HEADERFUNCTION, mpc_url_header);
	curl_easy_setopt(mpc_curl, CURLOPT_WRITEFUNCTION, mpc_url_data);
	curl_easy_perform(mpc_curl);
	curl_easy_getinfo(mpc_curl, CURLINFO_SIZE_DOWNLOAD, &bytesin);

	if (!mpc_url_contenttype) {
		/* Cannot determine the content type */
		if (bytesin > 0) {
			/* But we connected ! It looks like an icecase server */
			retval = g_strdup(url);
		}
	}
	else if (strcmp(mpc_url_contenttype, "audio/x-scpls") == 0) {
		/*
		 * We have a shoutcast playlist
		 *
		 * Parse it looking for "fileX"
		 */
		lines = g_strsplit_set(mpc_url_content, "\r\n", 0);
		if (lines) {
			templines = lines;
			while (templines[0] && !retval) {
				parts = g_strsplit(templines[0], "=", 2);
				if (parts) {
					if (parts[0] && parts[1]) {
						if (g_strncasecmp(parts[0], "file", 4) == 0 && strlen(parts[0]) >= 5 && g_ascii_isdigit(parts[0][4])) {
							/* We found "fileX" - it's value is what we want */
							retval = mpc_url_parse(parts[1]);
						}
					}
					g_strfreev(parts);
				}
				templines++;
			}
			g_strfreev(lines);
		}
	}

	if (mpc_url_contenttype) {
		g_free(mpc_url_contenttype);
		mpc_url_contenttype = NULL;
	}
	if (mpc_url_content) {
		g_free(mpc_url_content);
		mpc_url_content= NULL;
	}

	return (retval);
}

/*
 * Gets called by curl through mpc_url_parse
 * It's job is to set mpc_url_contenttype when it matches the content-type header
 */
size_t mpc_url_header(void *ptr, size_t size, size_t nmemb, void *stream) {
	gint len;
	gchar ** parts = NULL;
	gchar ** parts2 = NULL;

	parts = g_strsplit(ptr, ": ", 2);
	if (!parts)
		parts = g_strsplit(ptr, ":", 2);
	if (parts) {
		if (parts[0] && parts[1]) {
			if (g_strcasecmp(parts[0], "content-type") == 0) {
				parts2 = g_strsplit_set(parts[1], "; \n\r", 0);
				if (parts2) {
					if (parts2[0]) {
						/* We found the content-type */
						if (mpc_url_contenttype)
							g_free(mpc_url_contenttype);
						mpc_url_contenttype = g_strdup(parts2[0]);
					}
					g_strfreev(parts2);
				}
			}
		}
		g_strfreev(parts);
	}

	len = size * nmemb;
	return(len);
}

/*
 * Gets called by curl through mpc_url_parse
 * It's job is to set mpc_url_content
 */
size_t mpc_url_data(void *ptr, size_t size, size_t nmemb, void *stream) {
	gint len;
	gchar * data = NULL;
	gchar * newcontent = NULL;

	len = size * nmemb;

	if (!mpc_url_contenttype
			|| strcmp(mpc_url_contenttype, "audio/x-scpls") != 0
			) {
		/* If the content-type was not caught or is bad, forget about the content */
		return (-1);
	}

	data = g_strndup(ptr, len);
	if (mpc_url_content) {
		newcontent = g_strdup_printf("%s%s", mpc_url_content, data);
		g_free(mpc_url_content);
	}
	else {
		newcontent = g_strdup(data);
	}
	mpc_url_content = newcontent;
	g_free(data);

	return(len);
}

/*
 * Takes a url that got dropped
 * Takes appropriate action on it based on user prefs
 */
void mpc_url_drop (gchar * droppedurl) {
	gchar * newurl = NULL;
	gchar * command = NULL;
	GPtrArray * list;
	GHashTable * hash;
	int i;
	gint id = -1;
	GtkWidget * errordialog;

	if (!droppedurl) {
		// Null url ? Ignore it
	}
	else {
		newurl = mpc_url_parse(droppedurl);
		if (newurl) {

			/*
			 * They dropped a good URL
			 */

			if (mpc_conf_drop == MPC_DROP_ACTION_CLEARADDPLAY) {
				/* Clear the playlist */
				mpc_mpd_do("clear\n");
			}

			/* Add it to the playlist */
			command = g_strdup_printf("add \"%s\"\n", newurl);
			mpc_mpd_do(command);
			g_free(command);

			if (mpc_conf_drop == MPC_DROP_ACTION_CLEARADDPLAY || mpc_conf_drop == MPC_DROP_ACTION_ADDPLAY) {
				/* Play it */
				list = mpc_mpd_get_clumps("playlistinfo\n", FALSE);
				if (list) {
					for (i=0; i < list->len; i++) {
						hash = g_ptr_array_index(list, i);
						id = g_strtod(g_hash_table_lookup(hash, "id"), NULL);
						g_hash_table_destroy(hash);
					}
					g_ptr_array_free(list, FALSE);
				}

				/* Now "id" should have the id of the last entry in the playlist */
				if (id >= 0) {
					command = g_strdup_printf("playid %d\n", id);
					mpc_mpd_do(command);
					g_free(command);
				}
			}

			g_free(newurl);
		}
		else {

			/*
			 * They dropped a bad URL
			 */
			errordialog = gtk_message_dialog_new (NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "URL (%s) could not be added", droppedurl);
			gtk_dialog_run(GTK_DIALOG(errordialog));
			gtk_widget_destroy(errordialog);

		}
	}
}

