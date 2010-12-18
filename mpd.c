/*
 * $Header: /cvsroot/gkrellmpc/mpd.c,v 1.6 2005/03/05 22:27:27 mina Exp $
 *
 * Holds all the functions needed to talk to MPD
 */

#include "globals.h"
#include "mpd.h"
#include "gkrellmpc.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

GIOChannel   * mpc_mpd = NULL;

/*
 * Connects to the MPD server, sets up the mpd object, sets the status decal to ON
 */
gboolean mpc_mpd_connect() {
	int sockfd;
	struct hostent *server;
	struct sockaddr_in serv_addr;
	gchar * line;
	gchar ** parts;
	gboolean retval;

	if (mpc_mpd) {
		/*
		 * Close existing connection
		 */
		mpc_mpd_disconnect();
	}

	if (!mpc_conf_hostname || !mpc_conf_port) {
		return (FALSE);
	}

	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) return(FALSE);
	if (!(server = gethostbyname(mpc_conf_hostname))) return(FALSE);

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
			(char *)&serv_addr.sin_addr.s_addr,
			server->h_length);
	serv_addr.sin_port = htons(mpc_conf_port);

	if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) return(FALSE);
	
	/* Getup the mpd object */
	mpc_mpd = g_io_channel_unix_new(sockfd);

	/*
	 * Find the OK otherwise we connected to something not-MPDish
	 */
	if (g_io_channel_read_line(mpc_mpd, &line, NULL, NULL, NULL) == G_IO_STATUS_NORMAL) {
		g_strchomp(line);
		parts = g_strsplit(line, " ", 2);
		if (strcmp(parts[0], "OK") == 0) {
			retval = TRUE;
		}
		else {
			mpc_mpd_disconnect();
			retval = FALSE;
		}
		g_strfreev(parts);
	}
	else {
		/* Read failed */
		mpc_mpd_disconnect();
		retval = FALSE;
	}

	if (retval) {
		gkrellm_draw_decal_pixmap(mpc_panel, mpc_status_decal, D_MISC_LED1);
		mpc_update_label(_("MPD"));
		mpc_update_songname("");
	}

	return(retval);
}

/*
 * Disconnects from MPD, destroys the mpd object, sets the status decal to off
 */
gboolean mpc_mpd_disconnect() {

	if (mpc_mpd) {
		g_io_channel_shutdown(mpc_mpd, FALSE, NULL);
		free(mpc_mpd);
		mpc_mpd = NULL;
	}
	
	gkrellm_draw_decal_pixmap(mpc_panel, mpc_status_decal, D_MISC_LED0);
	mpc_update_label(_("NO MPD"));
	mpc_update_songname("");
	return (TRUE);
}

/*
 * Sends a command to MPD
 * Returns true if successful, false if not
 * Use for bool-returning commands. Use mpd_get instead for data-returning commands
 */
gboolean mpc_mpd_do(gchar * command) {
	gchar *line;
	gboolean retval;

	if (!mpc_mpd && !mpc_mpd_connect()) {
		return (FALSE);
	}

	if (g_io_channel_write_chars(mpc_mpd, command, -1, NULL, NULL) == G_IO_STATUS_NORMAL) {
		g_io_channel_flush(mpc_mpd, NULL);
		if (g_io_channel_read_line(mpc_mpd, &line, NULL, NULL, NULL) == G_IO_STATUS_NORMAL) {
			g_strchomp(line);
			if (strcmp(line, "OK") == 0) {
				/* It's a success */
				retval = TRUE;
			}
			else {
				/* It's a failure */
				retval = FALSE;
			}
		}
		else {
			/* Read failed */
			retval = FALSE;
		}
	}
	else {
		/* Write Failed */
		retval = FALSE;
	}

	if (retval)
		mpc_sync_with_mpd();

	return (retval);
}

/*
 * Sends a command to MPD
 * Returns a hashref of the results (needs to be freed with g_hash_table_destroy  afterwards), NULL on error
 * Use for keypair-returning commands. Use mpd_do instead for boolean-returning commands
 * Note that all key names coming back from MPD are lowercased for consistency before being shoved in the hash
 */
GHashTable * mpc_mpd_get(gchar * command) {
	gchar *line;
	gchar **parts;
	GHashTable *retval = NULL;
	gboolean success = TRUE;
	GIOStatus status;

	if (!mpc_mpd && !mpc_mpd_connect()) {
		return (NULL);
	}

	if (g_io_channel_write_chars(mpc_mpd, command, -1, NULL, NULL) == G_IO_STATUS_NORMAL) {
		g_io_channel_flush(mpc_mpd, NULL);

		retval = g_hash_table_new_full(g_str_hash, g_str_equal, free, free);

		while (1) {
			status = g_io_channel_read_line(mpc_mpd, &line, NULL, NULL, NULL);
			if (status != G_IO_STATUS_NORMAL) {
				/* Reading failed */
				success = FALSE;
				break;
			}
			g_strchomp(line);
			if (strcmp(line, "OK") == 0) {
				/* End of resultset with success */
				break;
			}
			else if (g_str_has_prefix(line, "ACK ")) {
				/* End of resultset with failure */
				success = FALSE;
				break;
			}
			else {
				/* A resultset entry */
				parts = g_strsplit(line, ": ", 2);
				if (!parts || !parts[0] || !parts[1]) {
					/* Resultset entry is not proper - ignore it */
				}
				else {
					/* nice pair - add to hash */
					g_hash_table_insert(retval, g_ascii_strdown(parts[0], -1), g_strdup(parts[1]));
				}
				g_strfreev(parts);
			}
		}

		if (!success) {
			g_hash_table_destroy(retval);
			retval = NULL;
		}

	}
	else {
		/* Write Failed */
		retval = NULL;
	}

	/* If we've reached here, all is well */
	return (retval);
}


/*
 * Sends a command to MPD
 * Returns an arrayref of hashrefs of the results, NULL on error
 * Use for keypair-returning commands that require clumping
 * Each hashref must be freed using g_hash_table_destroy, then the whole array must be freed with g_ptr_array_free
 * Note that all key names coming back from MPD are lowercased for consistency before being shoved in the hashes
 */
GPtrArray * mpc_mpd_get_clumps(gchar * command, gboolean clumpsingles) {
	gchar *line;
	gchar **parts;
	GPtrArray *retval = NULL;
	gboolean success = TRUE;
	GIOStatus status;
	int i;

	if (!mpc_mpd && !mpc_mpd_connect()) {
		return (NULL);
	}

	if (g_io_channel_write_chars(mpc_mpd, command, -1, NULL, NULL) == G_IO_STATUS_NORMAL) {
		g_io_channel_flush(mpc_mpd, NULL);

		retval = g_ptr_array_new();

		while (1) {
			status = g_io_channel_read_line(mpc_mpd, &line, NULL, NULL, NULL);
			if (status != G_IO_STATUS_NORMAL) {
				/* Reading failed */
				success = FALSE;
				break;
			}
			g_strchomp(line);
			if (strcmp(line, "OK") == 0) {
				/* End of resultset with success */
				break;
			}
			else if (g_str_has_prefix(line, "ACK ")) {
				/* End of resultset with failure */
				success = FALSE;
				break;
			}
			else {
				/* A resultset entry */
				parts = g_strsplit(line, ": ", 2);
				if (!parts || !parts[0] || !parts[1]) {
					/* Resultset entry is not proper - ignore it */
				}
				else {
					/* nice pair */
					if (clumpsingles || retval->len == 0 || g_hash_table_lookup_extended(g_ptr_array_index(retval, retval->len -1), g_ascii_strdown(parts[0], -1), NULL, NULL)) {
						/*
						 * Create a new hash and add it at the end of the array
						 * Either because the array is empty, or the last hash already has this key so we need a new clump
						 */
						g_ptr_array_add(retval, g_hash_table_new_full(g_str_hash, g_str_equal, free, free));
					}

					/* Add the key to the last hashref in the array */
					g_hash_table_insert(g_ptr_array_index(retval, retval->len -1), g_ascii_strdown(parts[0], -1), g_strdup(parts[1]));
				}
				g_strfreev(parts);
			}
		}

		if (!success) {
			for (i=0; i < retval->len; i++) {
				g_hash_table_destroy(g_ptr_array_index(retval, i));
			}
			g_ptr_array_free(retval, FALSE);
			retval = NULL;
		}

	}
	else {
		/* Write Failed */
		retval = NULL;
	}

	/* If we've reached here, all is well */
	return (retval);
}

