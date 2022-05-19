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

static inline MsgCallFun *
_thread_msg_call_inq(ThreadId thread_id, ub msg_id)
{
	s8 key[256];

	return (MsgCallFun *)base_ramkv_inq_key_ptr(
		_msg_call_kv,
		_thread_msg_call_key(key, sizeof(key), thread_id, msg_id));
}

static inline void
_thread_msg_call_add(ThreadId thread_id, ub msg_id, thread_msg_fun msg_fun, void *user_ptr)
{
	MsgCallFun *pFun;
	s8 key[256];

	/*
	 * 在程序刚开始跑的时候，还未来得及执行
	 * base_ramkv_add_key_ptr
	 * 这时，如果刚好有多次并发
	 * _thread_msg_call_inq，
	 * 这时会产生多个pFun，
	 * 其实只有一个pFun被base_ramkv_add_key_ptr
	 * 最后写入了KV。其他的pFun都被最后写入的那次给覆盖了，
	 * 这样就造成了pFun内存的泄漏。
	 * 但，没关系，这种情况很少见。要处理这个情况会影响程序效率，
	 * 比如在此函数加锁，这样就会大大降低这个函数
	 * 的执行效率，所以：
	 * 这里不处理这个小的泄漏问题，用泄漏换效率。
	 * 注意，这种处理策略会出现在其他的类似以下方式调用的代码里面，
	 * 未来还是要考虑一个好的效率和泄漏都顾全的方法。
	 * Renwei 20220515
	 */
	pFun = _thread_msg_call_inq(thread_id, msg_id);
	if(pFun == NULL)
	{
		pFun = dave_malloc(sizeof(MsgCallFun));
	}
	else
	{
		if((pFun->msg_fun == msg_fun)
			&& (pFun->user_ptr == user_ptr))
		{
			return;
		}
	}

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

