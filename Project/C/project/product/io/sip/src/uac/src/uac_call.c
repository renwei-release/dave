/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_sip.h"
#include "dave_osip.h"
#include "dave_rtp.h"
#include "sip_signal.h"
#include "sip_call.h"
#include "uac_main.h"
#include "uac_rtp_buffer.h"
#include "uac_cfg.h"
#include "uac_log.h"

static UACCall *
_uac_call_rtp_to_call(RTP *pRTP)
{
	if(pRTP->call == NULL)
	{
		UACABNOR("the call is NULL!");
		return NULL;
	}

	SIPCall *pCall = (SIPCall *)(pRTP->call);

	if(pCall->user_ptr == NULL)
	{
		UACABNOR("the user_ptr is NULL!");
		return NULL;
	}

	return (UACCall *)(pCall->user_ptr);
}

static void
_uac_call_rtp_start(void *rtp)
{
	RTP *pRTP = (RTP *)rtp;
	UACCall *pUACCall = _uac_call_rtp_to_call(pRTP);
	RTPStartReq *pReq = thread_msg(pReq);

	UACLOG("%s %s->%s owner:%s",
		pRTP->call_id, pRTP->call_from, pRTP->call_to,
		thread_name(pUACCall->owner_id));

	dave_strcpy(pReq->call_id, pRTP->call_id, sizeof(pReq->call_id));
	dave_strcpy(pReq->call_from, pRTP->call_from, sizeof(pReq->call_from));
	dave_strcpy(pReq->call_to, pRTP->call_to, sizeof(pReq->call_to));

	id_co(pUACCall->owner_id, RTP_START_REQ, pReq, RTP_START_RSP);
}

static void
_uac_call_rtp_stop(void *rtp)
{
	RTP *pRTP = (RTP *)rtp;
	UACCall *pUACCall = _uac_call_rtp_to_call(pRTP);
	RTPStopReq *pReq = thread_msg(pReq);

	UACLOG("%s %s->%s owner:%s",
		pRTP->call_id, pRTP->call_from, pRTP->call_to,
		thread_name(pUACCall->owner_id));

	dave_strcpy(pReq->call_id, pRTP->call_id, sizeof(pReq->call_id));
	dave_strcpy(pReq->call_from, pRTP->call_from, sizeof(pReq->call_from));
	dave_strcpy(pReq->call_to, pRTP->call_to, sizeof(pReq->call_to));

	id_co(pUACCall->owner_id, RTP_STOP_REQ, pReq, RTP_STOP_RSP);
}

static void
_uac_call_rtp_recv(void *rtp, u8 payload_type, u16 sequence_number, u32 timestamp, u32 ssrc, s8 *payload_ptr, ub payload_len)
{
	RTP *pRTP = (RTP *)rtp;
	UACCall *pUACCall = _uac_call_rtp_to_call(pRTP);
	MBUF *payload_data;
	u16 send_sequence_number;

	UACDEBUG("call:%s->%s owner:%s", pRTP->call_from, pRTP->call_to, thread_name(pUACCall->owner_id));

	payload_data = uac_rtp_buffer(
		&send_sequence_number,
		&pUACCall->rtp_buffer,
		payload_type, sequence_number, timestamp, ssrc, payload_ptr, payload_len);

	if(payload_data != NULL)
	{
		RTPDataReq *pReq = thread_msg(pReq);
		dave_strcpy(pReq->call_id, pRTP->call_id, sizeof(pReq->call_id));
		dave_strcpy(pReq->call_from, pRTP->call_from, sizeof(pReq->call_from));
		dave_strcpy(pReq->call_to, pRTP->call_to, sizeof(pReq->call_to));
		pReq->payload_type = payload_type;
		pReq->sequence_number = send_sequence_number;
		pReq->timestamp = timestamp;
		pReq->ssrc = ssrc;
		pReq->payload_data = payload_data;
		pReq->ptr = pReq;

		id_msg(pUACCall->owner_id, RTP_DATA_REQ, pReq);
	}
}

static void
_uac_call_start(void *call)
{
	SIPCall *pSIPCall = (SIPCall *)call;
	UACCall *pUACCall = (UACCall *)(pSIPCall->user_ptr);
	SIPStartReq *pReq = thread_msg(pReq);

	UACLOG("call:%s start!", pSIPCall->call_data);

	dave_strcpy(pReq->call_id, osip_call_id_get_number(pSIPCall->call_id), sizeof(pReq->call_id));
	dave_strcpy(pReq->call_from, osip_uri_get_username(osip_from_get_url(pSIPCall->from)), sizeof(pReq->call_from));
	dave_strcpy(pReq->call_to, osip_uri_get_username(osip_to_get_url(pSIPCall->to)), sizeof(pReq->call_to));

	id_co(pUACCall->owner_id, SIP_START_REQ, pReq, SIP_START_RSP);
}

static void
_uac_call_end(void *call)
{
	SIPCall *pSIPCall = (SIPCall *)call;
	UACCall *pUACCall = (UACCall *)(pSIPCall->user_ptr);
	SIPStartReq *pReq = thread_msg(pReq);

	UACLOG("call:%s end!", pSIPCall->call_data);

	uac_main_del_call(pSIPCall->call_data);

	dave_strcpy(pReq->call_id, osip_call_id_get_number(pSIPCall->call_id), sizeof(pReq->call_id));
	dave_strcpy(pReq->call_from, osip_uri_get_username(osip_from_get_url(pSIPCall->from)), sizeof(pReq->call_from));
	dave_strcpy(pReq->call_to, osip_uri_get_username(osip_to_get_url(pSIPCall->to)), sizeof(pReq->call_to));

	id_co(pUACCall->owner_id, SIP_STOP_REQ, pReq, SIP_STOP_RSP);
}

// =====================================================================

RetCode
uac_call(s8 *call_id_ptr, ub call_id_len, ThreadId owner_id, s8 *phone_number)
{
	UACCall *pUACCall;
	SIPCall *pSIPCall;

	pUACCall = uac_main_inq_phone_number(phone_number);
	if(pUACCall != NULL)
	{
		UACLOG("thread:%s/%s the call:%s on my table!",
			thread_name(pUACCall->owner_id), thread_name(owner_id),
			phone_number);
		return RetCode_Resource_conflicts;
	}

	pUACCall = uac_main_build_call(owner_id, phone_number);

	pSIPCall = sip_call(
		uac_main_signal(), phone_number,
		_uac_call_rtp_start, _uac_call_rtp_stop, _uac_call_rtp_recv,
		_uac_call_start, _uac_call_end,
		pUACCall);

	if(pSIPCall == NULL)
	{
		UACLOG("thread:%s/%s the call:%s failed!",
			thread_name(pUACCall->owner_id), thread_name(owner_id),
			phone_number);
		return RetCode_Resource_conflicts;
	}

	uac_main_setup_call(pUACCall, pSIPCall);

	if(call_id_ptr != NULL)
	{
		dave_strcpy(call_id_ptr, osip_call_id_get_number(pSIPCall->call_id), call_id_len);
	}

	UACLOG("call_id:%s owner:%s/%lx phone_number:%s",
		call_id_ptr, thread_name(owner_id), owner_id,
		phone_number);

	return RetCode_OK;
}

RetCode
uac_bye(s8 *call_id_ptr, ub call_id_len, ThreadId owner_id, s8 *phone_number)
{
	UACCall *pUACCall = uac_main_inq_phone_number(phone_number);

	if(pUACCall == NULL)
	{
		UACLOG("call:%s is end!", phone_number);
		return RetCode_OK;
	}

	if(pUACCall->owner_id != owner_id)
	{
		UACLOG("owner:%s/%lx %s/%lx mismatch! phone_number:%s",
			thread_name(pUACCall->owner_id), pUACCall->owner_id,
			thread_name(owner_id), owner_id,
			phone_number);
	}

	if(call_id_ptr != NULL)
	{
		dave_strcpy(call_id_ptr, osip_call_id_get_number(pUACCall->call->call_id), call_id_len);
	}

	sip_bye(uac_main_signal(), phone_number);

	UACLOG("call_id:%s owner:%s/%lx phone_number:%s",
		call_id_ptr, thread_name(owner_id), owner_id,
		phone_number);

	return RetCode_OK;
}

void
uac_rtp(
	s8 *call_id, s8 *call_from, s8 *call_to,
	u8 payload_type, u16 sequence_number, u32 timestamp, u32 ssrc,
	s8 *payload_ptr, ub payload_len)
{
	SIPCall *pSIPCall = sip_my_call(uac_main_signal(), call_id);

	if(pSIPCall == NULL)
	{
		UACLOG("call_id:%s can't find!", call_id);
		return;
	}

	if(pSIPCall->rtp == NULL)
	{
		UACLOG("call_id:%s rtp is NULL!", call_id);
		return;
	}

	pSIPCall->rtp->data_send(pSIPCall->rtp, payload_type, sequence_number, timestamp, ssrc, payload_ptr, payload_len);
}

