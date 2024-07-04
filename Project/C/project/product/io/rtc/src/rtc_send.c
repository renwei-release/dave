/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_tools.h"
#include "dave_rtc.h"
#include "tlv_parse.h"
#include "rtc_token.h"
#include "rtc_param.h"
#include "rtc_log.h"

static void
_rtc_send(RTCToken *pToken)
{
	RTCReq *pReq = thread_msg(pReq);
	RTCRsp rsp;

	dave_strcpy(pReq->token, pToken->token, sizeof(pReq->token));
	dave_strcpy(pReq->src, RTC_THREAD_NAME, sizeof(pReq->src));
	dave_strcpy(pReq->dst, pToken->src_name, sizeof(pReq->dst));
	if(pToken->app_data_length == 0)
	{
		RTCDEBUG("END %s", pToken->token);
		pReq->content = NULL;
	}
	else
	{
		RTCDEBUG("%d %s", pToken->app_data_length, pToken->token);
		pReq->content = t_a2b_bin_to_mbuf(pToken->app_data_buffer, pToken->app_data_length);
	}
	pReq->ptr = pToken;

	sync_msg(pToken->src_id, RTC_REQ, pReq, RTC_RSP, &rsp);

	pToken->app_data_length = 0;
}

static void
_rtc_app_data_to_buffer(s8 *token, s8 *value_ptr, ub value_len)
{
	RTCToken *pToken = rtc_token_inq(token);

	if(pToken == NULL)
	{
		RTCLOG("can't find token data:%s", token);
		return;
	}

	if(value_len > TOKEN_APP_DATA_BUFFER_MAX)
	{
		RTCLOG("value_len:%d is too longer!", value_len);
		return;
	}

	if(value_len == 0)
	{
		if(pToken->app_data_length != 0)
		{
			_rtc_send(pToken);
		}
		// send END flag.
		pToken->app_data_length = 0;
		_rtc_send(pToken);
		return;
	}

	if((pToken->app_data_length + value_len) > TOKEN_APP_DATA_BUFFER_MAX)
	{
		_rtc_send(pToken);
	}

	pToken->app_data_length += dave_memcpy(&pToken->app_data_buffer[pToken->app_data_length], value_ptr, value_len);

	if((pToken->app_data_length + 1024) >= TOKEN_APP_DATA_BUFFER_MAX)
	{
		_rtc_send(pToken);
	}
}

// =====================================================================

ub
rtc_send(RTCClient *pClient)
{
	s8 *value_ptr;
	ub value_len;

	if((pClient->token[0] == 0x00)
		&& (tlv_parse_get_token(pClient->token, sizeof(pClient->token), pClient->tlv_buffer_ptr, pClient->tlv_buffer_r_index) == dave_true))
	{
		rtc_token_add_client(pClient->token, pClient);
	}

	if(tlv_parse_get_app_data(&value_ptr, &value_len, pClient->tlv_buffer_ptr, pClient->tlv_buffer_r_index) == dave_true)
	{
		_rtc_app_data_to_buffer(pClient->token, value_ptr, value_len);
	}

	return pClient->tlv_buffer_r_index;
}

