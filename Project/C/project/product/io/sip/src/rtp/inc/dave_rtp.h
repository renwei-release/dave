/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_RTP_H__
#define __DAVE_RTP_H__

void dave_rtp_init(void);
void dave_rtp_exit(void);

typedef void(* rtp_data_start)(void *pRTP);
typedef void(* rtp_data_stop)(void *pRTP);
typedef void(* rtp_data_recv)(void *pRTP, u8 payload_type, u16 sequence_number, u32 timestamp, u32 ssrc, s8 *payload_ptr, ub payload_len);
typedef void(* rtp_data_send)(void *pRTP, u8 payload_type, u16 sequence_number, u32 timestamp, u32 ssrc, s8 *payload_ptr, ub payload_len);

#define RTP_HEADER_LEN 12
#define RTP_DATA_BUFFER 512 *1024

typedef struct {
    u8  version:2;
    u8  padding:1;
    u8  extension:1;
    u8  csrc_count:4;
    u8  marker:1;
    u8  payload_type:7;
    u16 sequence_number;
    u32 timestamp;
    u32 ssrc;
    u32 csrc[15];
} RTPHeadr;

typedef struct {
	RTPHeadr header;
	ub payload_len;
	s8 payload_ptr[256];
	void *next;
} RTPPkg;

typedef struct {
	s32 recv_rtp_socket;
	s32 send_rtp_socket;

	s8 local_rtp_ip[16];
	s8 local_rtp_port[16];
	s8 remote_rtp_ip[16];
	s8 remote_rtp_port[16];

	u8 u8_local_rtp_ip[4];
	u16 u16_local_rtp_port;
	u8 u8_remote_rtp_ip[4];
	u16 u16_remote_rtp_port;	

	s8 call_id[128];
	s8 call_from[128];
	s8 call_to[128];

	TLock rtp_data_pv;
	u8 payload_type;
	u16 sequence_number;
	u32 timestamp;
	u32 ssrc;
	ub rtp_data_r_index;
	ub rtp_data_w_index;
	u32 current_buffer_ssrc;
	u8 rtp_data_buffer[RTP_DATA_BUFFER];

	rtp_data_start data_start;
	rtp_data_stop data_stop;
	rtp_data_recv data_recv;
	rtp_data_send data_send;

	void *call;
} RTP;

RTP * dave_rtp_creat(
	s8 *local_rtp_ip, s8 *local_rtp_port,
	rtp_data_start data_start, rtp_data_stop data_stop, rtp_data_recv data_recv,
	void *call);

void dave_rtp_release(RTP *pRTP);

void dave_rtp_call_id_build(RTP *pRTP, s8 *call_id, s8 *call_from, s8 *call_to);

void dave_rtp_send_build(RTP *pRTP, s8 *remote_rtp_ip, s8 *remote_rtp_port);

dave_bool dave_rtp_recv(SocketRead *pRead);

void dave_rtp_send(RTP *pRTP, u8 payload_type, u16 sequence_number, u32 timestamp, u32 ssrc, MBUF *payload_data);

#endif

