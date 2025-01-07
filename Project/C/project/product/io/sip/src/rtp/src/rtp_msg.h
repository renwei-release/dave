/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RTP_MSG_H__
#define __RTP_MSG_H__
#include "dave_osip.h"

void rtp_msg_init(void);
void rtp_msg_exit(void);

typedef struct {
	u8 payload_type;
	u16 sequence_number;
	u32 timestamp;
	u32 ssrc;
	MBUF *payload_data;
} RTPDATA;

void rtp_msg_start(RTP *pRTP);
void rtp_msg_stop(RTP *pRTP);

RTPDATA rtp_msg_data_recv(RTP *pRTP, u8 payload_type, u16 sequence_number, u32 timestamp, u32 ssrc, s8 *payload_ptr, ub payload_len);

void rtp_msg_data_send(void *rtp, u8 payload_type, u16 sequence_number, u32 timestamp, u32 ssrc, s8 *payload_ptr, ub payload_len);

#endif

