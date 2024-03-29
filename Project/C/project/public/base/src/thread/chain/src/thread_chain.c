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
#include "thread_router.h"
#include "thread_tools.h"
#include "thread_log.h"
#include "chain_id.h"
#include "chain_buf.h"
#include "chain_cfg.h"

#define CHAIN_FUN_EMPTY_FLAG NULL
#define CHAIN_FUN_PTR_FLAG (s8 *)0x55aa55aa55aa

static inline void
_thread_chain_clean(ThreadChain *pChain)
{
	if(pChain == NULL)
		return;

	pChain->type = ChainType_none;

	pChain->chain_id[0] = '\0';
	pChain->call_id = 0;
	pChain->chain_counter = 0;
	pChain->generation = 0;

	pChain->send_time = pChain->recv_time = 0;

	pChain->src_gid[0] = '\0';
	pChain->dst_gid[0] = '\0';

	pChain->src_thread[0] = '\0';
	pChain->src_thread[0] = '\0';

	pChain->request = dave_true;
	pChain->msg_src = pChain->msg_dst = 0;
	pChain->msg_id = MSGID_RESERVED;

	((s8 **)(pChain->fun))[0] = CHAIN_FUN_EMPTY_FLAG;
	((s8 **)(pChain->fun))[1] = NULL;
	pChain->line = 0;
}

static inline void
_thread_chain_build_new(
	ThreadChain *pMsgChain,
	ThreadId msg_src, ThreadId msg_dst, ub msg_id,
	s8 *fun, ub line)
{
	_thread_chain_clean(pMsgChain);

	chain_id(pMsgChain->chain_id, sizeof(pMsgChain->chain_id));
	pMsgChain->call_id = chain_call_id();

	pMsgChain->send_time = dave_os_time_us();

	pMsgChain->msg_src = msg_src;
	pMsgChain->msg_dst = msg_dst;
	pMsgChain->msg_id = msg_id;

	((s8 **)(pMsgChain->fun))[0] = CHAIN_FUN_PTR_FLAG;
	((s8 **)(pMsgChain->fun))[1] = fun;
	pMsgChain->line = line;
}

static inline void
_thread_chain_build_copy(
	ThreadChain *pMsgChain, ThreadChain *pThreadChain,
	ThreadId msg_src, ThreadId msg_dst, ub msg_id,
	s8 *fun, ub line)
{
	_thread_chain_clean(pMsgChain);

	pMsgChain->type = pThreadChain->type;

	dave_strcpy(pMsgChain->chain_id, pThreadChain->chain_id, sizeof(pMsgChain->chain_id));

	if((thread_get_local(pThreadChain->msg_src) == thread_get_local(msg_dst))
		&& (thread_get_local(pThreadChain->msg_dst) == thread_get_local(msg_src))
		&& ((pThreadChain->msg_id + 1) == msg_id))
	{
		/*
		 * 因为应答消息都是在请求消息的枚举量之下定义的，
		 * 所以，应答消息的消息msg_id比请求的msg_id多1。
		 * 此时，应答消息继承请求消息的call_id和generation。
		 */
		pMsgChain->request = dave_false;
		pMsgChain->call_id = pThreadChain->call_id;
		pMsgChain->generation = pThreadChain->generation;
	}
	else
	{
		pMsgChain->call_id = chain_call_id();
		pMsgChain->generation = pThreadChain->generation + 1;
	}

	pMsgChain->send_time = dave_os_time_us();

	pMsgChain->msg_src = msg_src;
	pMsgChain->msg_dst = msg_dst;
	pMsgChain->msg_id = msg_id;

	((s8 **)(pMsgChain->fun))[0] = CHAIN_FUN_PTR_FLAG;
	((s8 **)(pMsgChain->fun))[1] = fun;
	pMsgChain->line = line;
}

static inline void
_thread_chain_run_copy(ThreadChain *pThreadChain, ThreadChain *pMsgChain, ThreadId msg_src, ThreadId msg_dst, ub msg_id)
{
	_thread_chain_clean(pThreadChain);

	pThreadChain->type = ChainType_execution;

	dave_strcpy(pThreadChain->chain_id, pMsgChain->chain_id, sizeof(pThreadChain->chain_id));
	pThreadChain->call_id = pMsgChain->call_id;
	pThreadChain->generation = pMsgChain->generation;

	pThreadChain->msg_src = msg_src;
	pThreadChain->msg_dst = msg_dst;
	pThreadChain->msg_id = msg_id;
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

	pChain->msg_src = msg_src;
	pChain->msg_dst = msg_dst;
	pChain->msg_id = msg_id;
}

static inline dave_bool
_thread_chain_enable(ThreadId msg_src, ThreadId msg_dst)
{
	if((thread_is_remote(msg_src) == dave_false)
		&& (thread_id_to_attrib(msg_src) != REMOTE_TASK_ATTRIB)
		&& (thread_is_remote(msg_dst) == dave_false)
		&& (thread_id_to_attrib(msg_dst) != REMOTE_TASK_ATTRIB))
	{
		return dave_false;
	}

	return chain_enable();
}

static inline ThreadChain *
_thread_chain_malloc(s8 *fun, ub line)
{
	return (ThreadChain *)__base_malloc__(sizeof(ThreadChain), dave_false, 0x00, fun, line);
}

static inline void
_thread_chain_free(ThreadChain *pChain)
{
	if(pChain != NULL)
	{
		pChain->type = ChainType_none;
		pChain->chain_id[0] = '\0';

		dave_free(pChain);
	}
}

// =====================================================================

void
thread_chain_init(void)
{
	chain_cfg_reset(NULL);

	chain_buf_init();

	chain_id_reset();
}

void
thread_chain_exit(void)
{
	chain_buf_exit();
}

void
thread_chain_reload_cfg(CFGUpdate *pUpdate)
{
	chain_cfg_reset(pUpdate);
}

void
thread_chain_reset(ThreadChain *pChain)
{
	dave_memset(pChain, 0x00, sizeof(ThreadChain));

	_thread_chain_clean(pChain);
}

void
thread_chain_free(ThreadChain *pChain)
{
	_thread_chain_free(pChain);
}

ThreadChain *
thread_chain_build_msg(
	void *msg_chain,
	ThreadId msg_src, ThreadId msg_dst, ub msg_id,
	s8 *fun, ub line)
{
	ThreadChain *pMsgChain;
	ThreadChain *pThreadChain;

	if(msg_chain != NULL)
	{
		return (ThreadChain *)msg_chain;
	}

	if((thread_internal_msg(msg_id) == dave_true)
		|| (chain_enable() == dave_false))
	{
		return NULL;
	}

	pThreadChain = thread_current_chain();
	if(pThreadChain == NULL)
	{
		THREADDEBUG("pThreadChain is NULL! %x/%s->%x/%s:%s <%s:%d>",
			msg_src, thread_id_to_name(msg_src), msg_dst, thread_id_to_name(msg_dst),
			msgstr(msg_id),
			fun, line);
		return NULL;
	}

	pMsgChain = _thread_chain_malloc(fun, line);

	if(pThreadChain->type == ChainType_none)
		_thread_chain_build_new(pMsgChain, msg_src, msg_dst, msg_id, fun, line);
	else
		_thread_chain_build_copy(pMsgChain, pThreadChain, msg_src, msg_dst, msg_id, fun, line);

	return pMsgChain;
}

void
thread_chain_clean_msg(MSGBODY *msg)
{
	if(msg->msg_chain != NULL)
	{
		_thread_chain_free((ThreadChain *)(msg->msg_chain));
		msg->msg_chain = NULL;
	}
}

ThreadChain *
thread_chain_run_msg(MSGBODY *msg)
{
	ThreadChain *pMsgChain = (ThreadChain *)(msg->msg_chain);
	ThreadChain *pThreadChain;

	if(pMsgChain == NULL)
	{
		return NULL;
	}

	if(thread_internal_msg(msg->msg_id) == dave_true)
	{
		return NULL;
	}

	pThreadChain = thread_current_chain();
	if(pThreadChain == NULL)
	{
		return NULL;
	}

	_thread_chain_run_copy(pThreadChain, pMsgChain, msg->msg_src, msg->msg_dst, msg->msg_id);

	pMsgChain->send_time = dave_os_time_us();

	return pThreadChain;
}

void
thread_chain_run_clean(ThreadChain *pThreadChain, MSGBODY *msg)
{
	ThreadChain *pMsgChain = (ThreadChain *)(msg->msg_chain);

	if(pMsgChain == NULL)
		return;

	pMsgChain->recv_time = dave_os_time_us();

	thread_chain_insert(
		ChainType_execution,
		msg->msg_chain, msg->msg_router,
		pMsgChain->src_gid, pMsgChain->dst_gid,
		msg->msg_src, msg->msg_dst,
		msg->msg_id, msg->msg_len, msg->msg_body);

	_thread_chain_clean(pThreadChain);
}

void
thread_chain_coroutine_msg(
	ThreadChain *pMsgChain, ThreadRouter *pMsgRouter,
	ThreadId msg_src, ThreadId msg_dst,
	ub msg_id, ub msg_len, u8 *msg_body)
{
	if(pMsgChain != NULL)
	{
		pMsgChain->send_time = pMsgChain->recv_time;
	
		thread_chain_insert(
			ChainType_coroutine,
			pMsgChain, pMsgRouter,
			pMsgChain->src_gid, pMsgChain->dst_gid,
			msg_src, msg_dst,
			msg_id, msg_len, msg_body);
	}
}

void
thread_chain_insert(
	ChainType type,
	ThreadChain *pChain, ThreadRouter *pRouter,
	s8 *src_gid, s8 *dst_gid,
	ThreadId msg_src, ThreadId msg_dst,
	ub msg_id, ub msg_len, void *msg_body)
{
	if((_thread_chain_enable(msg_src, msg_dst) == dave_false)
		|| (chain_type_enable(type) == dave_false))
	{
		return;
	}

	if(pChain == NULL)
	{
		THREADLTRACE(60,1,"chain is empty! type:%d %s->%s %s->%s:%s",
			type,
			src_gid, dst_gid,
			thread_id_to_name(msg_src), thread_id_to_name(msg_dst), msgstr(msg_id));
		return;
	}

	_thread_chain_insert_chain(pChain, type, src_gid, dst_gid, msg_src, msg_dst, msg_id);

	if(((s8 **)(pChain->fun))[0] == CHAIN_FUN_PTR_FLAG)
	{
		s8 *fun_ptr = ((s8 **)(pChain->fun))[1];
		dave_strcpy(pChain->fun, fun_ptr, sizeof(pChain->fun));
	}

	chain_buf_set(pChain, pRouter, msg_id, msg_len, msg_body);
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
	s8 *fun_ptr;

	if(pChain == NULL)
		return NULL;

	pBson = t_bson_malloc_object();

	pChain->send_time = dave_os_time_us();

	t_bson_add_string(pBson, "1", pChain->chain_id);
	t_bson_add_int64(pBson, "2", pChain->call_id);
	t_bson_add_int64(pBson, "3", pChain->generation);
	t_bson_add_int64(pBson, "4", pChain->send_time);
	t_bson_add_boolean(pBson, "5", pChain->request);
	if(((s8 **)(pChain->fun))[0] == CHAIN_FUN_PTR_FLAG)
	{
		fun_ptr = ((s8 **)(pChain->fun))[1];
	}
	else
	{
		fun_ptr = pChain->fun;
	}
	t_bson_add_string(pBson, "6", fun_ptr);
	t_bson_add_int64(pBson, "7", pChain->line);

	return pBson;
}

ThreadChain *
thread_bson_to_chain(void *pBson)
{
	ThreadChain *pChain;

	if(pBson == NULL)
		return NULL;

	pChain = _thread_chain_malloc((s8 *)__func__, (ub)__LINE__);

	_thread_chain_clean(pChain);

	pChain->type = ChainType_called;
	t_bson_cpy_string(pBson, "1", pChain->chain_id, sizeof(pChain->chain_id));
	t_bson_inq_int64(pBson, "2", &(pChain->call_id));
	t_bson_inq_int64(pBson, "3", &(pChain->generation));
	t_bson_inq_int64(pBson, "4", &(pChain->send_time));
	t_bson_inq_boolean(pBson, "5", &(pChain->request));
	t_bson_cpy_string(pBson, "6", pChain->fun, sizeof(pChain->fun));
	t_bson_inq_int64(pBson, "7", &(pChain->line));

	return pChain;
}

#endif

