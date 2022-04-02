/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __HTTP_MSG_H__
#define __HTTP_MSG_H__
#include "http_param.h"

/* for HTTPMSG_LISTEN_REQ message */
typedef struct {
	ub listen_port;
	HTTPMathcRule rule;
	HTTPListenType type;
	s8 path[DAVE_PATH_LEN];
	void *ptr;
} HTTPListenReq;

/* for HTTPMSG_LISTEN_RSP message */
typedef struct {
	ErrCode ret;
	ub listen_port;
	s8 path[DAVE_PATH_LEN];
	void *ptr;
} HTTPListenRsp;

/* for HTTPMSG_CLOSE_REQ message */
typedef struct {
	ub listen_port;
	s8 path[DAVE_PATH_LEN];
	void *ptr;
} HTTPCloseReq;

/* for HTTPMSG_CLOSE_RSP message */
typedef struct {
	ErrCode ret;
	ub listen_port;
	s8 path[DAVE_PATH_LEN];
	void *ptr;
} HTTPCloseRsp;

/* for HTTPMSG_RECV_REQ message */
typedef struct {
	ub listen_port;
	s8 remote_address[DAVE_URL_LEN];
	ub remote_port;
	HttpMethod method;
	HttpKeyValue head[DAVE_HTTP_HEAD_MAX];
	HttpContentType content_type;
	MBUF *content;
	ub local_creat_time;
	void *ptr;
} HTTPRecvReq;

/* for HTTPMSG_RECV_RSP message */
typedef struct {
	ErrCode ret;
	HttpContentType content_type;
	MBUF *content;
	ub local_creat_time;
	void *ptr;
} HTTPRecvRsp;

#endif

