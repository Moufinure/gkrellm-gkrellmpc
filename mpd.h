/*
 * $Header: /cvsroot/gkrellmpc/mpd.h,v 1.2 2005/01/01 19:19:49 mina Exp $
 */

#ifndef _MPD_H
#define _MPD_H

gboolean     mpc_mpd_connect();
gboolean     mpc_mpd_disconnect();
gboolean     mpc_mpd_do(gchar *);
GHashTable * mpc_mpd_get(gchar *);
GPtrArray  * mpc_mpd_get_clumps(gchar *, gboolean);

#endif
