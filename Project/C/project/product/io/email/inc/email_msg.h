/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __EMAIL_MSG_H__
#define __EMAIL_MSG_H__

#define EMAIL_THREAD_NAME "email"

/* for EMAIL_SEND_REQ message */
typedef struct {
	s8 subject[1024];
	MBUF *content;
	void *ptr;
} EmailSendReq;

/* for EMAIL_SEND_RSP message */
typedef struct {
	RetCode ret;
	void *ptr;
} EmailSendRsp;

#endif

