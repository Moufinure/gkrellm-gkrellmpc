/*
 * $Header: /cvsroot/gkrellmpc/url.h,v 1.2 2005/01/03 21:47:27 mina Exp $
 */

#ifndef _URL_H
#define _URL_H

void mpc_url_init (void);
void mpc_url_cleanup (void);
gchar * mpc_url_parse (const gchar *);
size_t mpc_url_header(void *ptr, size_t size, size_t nmemb, void *stream);
size_t mpc_url_data(void *ptr, size_t size, size_t nmemb, void *stream);
void mpc_url_drop (gchar * droppedurl);

#endif
