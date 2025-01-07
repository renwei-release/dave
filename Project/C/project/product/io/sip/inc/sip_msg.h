/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SIP_MSG_H__
#define __SIP_MSG_H__

#define UAC_THREAD_NAME "uac"

/* for RTP_START_REQ message */
typedef struct {
	s8 call_id[128];
	s8 call_from[128];
	s8 call_to[128];
	void *ptr;
} RTPStartReq;

/* for RTP_START_RSP message */
typedef struct {
	s8 call_id[128];
	s8 call_from[128];
	s8 call_to[128];
	void *ptr;
} RTPStartRsp;

/* for RTP_STOP_REQ message */
typedef struct {
	s8 call_id[128];
	s8 call_from[128];
	s8 call_to[128];
	void *ptr;
} RTPStopReq;

/* for RTP_STOP_RSP message */
typedef struct {
	s8 call_id[128];
	s8 call_from[128];
	s8 call_to[128];
	void *ptr;
} RTPStopRsp;

/* for RTP_DATA_REQ message */
typedef struct {
	s8 call_id[128];
	s8 call_from[128];
	s8 call_to[128];
	u8 payload_type;
	u16 sequence_number;
	u32 timestamp;
	u32 ssrc;
	MBUF *payload_data;
	void *ptr;
} RTPDataReq;

/* for RTP_DATA_RSP message */
typedef struct {
	s8 call_id[128];
	s8 call_from[128];
	s8 call_to[128];
	u8 payload_type;
	u16 sequence_number;
	u32 timestamp;
	u32 ssrc;
	MBUF *payload_data;
	void *ptr;
} RTPDataRsp;

/* for SIP_CALL_REQ message */
typedef struct {
	s8 phone_number[512];
	void *ptr;
} SIPCallReq;

/* for SIP_CALL_RSP message */
typedef struct {
	RetCode ret;
	s8 call_id[128];
	s8 phone_number[512];
	void *ptr;
} SIPCallRsp;

/* for SIP_BYE_REQ message */
typedef struct {
	s8 call_id[128];
	s8 phone_number[512];
	void *ptr;
} SIPByeReq;

/* for SIP_BYE_RSP message */
typedef struct {
	RetCode ret;
	s8 call_id[128];
	s8 phone_number[512];
	void *ptr;
} SIPByeRsp;

#endif

