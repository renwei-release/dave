/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RTC_MSG_H__
#define __RTC_MSG_H__

#define RTC_THREAD_NAME "rtc"

/* for RTC_REQ message */
typedef struct {
	s8 token[512];
	s8 src[128];
	s8 dst[128];
	MBUF *content;
	s8 format[64];
	void *ptr;
} RTCReq;

/* for RTC_RSP message */
typedef struct {
	s8 token[512];
	s8 src[128];
	s8 dst[128];
	MBUF *content;
	s8 format[64];
	void *ptr;
} RTCRsp;

/* for RTC_REG_REQ message */
typedef struct {
	s8 id[128];
	void *ptr;
} RTCRegReq;

/* for RTC_REG_RSP message */
typedef struct {
	s8 id[128];
	s8 token[512];
	void *ptr;
} RTCRegRsp;

/* for RTC_UNREG_REQ message */
typedef struct {
	s8 id[128];
	s8 token[512];
	void *ptr;
} RTCUnregReq;

/* for RTC_UNREG_RSP message */
typedef struct {
	s8 id[128];
	s8 token[512];
	void *ptr;
} RTCUnregRsp;

#endif

