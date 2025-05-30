/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#if defined(WEBSOCKET_3RDPARTY)
#include "libwebsockets.h"
#include "dave_os.h"
#include "dave_3rdparty.h"
#include "dave_tools.h"
#include "party_log.h"

static void *_websocket_thread_body = NULL;
static websocket_plugin _plugin = NULL;
static websocket_plugout _plugout = NULL;
static websocket_read _data_read = NULL;

static int
_websocket_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
    switch(reason)
	{
        case LWS_CALLBACK_ESTABLISHED:
            	PARTYLOG("Connection established");
				_plugin((void *)wsi);
            break;
        case LWS_CALLBACK_RECEIVE:
            	PARTYLOG("Received:%d", len);
				_data_read((void *)wsi, (s8 *)in, (ub)len);
            break;
		case LWS_CALLBACK_SERVER_WRITEABLE:
				PARTYLOG("LWS_CALLBACK_SERVER_WRITEABLE");
			break;
        case LWS_CALLBACK_CLOSED:
				PARTYLOG("Connection closed");
				_plugout((void *)wsi);
			break;
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
				PARTYLOG("Client connection error");
			break;
        case LWS_CALLBACK_CLIENT_CLOSED:
				PARTYLOG("Client connection closed");
			break;
		case LWS_CALLBACK_WSI_CREATE:
				PARTYLOG("LWS_CALLBACK_WSI_CREATE");
			break;
		case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED:
				PARTYLOG("LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED");
			break;
		case LWS_CALLBACK_EVENT_WAIT_CANCELLED:
				PARTYLOG("LWS_CALLBACK_EVENT_WAIT_CANCELLED");
			break;
		case LWS_CALLBACK_HTTP_CONFIRM_UPGRADE:
				PARTYLOG("LWS_CALLBACK_HTTP_CONFIRM_UPGRADE");
			break;
		case LWS_CALLBACK_HTTP_BIND_PROTOCOL:
				PARTYLOG("LWS_CALLBACK_HTTP_BIND_PROTOCOL");
			break;
		case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
				PARTYLOG("LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION");
			break;
		case LWS_CALLBACK_ADD_HEADERS:
				PARTYLOG("LWS_CALLBACK_ADD_HEADERS");
			break;
		case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
				PARTYLOG("LWS_CALLBACK_WS_PEER_INITIATED_CLOSE");
			break;
		case LWS_CALLBACK_WSI_DESTROY:
				PARTYLOG("LWS_CALLBACK_WSI_DESTROY");
			break;
		case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
				PARTYLOG("LWS_CALLBACK_FILTER_NETWORK_CONNECTION");
			break;
		default:
				PARTYLOG("unsupport reason:%d", reason);
			break;
	}
    return 0;
}

static struct lws_protocols protocols[] = {
    {
        "websocket",
        _websocket_callback,
        0,
        1024,
    },
    { NULL, NULL, 0, 0 } /* terminator */
};

static struct lws_context *
_rtc_websocket_start(void)
{
	struct lws_context_creation_info info;
	struct lws_context *context;

    memset(&info, 0, sizeof(info));
    info.port = 8080;
    info.protocols = protocols;

    context = lws_create_context(&info);
    if (context == NULL)
	{
        PARTYLOG("lws_create_context failed");
        return NULL;
    }

	return context;
}

static void
_rtc_websocket_stop(struct lws_context *context)
{
	lws_context_destroy(context);
}

static void *
_rtc_websocket_thread(void *arg)
{
	struct lws_context *context = _rtc_websocket_start();

	while((context != NULL) && (dave_os_thread_canceled(_websocket_thread_body) == dave_false))
	{
		lws_service(context, 10000);
	}

	_rtc_websocket_stop(context);

	return NULL;
}

// =====================================================================

void
dave_websocket_init(websocket_plugin plugin, websocket_plugout plugout, websocket_read data_read)
{
	_plugin = plugin;
	_plugout = plugout;
	_data_read = data_read;

	_websocket_thread_body = dave_os_create_thread("websocket", _rtc_websocket_thread, NULL);
	if(_websocket_thread_body == NULL)
	{
		PARTYLOG("i can not start websocket thread!");
	}
}

void
dave_websocket_exit(void)
{
	if(_websocket_thread_body != NULL)
	{
		dave_os_release_thread(_websocket_thread_body);
	}
}

void
dave_websocket_data_write(void *wsl, s8 *data_ptr, ub data_len)
{
	struct lws *wsi = (struct lws *)wsl;
	s8 *buf = (s8 *)dave_malloc(LWS_SEND_BUFFER_PRE_PADDING + data_len + LWS_SEND_BUFFER_POST_PADDING);

	memcpy(buf + LWS_SEND_BUFFER_PRE_PADDING, data_ptr, data_len);
	lws_write(wsi, (unsigned char *)(buf + LWS_SEND_BUFFER_PRE_PADDING), (size_t)data_len, LWS_WRITE_TEXT);

	dave_free(buf);
}

#endif

