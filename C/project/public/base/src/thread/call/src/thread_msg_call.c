/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "thread_tools.h"
#include "thread_parameter.h"
#include "thread_mem.h"
#include "thread_log.h"

static void *_msg_call_kv = NULL;

static inline s8 *
_thread_msg_call_key(s8 *key_ptr, ub key_len, ThreadId thread_id, ub msg_id)
{
	dave_snprintf(key_ptr, key_len, "%d-%d", thread_id, msg_id);

	return key_ptr;
}

static inline void
_thread_msg_call_add(ThreadId thread_id, ub msg_id, thread_msg_fun msg_fun, void *user_ptr)
{
	MsgCallFun *pFun = dave_malloc(sizeof(MsgCallFun));
	s8 key[256];

	pFun->thread_id = thread_id;
	pFun->msg_id = msg_id;
	pFun->msg_fun = msg_fun;
	pFun->user_ptr = user_ptr;

	base_ramkv_add_key_ptr(
		_msg_call_kv,
		_thread_msg_call_key(key, sizeof(key), thread_id, msg_id),
		pFun);
}

static inline void
_thread_msg_call_del(ThreadId thread_id, ub msg_id)
{
	s8 key[256];
	MsgCallFun *pFun;

	pFun = base_ramkv_del_key_ptr(
		_msg_call_kv,
		_thread_msg_call_key(key, sizeof(key), thread_id, msg_id));

	if(pFun != NULL)
		dave_free(pFun);
}

static inline MsgCallFun *
_thread_msg_call_inq(ThreadId thread_id, ub msg_id)
{
	s8 key[256];

	return (MsgCallFun *)base_ramkv_inq_key_ptr(
		_msg_call_kv,
		_thread_msg_call_key(key, sizeof(key), thread_id, msg_id));
}

static RetCode
_thread_msg_call_recycle(void *ramkv, s8 *key)
{
	MsgCallFun *pFun = base_ramkv_del_key_ptr(ramkv, key);

	if(pFun == NULL)
		return RetCode_empty_data;

	dave_free(pFun);

	return RetCode_OK;
}

// =====================================================================

void
thread_msg_call_init(void)
{
	_msg_call_kv = base_ramkv_malloc("tmckv", KvAttrib_ram, 0, NULL);
}

void
thread_msg_call_exit(void)
{
	base_ramkv_free(_msg_call_kv, _thread_msg_call_recycle);
}

dave_bool
thread_msg_call_register(ThreadId thread_id, ub msg_id, thread_msg_fun msg_fun, void *user_ptr)
{
	if((thread_id == INVALID_THREAD_ID) || (msg_fun == NULL))
	{
		THREADABNOR("invalid param:%d,%x", thread_id, msg_fun);
		return dave_false;
	}

	thread_id = thread_get_local(thread_id);
	_thread_msg_call_add(thread_id, msg_id, msg_fun, user_ptr);

	return dave_true;
}

void
thread_msg_call_unregister(ThreadId thread_id, ub msg_id)
{
	thread_id = thread_get_local(thread_id);
	_thread_msg_call_del(thread_id, msg_id);
}

MsgCallFun *
thread_msg_call(ThreadId thread_id, ub msg_id)
{
	thread_id = thread_get_local(thread_id);
	return _thread_msg_call_inq(thread_id, msg_id);
}

#endif

