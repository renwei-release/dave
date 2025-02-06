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
	if(pToken->data_length >= (TOKEN_DATA_BUFFER_MAX / 2))
	{
		RTCDataReq *pReq = thread_msg(pReq);

		dave_strcpy(pReq->token, pToken->token, sizeof(pReq->token));
		dave_strcpy(pReq->src, RTC_THREAD_NAME, sizeof(pReq->src));
		dave_strcpy(pReq->dst, pToken->src_name, sizeof(pReq->dst));
		pReq->sequence_number = pToken->send_serial;
		dave_strcpy(pReq->data_format, pToken->data_format, sizeof(pReq->data_format));
		pReq->data = t_a2b_bin_to_mbuf(pToken->data_buffer, pToken->data_length);
		pReq->ptr = pToken;

		pToken->data_length = 0;

		gid_msg(pToken->src_gid, pToken->src_name, RTC_DATA_REQ, pReq);
	}
}

static RTCToken *
_rtc_send_load_token(RTCClient *pClient, s8 *tlv_ptr, ub tlv_len)
{
	if((pClient->token[0] == 0x00)
		&& (tlv_parse_get_token(pClient->token, sizeof(pClient->token), tlv_ptr, tlv_len) == dave_true))
	{
		rtc_token_add_client(pClient->token, pClient);
	}

	return rtc_token_inq(pClient->token);
}

static void
_rtc_send_pre_buffer_clear(RTCToken *pToken)
{
	MBUF *pre_buffer;
	ub clean_counter;

	clean_counter = 0;

	while((clean_counter ++) < 1024)
	{
		pre_buffer = kv_del_ub_ptr(pToken->pre_buffer_kv, (ub)(pToken->recv_serial));
		if(pre_buffer == NULL)
			break;

		if((pToken->data_length + mlen(pre_buffer)) >= TOKEN_DATA_BUFFER_MAX)
		{
			RTCLOG("data buffer overflow:%d/%d", pToken->data_length, mlen(pre_buffer));
		}
		else
		{
			pToken->data_length += dave_memcpy(&pToken->data_buffer[pToken->data_length], ms8(pre_buffer), mlen(pre_buffer));
		}

		dave_mfree(pre_buffer);
	}
}

// =====================================================================

void
rtc_send(RTCClient *pClient, s8 *tlv_ptr, ub tlv_len)
{
	RTCToken *pToken;
	u16 my_data_serial;
	unsigned char *my_data_ptr;
	ub my_data_length;

	pToken = _rtc_send_load_token(pClient, tlv_ptr, tlv_len);
	if(pToken == NULL)
	{
		RTCLOG("invalid token:%s tlv:%d", pClient->token, tlv_len);
		return;
	}

	tlv_parse_get_tlv(pToken, &my_data_serial, &my_data_ptr, &my_data_length, tlv_ptr, tlv_len);

	if(my_data_serial < pToken->recv_serial)
	{
		RTCLOG("Ineffective serial:%d/%d", my_data_serial, pToken->recv_serial);
	}
	else if(my_data_serial == pToken->recv_serial)
	{
		pToken->recv_serial ++;
	
		if((pToken->data_length + my_data_length) >= TOKEN_DATA_BUFFER_MAX)
		{
			RTCLOG("data buffer overflow:%d/%d", pToken->data_length, my_data_length);
		}
		else
		{
			pToken->data_length += dave_memcpy(&pToken->data_buffer[pToken->data_length], my_data_ptr, my_data_length);
		}

		_rtc_send_pre_buffer_clear(pToken);

		_rtc_send(pToken);
	}
	else
	{
		MBUF *pre_buffer = t_a2b_bin_to_mbuf((s8 *)my_data_ptr, my_data_length);

		kv_add_ub_ptr(pToken->pre_buffer_kv, (ub)my_data_serial, pre_buffer);
	}
}

