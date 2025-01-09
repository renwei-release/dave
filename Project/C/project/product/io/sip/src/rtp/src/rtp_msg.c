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
#include "rtp_msg.h"
#include "rtp_log.h"

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
_rtp_data_echo(
	RTP *pRTP,
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
_rtp_data_buffer_get(RTP *pRTP)
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

#endif

static void
_rtp_data_buffer_set(RTP *pRTP, u32 ssrc, s8 *payload_data_ptr, ub payload_data_len)
{
	SAFECODEv1(pRTP->rtp_data_pv, {
		if(pRTP->rtp_data_w_index == 0)
		{
			pRTP->current_buffer_ssrc = ssrc;

			RTPLOG("ssrc:%d data_len:%d", ssrc, payload_data_len)
		}
		else
		{
			if(pRTP->current_buffer_ssrc != ssrc)
			{
				RTPLOG("It is possible that the conversation was interrupted by the user! %s->%s ssrc:%d/%d",
					pRTP->call_from, pRTP->call_to,
					pRTP->current_buffer_ssrc, ssrc);

				pRTP->current_buffer_ssrc = ssrc;
				pRTP->rtp_data_r_index = pRTP->rtp_data_w_index = 0;
			}
		}
	
		if((pRTP->rtp_data_w_index + payload_data_len) > (RTP_DATA_BUFFER - RTP_FRAME_DATA_LEN))
		{
			RTPABNOR("rtp buffer overflow! buffer:%d/%d/%d payload:%d",
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

// =====================================================================

void
rtp_msg_start(RTP *pRTP)
{
	if(pRTP->data_start == NULL)
	{
		RTPLOG("data_start is NULL");
	}
	else
	{
		pRTP->data_start(pRTP);
		pRTP->data_start = NULL;
	}
}

void
rtp_msg_stop(RTP *pRTP)
{
	if(pRTP->data_stop == NULL)
	{
		RTPLOG("data_stop is NULL");
	}
	else
	{
		pRTP->data_stop(pRTP);
		pRTP->data_stop = NULL;
	}
}

RTPDATA
rtp_msg_data_recv(
	RTP *pRTP,
	u8 payload_type,
	u16 sequence_number, u32 timestamp, u32 ssrc,
	s8 *payload_ptr, ub payload_len)
{
	pRTP->data_recv((void *)pRTP, payload_type, sequence_number, timestamp, ssrc, payload_ptr, payload_len);

#ifdef ENABLE_RTP_ECHO
	return _rtp_data_echo(pRTP, payload_type, sequence_number, timestamp, ssrc, payload_ptr, payload_len);
#else
	return _rtp_data_buffer_get(pRTP);
#endif
}

void
rtp_msg_data_send(void *rtp, u8 payload_type, u16 sequence_number, u32 timestamp, u32 ssrc, s8 *payload_ptr, ub payload_len)
{
	RTP *pRTP = (RTP *)rtp;

	_rtp_data_buffer_set(pRTP, ssrc, payload_ptr, payload_len);
}

