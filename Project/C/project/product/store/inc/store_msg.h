/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __STORE_MSG_H__
#define __STORE_MSG_H__
#include "dave_base.h"

#define STORE_THREAD_NAME "store"

/* for STORE_MYSQL_REQ message */
typedef struct {
	MBUF *sql;
	void *ptr;
} StoreMysqlReq;

/* for STORE_MYSQL_RSP message */
typedef struct {
	RetCode ret;
	MBUF *data;
	void *ptr;
} StoreMysqlRsp;

#endif

