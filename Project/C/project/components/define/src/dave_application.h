/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_APPLICATION_H__
#define __DAVE_APPLICATION_H__
#include "dave_error_code.h"
#include "dave_define.h"

/* for APPMSG_MCARD_REQ message */
typedef struct {
	GPSLocation location;
	ub radius;
	void *ptr;
} AppMsgMCardReq;

/* for APPMSG_MCARD_RSP message */
typedef struct {
	ErrCode ret;
	MCard mcard;
	void *ptr;
} AppMsgMCardRsp;

/* for APPMSG_TALK_MCARD_REQ message */
typedef struct {
	s8 url[DAVE_URL_LEN];
	MCard mcard;
	void *ptr;
} AppMsgTalkMCardReq;

/* for APPMSG_TALK_MCARD_RSP message */
typedef struct {
	ErrCode ret;
	MCard mcard;
	void *ptr;
} AppMsgTalkMCardRsp;

/* for APPMSG_FUNCTION_REGISTER_REQ message */
typedef struct {
	s8 thread_name[DAVE_THREAD_NAME_LEN];
	ub function_id;
	void *ptr;
} AppMsgFunctionRegReq;

/* for APPMSG_FUNCTION_REGISTER_RSP message */
typedef struct {
	ErrCode ret;
	s8 thread_name[DAVE_THREAD_NAME_LEN];
	ub function_id;
	void *ptr;
} AppMsgFunctionRegRsp;

/* for APPMSG_FUNCTION_UNREGISTER_REQ message */
typedef struct {
	s8 thread_name[DAVE_THREAD_NAME_LEN];
	ub function_id;
	void *ptr;
} AppMsgFunctionUnRegReq;

/* for APPMSG_FUNCTION_UNREGISTER_RSP message */
typedef struct {
	ErrCode ret;
	s8 thread_name[DAVE_THREAD_NAME_LEN];
	ub function_id;
	void *ptr;
} AppMsgFunctionUnRegRsp;

#endif

