/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_rtc.h"
#include "rtc_param.h"
#include "rtc_socket.h"
#include "rtc_websocket.h"
#include "rtc_token.h"
#include "rtc_send.h"
#include "rtc_recv.h"
#include "tlv_parse.h"
#include "rtc_cfg.h"
#include "rtc_log.h"

static void
_rtc_main_data(MSGBODY *msg)
{
	RTCDataReq *pReq = (RTCDataReq *)(msg->msg_body);

	rtc_recv(pReq);

	dave_mfree(pReq->data);
}

static void
_rtc_main_reg(MSGBODY *msg)
{
	RTCRegReq *pReq = (RTCRegReq *)(msg->msg_body);
	RTCRegRsp *pRsp = thread_msg(pRsp);
	s8 *token;

	token = rtc_token_add(msg->msg_src, msg->src_gid, msg->src_name, pReq->terminal_type, pReq->terminal_id);

	dave_strcpy(pRsp->token, token, sizeof(pRsp->token));
	dave_strcpy(pRsp->terminal_type, pReq->terminal_type, sizeof(pRsp->terminal_type));
	dave_strcpy(pRsp->terminal_id, pReq->terminal_id, sizeof(pRsp->terminal_id));
	pRsp->ptr = pReq->ptr;

	RTCLOG("%s/%s %s/%s -> token:%s",
		msg->src_gid, msg->src_name,
		pRsp->terminal_type, pRsp->terminal_id,
		pRsp->token);

	id_msg(msg->msg_src, RTC_REG_RSP, pRsp);
}

static void
_rtc_main_unreg(MSGBODY *msg)
{
	RTCUnregReq *pReq = (RTCUnregReq *)(msg->msg_body);
	RTCUnregRsp *pRsp = thread_msg(pRsp);

	rtc_token_del(pReq->token);

	dave_strcpy(pRsp->token, pReq->token, sizeof(pRsp->token));
	dave_strcpy(pRsp->terminal_type, pReq->terminal_type, sizeof(pRsp->terminal_type));
	dave_strcpy(pRsp->terminal_id, pReq->terminal_id, sizeof(pRsp->terminal_id));
	pRsp->ptr = pReq->ptr;

	RTCLOG("%s/%s %s/%s -> token:%s",
		msg->src_gid, msg->src_name,
		pRsp->terminal_type, pRsp->terminal_id,
		pRsp->token);

	id_msg(msg->msg_src, RTC_UNREG_RSP, pRsp);
}

// =====================================================================

void
rtc_main_init(void)
{
	reg_msg(RTC_DATA_REQ, _rtc_main_data);
	reg_msg(RTC_REG_REQ, _rtc_main_reg);
	reg_msg(RTC_UNREG_REQ, _rtc_main_unreg);
}

void
rtc_main_exit(void)
{
	unreg_msg(RTC_DATA_REQ);
	unreg_msg(RTC_REG_REQ);
	unreg_msg(RTC_UNREG_REQ);
}

void
rtc_main_recv(RTCClient *pClient)
{
	while(tlv_parse_find_end(pClient) > 0)
	{
		RTCLOG("find end:%d", pClient->tlv_buffer_r_index);

		rtc_send(pClient, pClient->tlv_buffer_ptr, pClient->tlv_buffer_r_index);

		if(pClient->tlv_buffer_r_index >= pClient->tlv_buffer_w_index)
		{
			pClient->tlv_buffer_r_index = pClient->tlv_buffer_w_index = 0;	
		}
		else
		{
			pClient->tlv_buffer_w_index = dave_memmove(
				pClient->tlv_buffer_ptr,
				&pClient->tlv_buffer_ptr[pClient->tlv_buffer_r_index],
				pClient->tlv_buffer_w_index - pClient->tlv_buffer_r_index);

			pClient->tlv_buffer_r_index = 0;
		}
	}
}

void
rtc_main_send(RTCClient *pClient, MBUF *data)
{
	if((pClient == NULL) || (data == NULL))
	{
		return;
	}

	switch(pClient->type)
	{
		case RTCClientType_socket:
				rtc_socket_send(pClient, data);
			break;
		case RTCClientType_websocket:
				rtc_websocket_send(pClient, data);
			break;
		default:
				RTCABNOR("unsupport type:%s", t_auto_RTCClientType_str(pClient->type));
				dave_mfree(data);
			break;
	}
}

