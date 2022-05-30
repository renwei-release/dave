/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "dave_3rdparty.h"
#include "dave_os.h"
#include "thread_struct.h"
#include "thread_mem.h"
#include "thread_thread.h"
#include "thread_tools.h"
#include "thread_log.h"

typedef struct {
	ThreadStruct *pThread;
	base_thread_fun thread_fun;
	MSGBODY msg;
	void *co;

	ub wait_thread_index;
	ub wait_wakeup_index;
	ub wait_msg;

	u8 *wait_body;
	ub wait_len;

	ub wait_start_time;
} CoroutineInfo;

static void *_coroutine_kv = NULL;

static inline ThreadId
_thread_coroutine_set_index(ThreadId thread_id, ub *wakeup_index)
{
	if(thread_thread_self(wakeup_index) == thread_id)
	{
		return thread_set_wakeup(thread_id, *wakeup_index);
	}
	else
	{
		*wakeup_index = 0;

		return thread_set_wakeup(thread_id, *wakeup_index);
	}
}

static inline ub
_thread_coroutine_key_kv(ub thread_index, ub wakeup_index, ub msg)
{
	return (thread_index << 32) + (wakeup_index << 16) + msg;
}

static inline void
_thread_coroutine_add_kv(ub thread_index, ub wakeup_index, ub msg, CoroutineInfo *pInfo)
{
	if(_coroutine_kv == NULL)
		return;

	kv_add_ub_ptr(_coroutine_kv, _thread_coroutine_key_kv(thread_index, wakeup_index, msg), pInfo);
}

static inline CoroutineInfo *
_thread_coroutine_del_kv(ub thread_index, ub wakeup_index, ub msg)
{
	if(_coroutine_kv == NULL)
		return NULL;

	return kv_del_ub_ptr(_coroutine_kv, _thread_coroutine_key_kv(thread_index, wakeup_index, msg));
}

static inline void
_thread_coroutine_wakeup_me(CoroutineInfo *pInfo)
{
	CoroutineWakeup *pWakeup;

	pWakeup = thread_msg(pWakeup);

	pWakeup->thread_index = pInfo->wait_thread_index;
	pWakeup->wakeup_index = pInfo->wait_wakeup_index;
	pWakeup->ptr = pInfo;

	thread_thread_write(pInfo->wait_thread_index, pInfo->wait_wakeup_index, MSGID_COROUTINE_WAKEUP, pWakeup);
}

static inline CoroutineInfo *
_thread_coroutine_info_malloc(ThreadStruct *pThread, base_thread_fun thread_fun, MSGBODY *msg)
{
	CoroutineInfo *pInfo = dave_malloc(sizeof(CoroutineInfo));

	pInfo->pThread = pThread;
	pInfo->thread_fun = thread_fun;
	pInfo->msg = *msg;
	pInfo->co = NULL;

	msg->mem_state = MsgMemState_captured;

	return pInfo;
}

static inline void
_thread_coroutine_info_free(CoroutineInfo *pInfo)
{
	if(pInfo->msg.msg_body != NULL)
	{
		thread_free(pInfo->msg.msg_body, pInfo->msg.msg_id, (s8 *)__func__, (ub)__LINE__);
	}

	if(pInfo->co != NULL)
	{
		dave_co_release(pInfo->co);
	}

	dave_free(pInfo);
}

static inline void
_thread_coroutine_running_step_6(CoroutineWakeup *pWakeup, ub wakeup_index)
{
	CoroutineInfo *pInfo = (CoroutineInfo *)(pWakeup->ptr);

	THREADLOG("");

	if(pWakeup->wakeup_index != wakeup_index)
	{
		THREADABNOR("Algorithm error, wakeup_index mismatch:%d/%d",
			pWakeup->wakeup_index, wakeup_index);
	}

	dave_co_resume(pInfo->co);
}

static dave_bool
_thread_coroutine_running_step_5(ThreadId src_id, ThreadId dst_id, ub wait_msg, void *wait_body, ub wait_len)
{
	ThreadStruct *pWaitThread;
	ub wakeup_index;
	CoroutineInfo *pInfo;

	pWaitThread = thread_find_busy_thread(dst_id);
	if(pWaitThread == NULL)
	{
		THREADLOG("%lx/%s can't find pSrcThread!", dst_id, thread_name(dst_id));
		return dave_false;
	}
	wakeup_index = thread_get_wakeup(dst_id);

	pInfo = _thread_coroutine_del_kv(pWaitThread->thread_index, wakeup_index, wait_msg);
	if(pInfo == NULL)
	{
		THREADDEBUG("thread_index:%d wakeup_index:%d wait_msg:%s wait_thread:%s",
			pWaitThread->thread_index, wakeup_index, msgstr(wait_msg),
			pWaitThread->thread_name);
		return dave_false;
	}

	THREADLOG("");

	if((pInfo->wait_thread_index != pWaitThread->thread_index)
		|| (pInfo->wait_wakeup_index != wakeup_index)
		|| (pInfo->wait_msg != wait_msg))
	{
		THREADABNOR("wait thread mismatch, thread_index:%d/%d wakeup_index:%d/%d wait_msg:%s/%s",
			pInfo->wait_thread_index, pWaitThread->thread_index,
			pInfo->wait_wakeup_index, wakeup_index,
			msgstr(pInfo->wait_msg), msgstr(wait_msg));
		return dave_false;
	}

	THREADLOG("");

	if(wait_len > pInfo->wait_len)
		wait_len = pInfo->wait_len;
	pInfo->wait_len = dave_memcpy(pInfo->wait_body, wait_body, wait_len);

	pInfo->wait_start_time = dave_os_time_us();

	_thread_coroutine_wakeup_me(pInfo);

	return dave_true;
}

static inline void *
_thread_coroutine_running_step_4(void *param)
{
	CoroutineInfo *pInfo = (CoroutineInfo *)param;

	THREADLOG("thread_index:%d wakeup_index:%d wait_msg:%s",
		pInfo->wait_thread_index, pInfo->wait_wakeup_index, msgstr(pInfo->wait_msg));

	_thread_coroutine_add_kv(pInfo->wait_thread_index, pInfo->wait_wakeup_index, pInfo->wait_msg, pInfo);

	dave_co_yield(pInfo->co);

	THREADLOG("wakeup me !!!!!!!!!!!!!!!!");

	return pInfo->wait_body;
}

static inline void *
_thread_coroutine_running_setp_3(ThreadStruct *pSrcThread, ThreadId *sync_src_id, ub wait_msg, u8 *wait_body, ub wait_len)
{
	ub wakeup_index;
	CoroutineInfo *pInfo;

	*sync_src_id = _thread_coroutine_set_index(*sync_src_id, &wakeup_index);

	pInfo = (CoroutineInfo *)thread_thread_get_coroutine_point(pSrcThread->thread_index, wakeup_index);
	if(pInfo == NULL)
	{
		THREADABNOR("%s can't find sync on wait_msg:%d!",
			pSrcThread->thread_name, wait_msg);
		return NULL;
	}

	pInfo->wait_thread_index = pSrcThread->thread_index;
	pInfo->wait_wakeup_index = wakeup_index;
	pInfo->wait_msg = wait_msg;

	pInfo->wait_body = wait_body;
	pInfo->wait_len = wait_len;

	pInfo->wait_start_time = dave_os_time_us();

	return pInfo;
}

static inline void *
_thread_coroutine_running_step_2(void *param)
{
	CoroutineInfo *pInfo = param;

	pInfo->thread_fun(&(pInfo->msg));

	_thread_coroutine_info_free(pInfo);

	return NULL;
}

static inline void
_thread_coroutine_running_step_1(ThreadStruct *pThread, base_thread_fun thread_fun, MSGBODY *msg)
{
	CoroutineInfo *pInfo = _thread_coroutine_info_malloc(pThread, thread_fun, msg);

	pInfo->co = dave_co_create(_thread_coroutine_running_step_2, pInfo);

	thread_thread_set_coroutine_point(pThread->thread_index, msg->thread_wakeup_index, pInfo);

	dave_co_resume(pInfo->co);
}

static inline void
_thread_coroutine_wakeup(MSGBODY *thread_msg)
{
	CoroutineWakeup *pWakeup = (CoroutineWakeup *)(thread_msg->msg_body);

	_thread_coroutine_running_step_6(pWakeup, thread_msg->thread_wakeup_index);
}

static inline void
_thread_coroutine_kv_timer_out(void *ramkv, s8 *key)
{
	THREADLOG("%s", key);
}

static inline dave_bool
_thread_coroutine_msg_can_be_go(MSGBODY *msg)
{
	switch(msg->msg_id)
	{
		case MSGID_COROUTINE_WAKEUP:
				return dave_false;
			break;
		default:
				return dave_true;
			break;
	}

	return dave_true;
}

// =====================================================================

void
thread_coroutine_malloc(ThreadStruct *pThread)
{
	base_thread_msg_register(pThread->thread_id, MSGID_COROUTINE_WAKEUP, _thread_coroutine_wakeup, NULL);
}

void
thread_coroutine_free(ThreadStruct *pThread)
{
	base_thread_msg_unregister(pThread->thread_id, MSGID_COROUTINE_WAKEUP);
}

dave_bool
thread_coroutine_running_setp_go(ThreadStruct *pThread, base_thread_fun thread_fun, MSGBODY *msg)
{
	if(_thread_coroutine_msg_can_be_go(msg) == dave_false)
		return dave_false;

	thread_other_lock();
	if(_coroutine_kv == NULL)
	{
		_coroutine_kv = kv_malloc("ckv", KvAttrib_ram, 30, _thread_coroutine_kv_timer_out);
	}
	thread_other_unlock();

	_thread_coroutine_running_step_1(pThread, thread_fun, msg);

	return dave_true;
}

void *
thread_coroutine_running_setp_setup(ThreadStruct *pSrcThread, ThreadId *sync_src_id, ub wait_msg, u8 *wait_body, ub wait_len)
{
	return _thread_coroutine_running_setp_3(pSrcThread, sync_src_id, wait_msg, wait_body, wait_len);
}

void *
thread_coroutine_running_step_yield(void *param)
{
	return _thread_coroutine_running_step_4(param);
}

dave_bool
thread_coroutine_running_step_resume(ThreadId src_id, ThreadId dst_id, ub wait_msg, void *catch_body, ub catch_len)
{
	return _thread_coroutine_running_step_5(src_id, dst_id, wait_msg, catch_body, catch_len);
}

#endif

