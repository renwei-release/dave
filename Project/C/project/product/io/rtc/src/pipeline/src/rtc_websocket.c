/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_3rdparty.h"
#include "rtc_main.h"
#include "rtc_token.h"
#include "rtc_log.h"

static void *_websocket_client_kv = NULL;

static RTCClient *
_rtc_websocket_malloc(void *wsi)
{
	RTCClient *pClient = dave_ralloc(sizeof(RTCClient));

	pClient->type = RTCClientType_websocket;
	pClient->wsi = wsi;

	pClient->tlv_buffer_len = TLV_BUFFER_LENGTH_MAX;
	pClient->tlv_buffer_ptr = dave_malloc(pClient->tlv_buffer_len);
	pClient->tlv_buffer_r_index = pClient->tlv_buffer_w_index = 0;

	t_lock_reset(&(pClient->event_pv));

	kv_add_ub_ptr(_websocket_client_kv, (ub)wsi, pClient);

	return pClient;
}

static void
_rtc_websocket_free(RTCClient *pClient)
{
	if(pClient == NULL)
		return;

	kv_del_ub_ptr(_websocket_client_kv, (ub)(pClient->wsi));

	dave_free(pClient->tlv_buffer_ptr);

	rtc_token_del(pClient->token);

	dave_free(pClient);
}

static void
_rtc_websocket_plugin(void *wsi)
{
	_rtc_websocket_malloc(wsi);
}

static void
_rtc_websocket_plugout(void *wsi)
{
	RTCClient *pClient = kv_del_ub_ptr(_websocket_client_kv, (ub)wsi);

	_rtc_websocket_free(pClient);
}

static void
_rtc_websocket_data_read(void *wsi, s8 *data_ptr, ub data_len)
{
	RTCClient *pClient = kv_inq_ub_ptr(_websocket_client_kv, (ub)wsi);

	t_stdio_print_hex("tlv", (u8 *)data_ptr, data_len);

	if(pClient == NULL)
	{
		RTCLOG("wsi:%lx can't find! data:%d", wsi, data_len);
		return;
	}

	if((pClient->tlv_buffer_w_index + data_len) > pClient->tlv_buffer_len)
	{
		RTCLOG("tlv buffer overflow:%d/%d/%d",
			pClient->tlv_buffer_w_index, data_len, pClient->tlv_buffer_len);
		return;
	}

	RTCLOG("data:%d/%d", pClient->tlv_buffer_w_index, data_len);

	pClient->tlv_buffer_w_index += dave_memcpy(&pClient->tlv_buffer_ptr[pClient->tlv_buffer_w_index], data_ptr, data_len);

	rtc_main_recv(pClient);
}

static RetCode
_rtc_websocket_recycle(void *ramkv, s8 *key)
{
	RTCClient *pClient = kv_del_key_ptr(ramkv, key);

	if(pClient == NULL)
		return RetCode_not_my_data;

	_rtc_websocket_free(pClient);

	return RetCode_OK;
}

// =====================================================================

void
rtc_websocket_init(void)
{
	_websocket_client_kv = kv_malloc("websocketclient", 0, NULL);

	dave_websocket_init(_rtc_websocket_plugin, _rtc_websocket_plugout, _rtc_websocket_data_read);
}

void
rtc_websocket_exit(void)
{
	dave_websocket_exit();

	kv_free(_websocket_client_kv, _rtc_websocket_recycle);
}

void
rtc_websocket_send(RTCClient *pClient, MBUF *data)
{
	dave_websocket_data_write(pClient->wsi, ms8(data), mlen(data));

	dave_mfree(data);
}

