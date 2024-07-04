/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_tools.h"
#include "dave_rtc.h"
#include "dave_os.h"
#include "tlv_parse.h"
#include "rtc_token.h"
#include "rtc_param.h"
#include "rtc_log.h"

static void
_rtc_recv_socket_send(RTCClient *pClient, MBUF *data)
{
	SocketWrite *pWrite = thread_msg(pWrite);

	pWrite->socket = pClient->socket;
	pWrite->data_len = mlen(data);
	pWrite->data = data;
	pWrite->close_flag = dave_false;

	name_msg(SOCKET_THREAD_NAME, SOCKET_WRITE, pWrite);
}

static RTCClient *
_rtc_recv_load_client(RTCReq *pReq)
{
	ub load_counter;
	RTCClient *pClient;

	for(load_counter=0; load_counter<6; load_counter++)
	{
		pClient = rtc_token_inq_client(pReq->token);
		if(pClient != NULL)
		{
			return pClient;
		}

		dave_os_sleep(100);
	}

	return NULL;
}

// =====================================================================

void
rtc_recv(RTCReq *pReq)
{
	RTCClient *pClient;
	MBUF *data;

	pClient = _rtc_recv_load_client(pReq);
	if(pClient == NULL)
	{
		RTCLOG("can't find token:%s", pReq->token);
		return;
	}

	if((pReq->content == NULL) || (pReq->content->len == 0))
	{
		RTCDEBUG("END %s", pReq->token);
		data = tlv_parse_set_close();
	}
	else
	{
		RTCDEBUG("%d %s", mlen(pReq->content), pReq->token);
		data = tlv_parse_set_app_data(pReq->token, ms8(pReq->content), mlen(pReq->content));
	}

	_rtc_recv_socket_send(pClient, data);	
}

