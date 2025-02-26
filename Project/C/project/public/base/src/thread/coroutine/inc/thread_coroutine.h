/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_COROUTINE_H__
#define __THREAD_COROUTINE_H__
#include "base_define.h"
#include "thread_struct.h"

typedef void (*coroutine_thread_fun)(base_thread_fun fun, MSGBODY *thread_msg);

void thread_coroutine_init(void);

void thread_coroutine_exit(void);

void thread_coroutine_reload_cfg(CFGUpdate *pUpdate);

void thread_coroutine_creat(ThreadStruct *pThread);

void thread_coroutine_die(ThreadStruct *pThread);

dave_bool thread_coroutine_running_step_co(
	ThreadStruct *pThread,
	coroutine_thread_fun coroutine_fun,
	base_thread_fun thread_fun,
	MSGBODY *msg);

void * thread_coroutine_running_step_setup(
	ThreadStruct *pSrcThread,
	ThreadId *src_id, ThreadId dst_id,
	ub req_msg_id, u8 *req_msg_body,
	ub rsp_msg_id, u8 *rsp_msg_body, ub rsp_msg_len);

void thread_coroutine_running_step_setup_clean(void *param);

void * thread_coroutine_running_step_yield(void *param);

dave_bool thread_coroutine_running_step_resume(
	void *msg_chain, void *msg_router,
	ThreadId src_id,
	ThreadStruct *pDstThread, ThreadId dst_id,
	ub msg_id, void *msg_body, ub msg_len);

ub thread_coroutine_info(s8 *info_ptr, ub info_len);

#endif

