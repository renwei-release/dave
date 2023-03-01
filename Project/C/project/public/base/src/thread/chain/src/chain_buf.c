/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "base_define.h"
#include "base_tools.h"
#include "base_lock.h"
#include "thread_chain.h"
#include "thread_log.h"

#define CHAIN_BUF_VERSION 2
#define CHAIN_BODY_MAX_LEN 48

typedef struct {
	MBUF *chain_data;
	void *next;
} ChainBuf;

static ChainBuf *_chain_buf_head = NULL;
static ChainBuf *_chain_buf_tail = NULL;

static inline ub
_chain_buf_build_chain(s8 *data_ptr, ub data_len, ThreadChain *pChain, u16 chain_version)
{
	ub data_index = 0;

	dave_byte_8(data_ptr[data_index++], data_ptr[data_index++], chain_version);
	dave_byte_32_8(data_ptr[data_index++], data_ptr[data_index++], data_ptr[data_index++], data_ptr[data_index++], sizeof(ThreadChain));
	data_index += dave_memcpy(&data_ptr[data_index], pChain, sizeof(ThreadChain));

	return data_index;
}

static inline ub
_chain_buf_build_router(s8 *data_ptr, ub data_len, ThreadRouter *pRouter)
{
	ub data_index = 0, router_index;

	if((pRouter == NULL) || (pRouter->uid[0] == '\0'))
	{
		dave_byte_8(data_ptr[data_index++], data_ptr[data_index++], 0);
		return data_index;
	}

	dave_byte_8(data_ptr[data_index++], data_ptr[data_index++], DAVE_ROUTER_UID_LEN);
	data_index += dave_memcpy(&data_ptr[data_index], pRouter->uid, DAVE_ROUTER_UID_LEN);

	dave_byte_32_8(data_ptr[data_index++], data_ptr[data_index++], data_ptr[data_index++], data_ptr[data_index++], pRouter->router_number);
	dave_byte_32_8(data_ptr[data_index++], data_ptr[data_index++], data_ptr[data_index++], data_ptr[data_index++], pRouter->current_router_index);

	for(router_index=0; router_index<DAVE_ROUTER_SUB_MAX; router_index++)
	{
		if(router_index >= pRouter->router_number)
			break;

		dave_byte_8(data_ptr[data_index++], data_ptr[data_index++], DAVE_THREAD_NAME_LEN);
		data_index += dave_memcpy(&data_ptr[data_index], pRouter->sub_router[router_index].thread, DAVE_THREAD_NAME_LEN);
		if(pRouter->sub_router[router_index].gid[0] == '\0')
		{
			dave_byte_8(data_ptr[data_index++], data_ptr[data_index++], 0);
		}
		else
		{
			dave_byte_8(data_ptr[data_index++], data_ptr[data_index++], DAVE_GLOBALLY_IDENTIFIER_LEN);
			data_index += dave_memcpy(&data_ptr[data_index], pRouter->sub_router[router_index].gid, DAVE_GLOBALLY_IDENTIFIER_LEN);
		}
	}

	return data_index;
}

static inline ub
_chain_buf_build_msg(s8 *data_ptr, ub data_len, ub msg_id, ub msg_len, void *msg_body, ThreadChain *pChain)
{
	ub data_index = 0;

	dave_byte_32_8(data_ptr[data_index++], data_ptr[data_index++], data_ptr[data_index++], data_ptr[data_index++], msg_id);

	if((pChain->type == ChainType_execution)
		|| (pChain->type == ChainType_coroutine))
	{
		if(msg_len > CHAIN_BODY_MAX_LEN)
		{
			msg_len = CHAIN_BODY_MAX_LEN;
		}

		dave_byte_32_8(data_ptr[data_index++], data_ptr[data_index++], data_ptr[data_index++], data_ptr[data_index++], msg_len);
		data_index += dave_memcpy(&data_ptr[data_index], msg_body, msg_len);
	}
	else
	{
		dave_byte_32_8(data_ptr[data_index++], data_ptr[data_index++], data_ptr[data_index++], data_ptr[data_index++], 0);
	}

	return data_index;
}

static inline ChainBuf *
_chain_buf_malloc(
	ThreadChain *pChain, ThreadRouter *pRouter,
	ub msg_id, ub msg_len, void *msg_body)
{
	ChainBuf *pBuf = dave_malloc(sizeof(ChainBuf));
	u16 chain_version = CHAIN_BUF_VERSION;

	pBuf->chain_data = dave_mmalloc(sizeof(ThreadChain) + sizeof(ThreadRouter) + msg_len + 128);
	pBuf->next = NULL;

	s8 *data_ptr = ms8(pBuf->chain_data);
	ub data_len = mlen(pBuf->chain_data);
	ub data_index = 0;

	data_index += _chain_buf_build_chain(&data_ptr[data_index], data_len-data_index, pChain, chain_version);
	data_index += _chain_buf_build_router(&data_ptr[data_index], data_len-data_index, pRouter);
	data_index += _chain_buf_build_msg(&data_ptr[data_index], data_len-data_index, msg_id, msg_len, msg_body, pChain);

	pBuf->chain_data->tot_len = pBuf->chain_data->len = data_index;

	return pBuf;
}

static inline void
_chain_buf_free(ChainBuf *pBuf)
{
	if(pBuf->chain_data != NULL)
		dave_mfree(pBuf->chain_data);
	dave_free(pBuf);
}

// =====================================================================

void
chain_buf_init(void)
{
	_chain_buf_head = _chain_buf_tail = NULL;
}

void
chain_buf_exit(void)
{
	ChainBuf *pBuf;

	base_lock();
	pBuf = _chain_buf_head;
	while(pBuf != NULL)
	{
		_chain_buf_free(pBuf);
		pBuf = pBuf->next;
	}
	base_unlock();
}

void
chain_buf_set(
	ThreadChain *pChain, ThreadRouter *pRouter,
	ub msg_id, ub msg_len, void *msg_body)
{
	ChainBuf *pBuf = _chain_buf_malloc(pChain, pRouter, msg_id, msg_len, msg_body);

	base_lock();
	if(_chain_buf_head == NULL)
	{
		_chain_buf_head = _chain_buf_tail = pBuf;
	}
	else
	{
		_chain_buf_tail->next = pBuf;
		_chain_buf_tail = pBuf;
	}
	base_unlock();
}

MBUF *
chain_buf_get(void)
{
	ChainBuf *pBuf;
	MBUF *chain_data;

	base_lock();
	pBuf = _chain_buf_head;
	if(_chain_buf_head != NULL)
	{
		_chain_buf_head = _chain_buf_head->next;
		if(_chain_buf_head == NULL)
		{
			_chain_buf_tail = NULL;
		}
	}
	base_unlock();

	if(pBuf != NULL)
	{
		chain_data = pBuf->chain_data;
		pBuf->chain_data = NULL;
		_chain_buf_free(pBuf);
	}
	else
	{
		chain_data = NULL;
	}

	return chain_data;
}

#endif

