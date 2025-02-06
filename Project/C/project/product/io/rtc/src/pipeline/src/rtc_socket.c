/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_rtc.h"
#include "rtc_param.h"
#include "rtc_token.h"
#include "rtc_send.h"
#include "rtc_recv.h"
#include "rtc_main.h"
#include "rtc_cfg.h"
#include "tlv_parse.h"
#include "rtc_log.h"

#define RTC_SOCKET_CLIENT_MAX DAVE_SERVER_SUPPORT_SOCKET_MAX

static s32 _rtc_server_socket = INVALID_SOCKET_ID;
static RTCClient _rtc_client[RTC_SOCKET_CLIENT_MAX];

static void
_rtc_socket_client_reset(RTCClient *pClient)
{
	pClient->type = RTCClientType_socket;

	pClient->socket = INVALID_SOCKET_ID;

	pClient->token[0] = 0x00;

	pClient->tlv_buffer_ptr = NULL;
	pClient->tlv_buffer_len = 0;
	pClient->tlv_buffer_r_index = pClient->tlv_buffer_w_index = 0;
}

static void
_rtc_socket_client_all_reset(void)
{
	ub client_index;
	RTCClient *pClient;

	dave_memset(_rtc_client, 0x00, sizeof(_rtc_client));

	for(client_index=0; client_index<RTC_SOCKET_CLIENT_MAX; client_index++)
	{
		pClient = &_rtc_client[client_index];

		t_lock_reset(&(pClient->event_pv));

		_rtc_socket_client_reset(pClient);
	}
}

static RTCClient *
_rtc_socket_client_malloc(s32 socket)
{
	RTCClient *pClient;

	if(socket >= RTC_SOCKET_CLIENT_MAX)
	{
		RTCLOG("invalid client_socket:%d/%d", socket, RTC_SOCKET_CLIENT_MAX);
		return NULL;
	}

	pClient = &_rtc_client[socket];

	_rtc_socket_client_reset(pClient);

	pClient->type = RTCClientType_socket;
	pClient->socket = socket;

	pClient->tlv_buffer_len = TLV_BUFFER_LENGTH_MAX;
	pClient->tlv_buffer_ptr = dave_malloc(pClient->tlv_buffer_len);
	pClient->tlv_buffer_r_index = pClient->tlv_buffer_w_index = 0;

	return pClient;
}

static void
_rtc_socket_client_free(s32 socket)
{
	RTCClient *pClient;

	if(socket >= RTC_SOCKET_CLIENT_MAX)
	{
		RTCLOG("invalid client_socket:%d/%d", socket, RTC_SOCKET_CLIENT_MAX);
		return;
	}

	pClient = &_rtc_client[socket];

	dave_free(pClient->tlv_buffer_ptr);

	rtc_token_del(pClient->token);

	_rtc_socket_client_reset(pClient);
}

static void
_rtc_socket_close(s32 socket)
{
	SocketDisconnectReq *pReq = thread_msg(pReq);

	pReq->socket = socket;
	pReq->ptr = NULL;

	name_msg(SOCKET_THREAD_NAME, SOCKET_DISCONNECT_REQ, pReq);
}

static dave_bool
_rtc_socket_bind(void)
{
	SocketBindReq *pReq = thread_reset_msg(pReq);
	SocketBindRsp rsp;

	pReq->NetInfo.domain = DM_SOC_PF_RAW_INET;
	pReq->NetInfo.type = TYPE_SOCK_STREAM;
	pReq->NetInfo.addr_type = NetAddrIPType;
	pReq->NetInfo.addr.ip.ver = IPVER_IPV4;
	dave_memset(pReq->NetInfo.addr.ip.ip_addr, 0x00, 16);
	pReq->NetInfo.port = rtc_server_port();
	pReq->NetInfo.fixed_src_flag = NotFixedPort;
	pReq->NetInfo.enable_keepalive_flag = KeepAlive_enable;
	pReq->NetInfo.netcard_bind_flag = NetCardBind_disable;

	RTCLOG("%s", ipv4str(pReq->NetInfo.addr.ip.ip_addr, pReq->NetInfo.port));

	if(sync_msg(thread_id(SOCKET_THREAD_NAME), SOCKET_BIND_REQ, pReq, SOCKET_BIND_RSP, &rsp) == NULL)
	{
		RTCLOG("port:%d bind failed", rtc_server_port());
		return dave_false;
	}
	else
	{
		_rtc_server_socket = rsp.socket;

		RTCLOG("port:%d bind success! socket:%d", rtc_server_port(), _rtc_server_socket);
		return dave_true;
	}
}

static void
_rtc_socket_unbind(void)
{
	SocketDisconnectReq *pReq = thread_reset_msg(pReq);

	if(_rtc_server_socket == INVALID_SOCKET_ID)
		return;

	pReq->socket = _rtc_server_socket;

	name_msg(SOCKET_THREAD_NAME, SOCKET_DISCONNECT_REQ, pReq);
}

static void
_rtc_socket_plugin(MSGBODY *msg)
{
	SocketPlugIn *pPlugin = (SocketPlugIn *)(msg->msg_body);
	s32 client_socket = pPlugin->child_socket;
	RTCClient *pClient;

	if(client_socket >= RTC_SOCKET_CLIENT_MAX)
	{
		RTCLOG("invalid client_socket:%d/%d", client_socket, RTC_SOCKET_CLIENT_MAX);
		return;
	}

	pClient = &_rtc_client[client_socket];

	if(pClient->socket != INVALID_SOCKET_ID)
	{
		RTCABNOR("invalid client:%d", client_socket);
		return;
	}

	RTCLOG("socket:%d", pPlugin->child_socket);

	_rtc_socket_client_malloc(client_socket);
}

static void
_rtc_socket_plugout(MSGBODY *msg)
{
	SocketPlugOut *pPlugout = (SocketPlugOut *)(msg->msg_body);
	s32 client_socket = pPlugout->socket;
	RTCClient *pClient;

	if(client_socket >= RTC_SOCKET_CLIENT_MAX)
	{
		RTCLOG("invalid client_socket:%d/%d", client_socket, RTC_SOCKET_CLIENT_MAX);
		return;
	}

	pClient = &_rtc_client[client_socket];

	if(pClient->socket == INVALID_SOCKET_ID)
	{
		RTCABNOR("invalid client:%d", client_socket);
		return;
	}

	RTCLOG("socket:%d", pPlugout->socket);

	_rtc_socket_client_free(client_socket);
}

static ub
_rtc_socket_event(SocketRawEvent *pEvent, RTCClient *pClient)
{
	ub rx_len;

	rx_len = pClient->tlv_buffer_len - pClient->tlv_buffer_w_index;
	if((rx_len == 0) || (rx_len > pClient->tlv_buffer_len))
	{
		RTCABNOR("socket:%d token:%s buffer overflow:%d/%d!",
			pClient->socket, pClient->token, pClient->tlv_buffer_len, pClient->tlv_buffer_w_index);
		return 0;
	}

	if(dave_os_recv(pEvent->os_socket, &(pEvent->NetInfo), (u8 *)(&(pClient->tlv_buffer_ptr[pClient->tlv_buffer_w_index])), &rx_len) == dave_false)
	{
		RTCLOG("socket:%d recv close", pEvent->socket);	
		_rtc_socket_close(pEvent->socket);
		return 0;
	}

	if(rx_len == 0)
	{
		return 0;
	}

	pClient->tlv_buffer_w_index += rx_len;

	rtc_main_recv(pClient);

	dave_mfree(pEvent->data);

	RTCDEBUG("socket:%d rx_len:%ld index:%ld/%ld",
		pEvent->socket, rx_len,
		pClient->tlv_buffer_r_index, pClient->tlv_buffer_w_index);

	return rx_len;
}

static void
_rtc_socket_safe_event(MSGBODY *msg)
{
	SocketRawEvent *pEvent = (SocketRawEvent *)(msg->msg_body);
	s32 client_socket = pEvent->socket;
	RTCClient *pClient;

	if(client_socket >= RTC_SOCKET_CLIENT_MAX)
	{
		RTCLOG("invalid client_socket:%d/%d", client_socket, RTC_SOCKET_CLIENT_MAX);
		return;
	}

	pClient = &_rtc_client[client_socket];

	SAFECODEv1(pClient->event_pv, {

		while(_rtc_socket_event(pEvent, pClient) > 0);

	} );
}

static void
_rtc_socket_init(MSGBODY *msg)
{
	if(_rtc_socket_bind() == dave_true)
	{
		reg_msg(SOCKET_PLUGIN, _rtc_socket_plugin);
		reg_msg(SOCKET_PLUGOUT, _rtc_socket_plugout);
		reg_msg(SOCKET_RAW_EVENT, _rtc_socket_safe_event);
	}

	_rtc_socket_client_all_reset();
}

static void
_rtc_socket_exit(void)
{
	unreg_msg(SOCKET_PLUGIN);
	unreg_msg(SOCKET_PLUGOUT);
	unreg_msg(SOCKET_READ);

	_rtc_socket_unbind();
}

// =====================================================================

void
rtc_socket_init(void)
{
	inner_loop(_rtc_socket_init);
}

void
rtc_socket_exit(void)
{
	_rtc_socket_exit();
}

void
rtc_socket_send(RTCClient *pClient, MBUF *data)
{
	SocketWrite *pWrite = thread_msg(pWrite);

	pWrite->socket = pClient->socket;
	pWrite->data_len = mlen(data);
	pWrite->data = data;
	pWrite->close_flag = dave_false;

	name_msg(SOCKET_THREAD_NAME, SOCKET_WRITE, pWrite);
}

