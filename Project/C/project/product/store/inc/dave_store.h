/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_STORE_H__
#define __DAVE_STORE_H__
#include "dave_base.h"
#include "store_msg.h"

typedef struct {
	RetCode ret;
	void *pJson;
} StoreSqlRet;

static inline StoreSqlRet
STORESQL(const char *sql, ...)
{
	StoreMysqlReq *pReq = thread_msg(pReq);
	StoreMysqlRsp *pRsp;
	va_list list_args;
	StoreSqlRet ret;

	pReq->sql = dave_mmalloc(10240);
	pReq->ptr = pReq;

	va_start(list_args, sql);
	pReq->sql->len = pReq->sql->tot_len = vsnprintf(dave_mptr(pReq->sql), dave_mlen(pReq->sql), sql, list_args);
	va_end(list_args);

	pRsp = name_co(STORE_THREAD_NAME, STORE_MYSQL_REQ, pReq, STORE_MYSQL_RSP);
	if(pRsp == NULL)
	{
		ret.ret = RetCode_Request_failed;
		ret.pJson = NULL;
	}
	else
	{
		ret.ret = pRsp->ret;
		ret.pJson = t_a2b_mbuf_to_json(pRsp->data);

		dave_mfree(pRsp->data);
	}

	return ret;
}

#endif

