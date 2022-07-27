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
#include "thread_chain.h"
#include "thread_tools.h"
#include "thread_log.h"
#include "chain_id.h"
#include "chain_buf.h"
#include "chain_cfg.h"

static inline void
_thread_chain_clean(ThreadChain *pChain)
{
	if(pChain == NULL)
		return;

	pChain->type = ChainType_none;

	pChain->chain_id[0] = '\0';
	pChain->chain_counter = 0;
	pChain->generation = 0;

	pChain->send_time = pChain->recv_time = 0;

	pChain->src_gid[0] = '\0';
	pChain->dst_gid[0] = '\0';

	pChain->src_thread[0] = '\0';
	pChain->src_thread[0] = '\0';

	pChain->msg_id = MSGID_RESERVED;
	pChain->msg_serial = 0;
}

static inline void
_thread_chain_build_new(ThreadChain *pMsgChain)
{
	_thread_chain_clean(pMsgChain);

	chain_id(pMsgChain->chain_id, sizeof(pMsgChain->chain_id));
	pMsgChain->send_time = dave_os_time_us();
	pMsgChain->msg_serial = chain_msg_serial();
}

static inline void
_thread_chain_build_copy(ThreadChain *pDstChain, ThreadChain *pSrcChain)
{
	_thread_chain_clean(pDstChain);

	dave_strcpy(pDstChain->chain_id, pSrcChain->chain_id, sizeof(pDstChain->chain_id));
	pDstChain->generation = pSrcChain->generation + 1;
	pDstChain->send_time = dave_os_time_us();
	pDstChain->msg_serial = chain_msg_serial();
}

static inline void
_thread_chain_run_copy(ThreadChain *pDstChain, ThreadChain *pSrcChain)
{
	_thread_chain_clean(pDstChain);

	dave_strcpy(pDstChain->chain_id, pSrcChain->chain_id, sizeof(pDstChain->chain_id));
	pDstChain->generation = pSrcChain->generation;
}

static inline void
_thread_chain_insert_chain(
	ThreadChain *pChain,
	ChainType type,
	s8 *src_gid, s8 *dst_gid,
	ThreadId msg_src, ThreadId msg_dst,
	ub msg_id)
{
	pChain->type = type;
	pChain->chain_counter = chain_counter();

	pChain->recv_time = dave_os_time_us();

	dave_strcpy(pChain->src_gid, src_gid, sizeof(pChain->src_gid));
	dave_strcpy(pChain->dst_gid, dst_gid, sizeof(pChain->dst_gid));

	dave_strcpy(pChain->src_thread, thread_id_to_name(msg_src), sizeof(pChain->src_thread));
	dave_strcpy(pChain->dst_thread, thread_id_to_name(msg_dst), sizeof(pChain->dst_thread));

	pChain->msg_id = msg_id;
}

// =====================================================================

void
thread_chain_init(void)
{
	chain_cfg_reset();

	chain_buf_init();

	chain_id_reset();
}

void
thread_chain_exit(void)
{
	chain_buf_exit();
}

ThreadChain *
thread_chain_malloc(void)
{
	return (ThreadChain *)dave_malloc(sizeof(ThreadChain));
}

void
thread_chain_free(ThreadChain *pChain)
{
	dave_free(pChain);
}

void
thread_chain_reset(ThreadChain *pChain)
{
	dave_memset(pChain, 0x00, sizeof(ThreadChain));

	_thread_chain_clean(pChain);
}

dave_bool
thread_chain_enable(ThreadId msg_src, ThreadId msg_dst, ub msg_id)
{
	if((thread_is_remote(msg_src) == dave_false)
		&& (thread_id_to_attrib(msg_src) != REMOTE_TASK_ATTRIB)
		&& (thread_is_remote(msg_dst) == dave_false)
		&& (thread_id_to_attrib(msg_dst) != REMOTE_TASK_ATTRIB))
	{
		return dave_false;
	}

	THREADDEBUG("thread_id:%lx msg_id:%s %s",
		thread_id, msgstr(msg_id),
		chain_enable()==dave_true?"enable":"disable");

	return chain_enable();
}

void
thread_chain_build_msg(
	ThreadChain *pMsgChain,
	ThreadId msg_src, ThreadId msg_dst,
	ub msg_id)
{
	ThreadChain *pThreadChain;

	pThreadChain = thread_current_chain();
	if(pThreadChain == NULL)
	{
		THREADABNOR("pThreadChain is NULL! %x/%s->%x/%s:%s",
			msg_src, thread_id_to_name(msg_src), msg_dst, thread_id_to_name(msg_dst),
			msgstr(msg_id));
		dave_memset(pMsgChain, 0x00, sizeof(ThreadChain));
		return;
	}

	if(pThreadChain->type == ChainType_none)
		_thread_chain_build_new(pMsgChain);
	else
		_thread_chain_build_copy(pMsgChain, pThreadChain);

	THREADDEBUG("pThreadChain:%lx type:%d chain_id:%s %s->%s %s->%s:%s",
		pThreadChain,
		pThreadChain->type,
		pMsgChain->chain_id,
		pMsgChain->src_gid, pMsgChain->dst_gid,
		thread_id_to_name(msg_src), thread_id_to_name(msg_dst), msgstr(msg_id));
}

ThreadChain *
thread_chain_run_msg(MSGBODY *msg)
{
	ThreadChain *pMsgChain = (ThreadChain *)(msg->msg_chain);
	ThreadChain *pThreadChain;

	if((pMsgChain == NULL)
		|| (thread_chain_enable(msg->msg_src, msg->msg_dst, msg->msg_id) == dave_false))
	{
		return NULL;
	}

	pThreadChain = thread_current_chain();
	if(pThreadChain == NULL)
	{
		return NULL;
	}

	_thread_chain_run_copy(pThreadChain, pMsgChain);

	pMsgChain->send_time = dave_os_time_us();

	return pThreadChain;
}

void
thread_chain_run_clean(ThreadChain *pChain, MSGBODY *msg)
{
	ThreadChain *pMsgChain = (ThreadChain *)(msg->msg_chain);

	if(pMsgChain == NULL)
		return;

	pMsgChain->recv_time = dave_os_time_us();

	thread_chain_insert(
		ChainType_execution,
		pMsgChain,
		pMsgChain->src_gid, pMsgChain->dst_gid,
		msg->msg_src, msg->msg_dst,
		msg->msg_id, msg->msg_len, msg->msg_body);

	_thread_chain_clean(pChain);
}

void
thread_chain_clean_msg(MSGBODY *msg)
{
	if(msg->msg_chain != NULL)
	{
		thread_chain_free((ThreadChain *)(msg->msg_chain));
		msg->msg_chain = NULL;
	}
}

void
thread_chain_insert(
	ChainType type,
	ThreadChain *pChain,
	s8 *src_gid, s8 *dst_gid,
	ThreadId msg_src, ThreadId msg_dst,
	ub msg_id, ub msg_len, void *msg_body)
{
	if(pChain == NULL)
	{
		THREADLOG("chain is empty! type:%s %s->%s %s->%s:%s",
			type==ChainType_calling?"calling":type==ChainType_called?"called":"execution",
			src_gid, dst_gid,
			thread_id_to_name(msg_src), thread_id_to_name(msg_dst), msgstr(msg_id));
		return;
	}

	if(chain_enable() == dave_false)
	{
		return;
	}

	_thread_chain_insert_chain(pChain, type, src_gid, dst_gid, msg_src, msg_dst, msg_id);

	THREADDEBUG("type:%s chain_id:%s chain_counter:%d generation:%d time:%lx/%lx %s->%s %s->%s:%s",
		type==ChainType_calling?"calling":type==ChainType_called?"called":"execution",
		pChain->chain_id, pChain->chain_counter, pChain->generation,
		pChain->send_time, pChain->recv_time,
		pChain->src_gid, pChain->dst_gid,
		pChain->src_thread, pChain->dst_thread, msgstr(pChain->msg_id));

	chain_buf_set(pChain, msg_id, msg_len, msg_body);
}

MBUF *
thread_chain_report(void)
{
	return chain_buf_get();
}

void *
thread_chain_to_bson(ThreadChain *pChain)
{
	void *pBson;

	if(pChain == NULL)
		return NULL;

	pBson = t_bson_malloc_object();

	pChain->send_time = dave_os_time_us();

	t_bson_add_string(pBson, "1", pChain->chain_id);
	t_bson_add_int64(pBson, "2", pChain->generation);
	t_bson_add_int64(pBson, "3", pChain->send_time);
	t_bson_add_int64(pBson, "4", pChain->msg_serial);

	return pBson;
}

ThreadChain *
thread_bson_to_chain(void *pBson)
{
	ThreadChain *pChain;
	size_t chain_id_len;

	if(pBson == NULL)
		return NULL;

	pChain = thread_chain_malloc();

	_thread_chain_clean(pChain);
	chain_id_len = sizeof(pChain->chain_id);

	pChain->type = ChainType_called;
	t_bson_cpy_string(pBson, "1", pChain->chain_id, &chain_id_len);
	t_bson_inq_int64(pBson, "2", &(pChain->generation));
	t_bson_inq_int64(pBson, "3", &(pChain->send_time));
	t_bson_inq_int64(pBson, "4", &(pChain->msg_serial));

	return pChain;
}

#endif

