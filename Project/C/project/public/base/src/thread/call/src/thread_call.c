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
#include "thread_parameter.h"
#include "thread_mem.h"
#include "thread_struct.h"
#include "thread_thread.h"
#include "thread_call.h"
#include "thread_sync_call.h"
#include "thread_msg_call.h"
#include "thread_coroutine.h"
#include "thread_tools.h"
#include "thread_log.h"

// =====================================================================

void
thread_call_init(void)
{
	thread_sync_call_init();
	thread_msg_call_init();
}

void
thread_call_exit(void)
{
	thread_sync_call_exit();
	thread_msg_call_exit();
}

dave_bool
thread_call_msg_register(ThreadId thread_id, ub msg_id, base_thread_fun msg_fun, void *user_ptr)
{
	return thread_msg_call_register(thread_id, msg_id, msg_fun, user_ptr);
}

void
thread_call_msg_unregister(ThreadId thread_id, ub msg_id)
{
	thread_msg_call_unregister(thread_id, msg_id);
}

MsgCallFun *
thread_call_msg(ThreadId thread_id, ub msg_id)
{
	return thread_msg_call(thread_id, msg_id);
}

void
thread_call_sync_check(void)
{
	thread_sync_call_check();
}

void *
thread_call_sync_pre(
	ThreadStruct *pSrcThread, ThreadId *src_id,
	ThreadStruct *pDstThread, ThreadId dst_id,
	ub req_msg_id, u8 *req_msg_body, ub req_msg_len,
	ub rsp_msg_id, u8 *rsp_msg_body, ub rsp_msg_len)
{
#ifdef ENABLE_THREAD_COROUTINE
	if(thread_enable_coroutine(pSrcThread, req_msg_id) == dave_true)
		return thread_coroutine_running_step_setup(pSrcThread, src_id, dst_id, req_msg_id, req_msg_body, rsp_msg_id, rsp_msg_body, rsp_msg_len);
	else
#endif
		return thread_sync_call_step_1_pre(pSrcThread, src_id, pDstThread, rsp_msg_id, rsp_msg_body, rsp_msg_len);
}

void
thread_call_sync_pre_clean(
	ThreadStruct *pSrcThread, ub req_msg_id,
	void *param)
{
#ifdef ENABLE_THREAD_COROUTINE
	if(thread_enable_coroutine(pSrcThread, req_msg_id) == dave_true)
		thread_coroutine_running_step_setup_clean(param);
#endif
}

void *
thread_call_sync_wait(
	ThreadStruct *pSrcThread, ThreadStruct *pDstThread,
	void *pSync)
{
#ifdef ENABLE_THREAD_COROUTINE
	if(thread_enable_coroutine(pSrcThread, MSGID_RESERVED) == dave_true)
		return thread_coroutine_running_step_yield(pSync);
	else
#endif
		return thread_sync_call_step_2_wait(pSrcThread, pDstThread, pSync);
}

dave_bool
thread_call_sync_catch(
	void *msg_chain, void *msg_router,
	ThreadId src_id,
	ThreadStruct *pDstThread, ThreadId dst_id,
	ub msg_id, void *msg_body, ub msg_len,
	dave_bool *hold_body)
{
	dave_bool catch_flag;

#ifdef ENABLE_THREAD_COROUTINE
	if(thread_enable_coroutine(pDstThread, msg_id) == dave_true)
	{
		catch_flag = *hold_body = thread_coroutine_running_step_resume(
			msg_chain, msg_router,
			src_id,
			pDstThread, dst_id,
			msg_id, msg_body, msg_len);
	}
	else
#endif
	{
		*hold_body = dave_false;
		catch_flag = thread_sync_call_step_3_catch(pDstThread, dst_id, src_id, msg_id, msg_body, msg_len);
	}

	return catch_flag;
}

#endif

