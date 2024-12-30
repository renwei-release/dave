/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_osip.h"
#include "dave_uac.h"
#include "uac_class.h"
#include "uac_server.h"
#include "uac_client.h"
#include "uac_rtp.h"
#include "uac_rtp_msg.h"
#include "uac_log.h"

// #define ENABLE_RTP_ECHO

#define RTP_FRAME_DATA_LEN 160

const u8 _mte_bag[RTP_FRAME_DATA_LEN] = {
	0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5,
	0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5,
	0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5,
	0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5,
	0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5,
	0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5,
	0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5,
	0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5,
	0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5,
	0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5, 0xd5,
};

#ifdef ENABLE_RTP_ECHO

static RTPDATA
_uac_rtp_data_echo(
	UACRTP *pRTP,
	u8 payload_type,
	u16 sequence_number, u32 timestamp, u32 ssrc,
	s8 *payload_ptr, ub payload_len)
{
	RTPDATA echo_rtp_data;

	echo_rtp_data.payload_type = payload_type;
	echo_rtp_data.sequence_number = sequence_number;
	echo_rtp_data.timestamp = timestamp;
	echo_rtp_data.ssrc = ssrc;
	echo_rtp_data.payload_data = t_a2b_bin_to_mbuf(payload_ptr, payload_len);

	return echo_rtp_data;
}

#else

static RTPDATA
_uac_rtp_data_buffer_get(UACRTP *pRTP)
{
	RTPDATA echo_rtp_data;

	SAFECODEv1(pRTP->rtp_data_pv, {
		pRTP->sequence_number += 1;
		pRTP->timestamp += RTP_FRAME_DATA_LEN;

		echo_rtp_data.payload_type = pRTP->payload_type;
		echo_rtp_data.sequence_number = pRTP->sequence_number;
		echo_rtp_data.timestamp = pRTP->timestamp;
		echo_rtp_data.ssrc = pRTP->ssrc;

		if((pRTP->rtp_data_w_index - pRTP->rtp_data_r_index) >= RTP_FRAME_DATA_LEN)
		{
			echo_rtp_data.payload_data = t_a2b_bin_to_mbuf((s8 *)(&pRTP->rtp_data_buffer[pRTP->rtp_data_r_index]), RTP_FRAME_DATA_LEN);
			pRTP->rtp_data_r_index += RTP_FRAME_DATA_LEN;
			if(pRTP->rtp_data_r_index >= pRTP->rtp_data_w_index)
			{
				pRTP->rtp_data_w_index = pRTP->rtp_data_r_index = 0;
			}
		}
		else
		{
			echo_rtp_data.payload_data = t_a2b_bin_to_mbuf((s8 *)_mte_bag, RTP_FRAME_DATA_LEN);
		}
	});

	return echo_rtp_data;
}

static void
_uac_rtp_data_buffer_set(UACRTP *pRTP, u32 ssrc, s8 *payload_data_ptr, ub payload_data_len)
{
	SAFECODEv1(pRTP->rtp_data_pv, {
		if(pRTP->rtp_data_w_index == 0)
		{
			pRTP->current_buffer_ssrc = ssrc;

			UACLOG("ssrc:%d data_len:%d", ssrc, payload_data_len)
		}
		else
		{
			if(pRTP->current_buffer_ssrc != ssrc)
			{
				UACLOG("It is possible that the conversation was interrupted by the user! %s %s->%s ssrc:%d/%d",
					pRTP->call_id, pRTP->call_from, pRTP->call_to,
					pRTP->current_buffer_ssrc, ssrc);

				pRTP->current_buffer_ssrc = ssrc;
				pRTP->rtp_data_r_index = pRTP->rtp_data_w_index = 0;
			}
		}
	
		if((pRTP->rtp_data_w_index + payload_data_len) > (RTP_DATA_BUFFER - RTP_FRAME_DATA_LEN))
		{
			UACABNOR("rtp buffer overflow! buffer:%d/%d/%d payload:%d",
				pRTP->rtp_data_r_index, pRTP->rtp_data_w_index, RTP_DATA_BUFFER,
				payload_data_len);
		}
		else
		{
			dave_memcpy(&pRTP->rtp_data_buffer[pRTP->rtp_data_w_index], payload_data_ptr, payload_data_len);
			pRTP->rtp_data_w_index += payload_data_len;
			if((payload_data_len % RTP_FRAME_DATA_LEN) != 0)
			{
				ub padding_len = RTP_FRAME_DATA_LEN - (payload_data_len % RTP_FRAME_DATA_LEN);
				dave_memcpy(&pRTP->rtp_data_buffer[pRTP->rtp_data_w_index], _mte_bag, padding_len);
				pRTP->rtp_data_w_index += padding_len;
			}
		}
	})
}

static void
_uac_rtp_data(MSGBODY *msg)
{
	RTPDataRsp *pRsp = (RTPDataRsp *)(msg->msg_body);
	UACRTP *pRTP = uac_rtp_call_id_to_rtp(pRsp->call_id);

	if(pRTP != NULL)
	{
		_uac_rtp_data_buffer_set(pRTP, pRsp->ssrc, ms8(pRsp->payload_data), mlen(pRsp->payload_data));
	}
	else
	{
		UACLOG("call:%s %s->%s can't find!", pRsp->call_id, pRsp->call_from, pRsp->call_to);
	}

	dave_free(pRsp->payload_data);
}

#endif

static void
_uac_rtp_reset(MSGBODY *msg)
{
	RTPResetReq *pReq = (RTPResetReq *)(msg->msg_body);
	RTPResetRsp *pRsp = thread_msg(pRsp);
	UACRTP *pRTP = uac_rtp_call_id_to_rtp(pReq->call_id);

	if(pRTP != NULL)
	{
		SAFECODEv1(pRTP->rtp_data_pv, {
			pRTP->rtp_data_r_index = pRTP->rtp_data_w_index = 0;
		});
	}

	dave_strcpy(pRsp->call_id, pReq->call_id, sizeof(pRsp->call_id));
	dave_strcpy(pRsp->call_from, pReq->call_from, sizeof(pRsp->call_from));
	dave_strcpy(pRsp->call_to, pReq->call_to, sizeof(pRsp->call_to));
	pRsp->ptr = pReq->ptr;

	id_msg(msg->msg_src, RTP_RESET_RSP, pRsp);
}

// =====================================================================

void
uac_rtp_msg_init(void)
{
#ifndef ENABLE_RTP_ECHO
 	reg_msg(RTP_DATA_RSP, _uac_rtp_data);
#endif
	reg_msg(RTP_RESET_REQ, _uac_rtp_reset);
}

void
uac_rtp_msg_exit(void)
{
#ifndef ENABLE_RTP_ECHO
	unreg_msg(RTP_DATA_RSP);
#endif
	unreg_msg(RTP_RESET_REQ);
}

void
uac_rtp_msg_start(s8 *call_id, s8 *call_from, s8 *call_to)
{
	RTPStartReq *pReq = thread_msg(pReq);
	RTPStartRsp rsp;

	dave_strcpy(pReq->call_id, call_id, sizeof(pReq->call_id));
	dave_strcpy(pReq->call_from, call_from, sizeof(pReq->call_from));
	dave_strcpy(pReq->call_to, call_to, sizeof(pReq->call_to));

	sync_msg(thread_id("BOX"), RTP_START_REQ, pReq, RTP_START_RSP, &rsp);
}

void
uac_rtp_msg_stop(s8 *call_id, s8 *call_from, s8 *call_to)
{
	RTPStopReq *pReq = thread_msg(pReq);
	RTPStopRsp rsp;

	dave_strcpy(pReq->call_id, call_id, sizeof(pReq->call_id));
	dave_strcpy(pReq->call_from, call_from, sizeof(pReq->call_from));
	dave_strcpy(pReq->call_to, call_to, sizeof(pReq->call_to));

	sync_msg(thread_id("BOX"), RTP_STOP_REQ, pReq, RTP_STOP_RSP, &rsp);
}

RTPDATA
uac_rtp_msg_data(
	UACRTP *pRTP,
	u8 payload_type,
	u16 sequence_number, u32 timestamp, u32 ssrc,
	s8 *payload_ptr, ub payload_len)
{
	RTPDataReq *pReq = thread_msg(pReq);

	dave_strcpy(pReq->call_id, pRTP->call_id, sizeof(pReq->call_id));
	dave_strcpy(pReq->call_from, pRTP->call_from, sizeof(pReq->call_from));
	dave_strcpy(pReq->call_to, pRTP->call_to, sizeof(pReq->call_to));
	pReq->payload_type = payload_type;
	pReq->sequence_number = sequence_number;
	pReq->timestamp = timestamp;
	pReq->ssrc = ssrc;
	pReq->payload_data = t_a2b_bin_to_mbuf(payload_ptr, payload_len);
	pReq->ptr = pReq;

	name_msg("BOX", RTP_DATA_REQ, pReq);

#ifdef ENABLE_RTP_ECHO
	return _uac_rtp_data_echo(pRTP, payload_type, sequence_number, timestamp, ssrc, payload_ptr, payload_len);
#else
	return _uac_rtp_data_buffer_get(pRTP);
#endif
}

