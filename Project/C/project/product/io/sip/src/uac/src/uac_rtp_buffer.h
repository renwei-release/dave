/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UAC_RTP_BUFFER_H__
#define __UAC_RTP_BUFFER_H__

void uac_rtp_buffer_init(UACRTPBuffer *pBuffer);
void uac_rtp_buffer_exit(UACRTPBuffer *pBuffer);

MBUF * uac_rtp_buffer(
	u16 *send_sequence_number,
	UACRTPBuffer *pBuffer,
	u8 payload_type, u16 sequence_number, u32 timestamp, u32 ssrc, s8 *payload_ptr, ub payload_len);

#endif

