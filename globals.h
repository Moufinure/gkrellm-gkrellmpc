
/*
 * $Header: /cvsroot/gkrellmpc/globals.h,v 1.15 2005/03/04 18:06:58 mina Exp $
 *
 * Holds all the global vars used by GkrellMPC
 */

#ifndef _GLOBALS_H
#define _GLOBALS_H

#define VERSION "0.1_beta10"

#define PLUGIN_PLACEMENT	MON_MAIL
#define PLUGIN_NAME  "GKrellMPC"
#define PLUGIN_STYLE_NAME  PLUGIN_NAME

#include <gkrellm2/gkrellm.h>

typedef enum {
	  MPC_WHEEL_ACTION_VOLUME
	, MPC_WHEEL_ACTION_POSITION
} MPC_WHEEL_ACTIONS;

typedef enum {
	  MPC_RIGHTCLICK_ACTION_CONFIG
	, MPC_RIGHTCLICK_ACTION_PLAYLIST
} MPC_RIGHTCLICK_ACTIONS;

typedef enum {
	  MPC_MIDDLECLICK_ACTION_PAUSE
	, MPC_MIDDLECLICK_ACTION_DROP
} MPC_MIDDLECLICK_ACTIONS;

typedef enum {
	  MPC_DROP_ACTION_CLEARADDPLAY
	, MPC_DROP_ACTION_ADDPLAY
	, MPC_DROP_ACTION_ADD
} MPC_DROP_ACTIONS;

/*
 * Globals from gkrellmpc.c
 */
extern GkrellmPanel * mpc_panel;
extern GkrellmDecal * mpc_status_decal;
extern gint mpc_id;

/*
 * Globals from mpd.c
 */
extern GIOChannel   * mpc_mpd;

/*
 * Globals from conf.c
 */
extern gchar        * mpc_conf_hostname;
extern gint           mpc_conf_port;
extern gint           mpc_conf_scrollspeed;
extern MPC_WHEEL_ACTIONS mpc_conf_wheelaction;
extern gint           mpc_conf_wheelamount;
extern MPC_RIGHTCLICK_ACTIONS mpc_conf_rightclick;
extern MPC_MIDDLECLICK_ACTIONS mpc_conf_middleclick;
extern MPC_DROP_ACTIONS mpc_conf_drop;

/*
 * Globals from playlist.c
 */
extern GtkWidget    * mpc_playlist;
extern gint           mpc_playlistversion;

/*
 * Globals from addlist.c
 */
extern GtkWidget    * mpc_addlist;

#endif
