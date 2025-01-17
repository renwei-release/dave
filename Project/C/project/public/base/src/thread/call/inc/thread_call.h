/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_CALL_H__
#define __THREAD_CALL_H__
#include "base_define.h"
#include "thread_struct.h"

void thread_call_init(void);

void thread_call_exit(void);

dave_bool thread_call_msg_register(ThreadId thread_id, ub msg_id, base_thread_fun msg_fun, void *user_ptr);

void thread_call_msg_unregister(ThreadId thread_id, ub msg_id);

MsgCallFun * thread_call_msg(ThreadId thread_id, ub msg_id);

void thread_call_sync_check(void);

void * thread_call_sync_pre(
	ThreadStruct *pSrcThread, ThreadId *src_id,
	ThreadStruct *pDstThread, ThreadId dst_id,
	ub req_msg_id, u8 *req_msg_body, ub req_msg_len,
	ub rsp_msg_id, u8 *rsp_msg_body, ub rsp_msg_len);

void thread_call_sync_pre_clean(
	ThreadStruct *pSrcThread, ub req_msg_id,
	void *param);

void * thread_call_sync_wait(
	ThreadStruct *pSrcThread, ThreadStruct *pDstThread,
	void *pSync);

dave_bool thread_call_sync_catch(
	void *msg_chain, void *msg_router,
	ThreadId src_id,
	ThreadStruct *pDstThread, ThreadId dst_id,
	ub msg_id, void *msg_body, ub msg_len,
	dave_bool *hold_body);

#endif

