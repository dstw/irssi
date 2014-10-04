#ifndef H_IRSSI_SRC_CORE_PROXY_PRIV_H
#define H_IRSSI_SRC_CORE_PROXY_PRIV_H

#include "settings.h"
#include <stdbool.h>

/* stolen from linux kernel */
#define container_of(ptr, type, member)  __extension__ ({	    \
	const __typeof__( ((type *)0)->member ) *__mptr = (ptr);    \
	(type *)( (char *)__mptr - offsetof(type,member) );})


inline static void _network_proxy_create(struct network_proxy *dst)
{
	dst->port = settings_get_int("proxy_port");
	dst->host = g_strdup(settings_get_str("proxy_address"));
}

inline static void _network_proxy_clone(struct network_proxy *dst, const struct network_proxy *src)
{
	dst->host = g_strdup(src->host);
	dst->port = src->port;

	dst->destroy = src->destroy;
	dst->connect = src->connect;
	dst->clone = src->clone;
}

inline static void _network_proxy_destroy(struct network_proxy *proxy)
{
	g_free(proxy->host);
}



inline static bool _network_proxy_send_all(GIOChannel *ch, const void *buf, ssize_t len)
{
	GError *err = NULL;
	gsize written;
	GIOStatus status;

	while ((status=g_io_channel_write_chars(ch, buf, len, &written,
						&err))==G_IO_STATUS_AGAIN)
		continue;

	if (status==G_IO_STATUS_NORMAL)
		return true;

	if (err) {
		g_warning("failed to send proxy request: %s", err->message);
		g_error_free(err);
	}

	return false;
}

inline static bool _network_proxy_recv_all(GIOChannel *ch, void *buf_v, size_t len)
{
	GError *err = NULL;
	gchar *buf = buf_v;

	while (len>0) {
		GIOStatus status;
		gsize l;

		status = g_io_channel_read_chars(ch, buf, len, &l, &err);
		if (status==G_IO_STATUS_AGAIN)
			continue;
		if (status!=G_IO_STATUS_NORMAL)
			break;

		buf += l;
		len -= l;
	}

	if (len==0)
		return true;

	if (err) {
		g_warning("failed to send proxy request: %s", err->message);
		g_error_free(err);
	}

	return false;
}

inline static bool _network_proxy_flush(GIOChannel *ch)
{
	GError *err = NULL;
	GIOStatus status;

	while ((status=g_io_channel_flush(ch, &err))==G_IO_STATUS_AGAIN)
		continue;

	if (status==G_IO_STATUS_NORMAL)
		return true;

	if (err) {
		g_warning("failed to flush proxy channel: %s", err->message);
		g_error_free(err);
	}

	return false;
}

#endif
