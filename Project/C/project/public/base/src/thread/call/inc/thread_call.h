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
	ub msg_id, u8 *msg_body, ub msg_len,
	ub wait_msg, u8 *wait_body, ub wait_len);

void * thread_call_sync_wait(
	ThreadStruct *pSrcThread, ThreadStruct *pDstThread,
	void *pSync);

dave_bool thread_call_sync_catch(
	ThreadId src_id,
	ThreadStruct *pDstThread, ThreadId dst_id,
	ub msg_id, void *msg_body, ub msg_len);

#endif

