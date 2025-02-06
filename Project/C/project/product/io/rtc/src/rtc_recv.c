/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_tools.h"
#include "dave_rtc.h"
#include "dave_os.h"
#include "rtc_main.h"
#include "rtc_token.h"
#include "rtc_param.h"
#include "tlv_parse.h"
#include "rtc_log.h"

static RTCClient *
_rtc_recv_load_client(RTCDataReq *pReq)
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
rtc_recv(RTCDataReq *pReq)
{
	RTCClient *pClient;
	RTCToken *pToken;
	MBUF *data = NULL;

	pClient = _rtc_recv_load_client(pReq);
	if(pClient == NULL)
	{
		RTCLOG("can't find req token:%s", pReq->token);
		return;
	}

	if((ms8(pReq->data) == NULL) || (mlen(pReq->data) == 0))
	{
		RTCDEBUG("END %s", pReq->token);
		data = tlv_parse_set_close();
	}
	else
	{
		pToken = rtc_token_inq(pClient->token);

		if(pToken == NULL)
		{
			RTCABNOR("can't find the client token:%s", pClient->token);
		}
		else
		{
			RTCDEBUG("%d %s", mlen(pReq->data), pReq->token);
			data = tlv_parse_set_data(pReq->token, pReq->data_format, pToken->local_serial, ms8(pReq->data), mlen(pReq->data));
			pToken->local_serial ++;
		}
	}

	rtc_main_send(pClient, data);	
}

