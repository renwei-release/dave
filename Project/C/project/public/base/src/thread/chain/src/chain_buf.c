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

#define CHAIN_BODY_MAX_LEN 48

typedef struct {
	MBUF *chain_data;
	void *next;
} ChainBuf;

static ChainBuf *_chain_buf_head = NULL;
static ChainBuf *_chain_buf_tail = NULL;

static ChainBuf *
_chain_buf_malloc(
	ThreadChain *pChain,
	ub msg_id, ub msg_len, void *msg_body)
{
	ChainBuf *pBuf = dave_malloc(sizeof(ChainBuf));
	u16 chain_version = 1;

	pBuf->chain_data = dave_mmalloc(sizeof(ThreadChain) + msg_len + 128);
	pBuf->next = NULL;

	s8 *data_ptr = ms8ptr(pBuf->chain_data);
	ub data_index;

	data_index = 0;

	dave_byte_8(data_ptr[data_index++], data_ptr[data_index++], chain_version);
	dave_byte_32_8(data_ptr[data_index++], data_ptr[data_index++], data_ptr[data_index++], data_ptr[data_index++], sizeof(ThreadChain));
	data_index += dave_memcpy(&data_ptr[data_index], pChain, sizeof(ThreadChain));

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

	pBuf->chain_data->tot_len = pBuf->chain_data->len = data_index;

	return pBuf;
}

static void
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
	ThreadChain *pChain,
	ub msg_id, ub msg_len, void *msg_body)
{
	ChainBuf *pBuf = _chain_buf_malloc(pChain, msg_id, msg_len, msg_body);

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

