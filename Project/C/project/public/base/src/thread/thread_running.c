/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "thread_struct.h"
#include "thread_lock.h"
#include "thread_mem.h"
#include "thread_tools.h"
#include "thread_coroutine.h"
#include "thread_log.h"

static inline ThreadStack *
_thread_running_push_stack(ThreadStack **ppCurrentMsgStack, ThreadStruct *pThread)
{
	ThreadStack *pStack;

	pStack = thread_malloc(sizeof(ThreadStack), MSGID_RESERVED, (s8 *)__func__, (ub)__LINE__);

	thread_lock();
	pStack->next_stack = *ppCurrentMsgStack;
	pStack->thread_id = pThread->thread_id;
	*ppCurrentMsgStack = pStack;
	thread_unlock();

	return pStack;
}

static inline void
_thread_running_pop_stack(ThreadStack **ppCurrentMsgStack, ThreadStack *pStack)
{
	thread_lock();
	if(*ppCurrentMsgStack != NULL)
	{
		*ppCurrentMsgStack = (*ppCurrentMsgStack)->next_stack;
	}
	thread_unlock();

	thread_free(pStack, MSGID_RESERVED, (s8 *)__func__, (ub)__LINE__);
}

static inline void
_thread_running(ThreadStruct *pThread, base_thread_fun thread_fun, MSGBODY *msg)
{
	if(thread_fun == NULL)
		return;

#ifdef ENABLE_THREAD_STATISTICS
	ub run_time = thread_statistics_start_msg(msg);
#endif

	if(thread_enable_coroutine(pThread) == dave_true)
	{
		if(thread_coroutine_running_setp_go(pThread, thread_fun, msg) == dave_false)
			thread_fun(msg);
	}
	else
	{
		thread_fun(msg);
	}

#ifdef ENABLE_THREAD_STATISTICS
	thread_statistics_end_msg(run_time, pThread, msg);
#endif
}

// =====================================================================

void
thread_running(
	ThreadStack **ppCurrentMsgStack,
	base_thread_fun thread_fun,
	ThreadStruct *pThread,
	MSGBODY *msg,
	dave_bool enable_stack)
{
	ThreadStack *pStack = NULL;

	if((pThread == NULL) || (msg == NULL))
	{
		THREADABNOR("pThread:%x or msg:%x is NULL!", pThread, msg);
		return;
	}

	if(pThread->thread_id != thread_get_local(msg->msg_dst))
	{
		THREADLOG("schedule error! thread:%s<%d> msg:%s<%d>->%s<%d> %d %d",
			pThread->thread_name, pThread->thread_id,
			thread_name(msg->msg_src), msg->msg_src,
			thread_name(msg->msg_dst), msg->msg_dst,
			msg->msg_id,
			msg->thread_wakeup_index);
	}

	if(enable_stack == dave_true)
	{
		pStack = _thread_running_push_stack(ppCurrentMsgStack, pThread);
	}

	_thread_running(pThread, thread_fun, msg);

	if(enable_stack == dave_true)
	{
		_thread_running_pop_stack(ppCurrentMsgStack, pStack);
	}
}

#endif

