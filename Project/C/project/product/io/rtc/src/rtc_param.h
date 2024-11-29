/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RTC_PARAM_H__
#define __RTC_PARAM_H__
#include "dave_base.h"
#include "tlv_tag.h"

#define RTC_CLIENT_MAX DAVE_SERVER_SUPPORT_SOCKET_MAX
#define TLV_BUFFER_LENGTH_MAX 1 * 1024 * 1024
#define TOKEN_APP_DATA_BUFFER_MAX 512 * 1024

typedef struct {
	s32 socket;

	s8 token[512];

	s8 *tlv_buffer_ptr;
	ub tlv_buffer_len;
	ub tlv_buffer_w_index;
	ub tlv_buffer_r_index;

	TLock event_pv;
} RTCClient;

typedef struct {
	s8 token[512];
	s8 id[128];

	ThreadId src_id;
	s8 src_gid[DAVE_GLOBALLY_IDENTIFIER_LEN];
	s8 src_name[DAVE_THREAD_NAME_LEN];

	ub app_data_length;
	s8 app_data_buffer[TOKEN_APP_DATA_BUFFER_MAX];
	s8 app_format[64];

	RTCClient *pClient;
} RTCToken;

#endif

