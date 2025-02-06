/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RTC_MAIN_H__
#define __RTC_MAIN_H__
#include "dave_base.h"
#include "dave_tools.h"
#include "rtc_param.h"

typedef struct {
	RTCClientType type;
	s32 socket;
	void *wsi;

	s8 token[512];

	s8 *tlv_buffer_ptr;
	ub tlv_buffer_len;
	ub tlv_buffer_w_index;
	ub tlv_buffer_r_index;

	TLock event_pv;
} RTCClient;

typedef struct {
	s8 token[512];
	s8 terminal_type[128];
	s8 terminal_id[128];

	ThreadId src_id;
	s8 src_gid[DAVE_GLOBALLY_IDENTIFIER_LEN];
	s8 src_name[DAVE_THREAD_NAME_LEN];

	s8 data_format[64];
	ub data_length;
	s8 data_buffer[TOKEN_DATA_BUFFER_MAX];
	u16 recv_serial;
	u16 send_serial;
	u16 local_serial;
	void *pre_buffer_kv;

	RTCClient *pClient;

	sb lift_counter;
} RTCToken;

void rtc_main_init(void);
void rtc_main_exit(void);

void rtc_main_recv(RTCClient *pClient);
void rtc_main_send(RTCClient *pClient, MBUF *data);

#endif

