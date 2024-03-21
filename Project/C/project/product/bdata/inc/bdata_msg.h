/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __BDATA_MSG_H__
#define __BDATA_MSG_H__
#include "dave_base.h"

#define BDATA_THREAD_NAME "bdata"

typedef enum {
	BDataLogLevel_normal = 0,
	BDataLogLevel_report,
	BDataLogLevel_max = 0x1fffffffffffffff
} BDataLogLevel;

/* for BDATA_LOG_REQ message */
typedef struct {
	BDataLogLevel level;

	s8 version[DAVE_VERNO_STR_LEN];
	s8 sub_flag[DAVE_NORMAL_STR_LEN];
	DateStruct local_date;

	s8 fun[DAVE_NORMAL_STR_LEN];
	ub line;

	s8 host_name[DAVE_NORMAL_NAME_LEN];
	u8 host_mac[DAVE_MAC_ADDR_LEN];
	u8 host_ipv4[DAVE_IP_V4_ADDR_LEN];
	u8 host_ipv6[DAVE_IP_V6_ADDR_LEN];

	MBUF *log_data;
	void *ptr;
} BDataLogReq;

/* for BDATA_LOG_RSP message */
typedef struct {
	RetCode ret;
	void *ptr;
} BDataLogRsp;

#endif

