/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UAC_RTP_MSG_H__
#define __UAC_RTP_MSG_H__
#include "dave_osip.h"

void uac_rtp_msg_init(void);
void uac_rtp_msg_exit(void);

typedef struct {
	u8 payload_type;
	u16 sequence_number;
	u32 timestamp;
	u32 ssrc;
	MBUF *payload_data;
} RTPDATA;

void uac_rtp_msg_start(s8 *call_id, s8 *call_from, s8 *call_to);
void uac_rtp_msg_stop(s8 *call_id, s8 *call_from, s8 *call_to);

RTPDATA uac_rtp_msg_data(
	UACRTP *pRTP,
	u8 payload_type,
	u16 sequence_number, u32 timestamp, u32 ssrc,
	s8 *payload_ptr, ub payload_len);

#endif

