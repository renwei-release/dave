/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_STORE_H__
#define __DAVE_STORE_H__
#include "dave_base.h"
#include "dave_3rdparty.h"
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

static inline s8 *
STORELOAD(StoreSqlRet ret, ub row, ub column)
{
	ub array_length, row_length;
	void *pTheRow;

	if(ret.pJson == NULL)
		return NULL;

	array_length = dave_json_get_array_length(ret.pJson);
	if(row >= array_length)
	{
		return NULL;
	}

	pTheRow = dave_json_c_get_array_idx(ret.pJson, row);
	if(pTheRow == NULL)
	{
		return NULL;
	}

	row_length = dave_json_get_array_length(pTheRow);
	if(column >= row_length)
	{
		return NULL;
	}

	return dave_json_c_array_get_str(pTheRow, column, NULL);
}

static inline s8 *
STORELOADstr(StoreSqlRet ret, ub column)
{
	return STORELOAD(ret, 0, column);
}

static inline sb
STORELOADsb(StoreSqlRet ret, ub column)
{
	s8 *sb_str = STORELOAD(ret, 0, column);

	if(sb_str == NULL)
		return 0;

	return stringdigital(sb_str);
}

#endif

