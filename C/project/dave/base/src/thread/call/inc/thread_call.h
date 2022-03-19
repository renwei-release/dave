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

dave_bool thread_call_msg_register(ThreadId thread_id, ub msg_id, thread_msg_fun msg_fun, void *user_ptr);

void thread_call_msg_unregister(ThreadId thread_id, ub msg_id);

MsgCallFun * thread_call_msg(ThreadId thread_id, ub msg_id);

void thread_call_sync_check(void);

ThreadSync * thread_call_sync_pre(ThreadStruct *pSrcThread, ThreadId *sync_src_id, ThreadStruct *pDstThread, ub wait_msg, u8 *wait_body, ub wait_len);

void * thread_call_sync_wait(ThreadStruct *pSrcThread, ThreadStruct *pDstThread, ThreadSync *pSync);

dave_bool thread_call_sync_catch(ThreadStruct *pDstThread, ThreadId dst_id, ThreadId wait_thread, ub wait_msg, void *catch_body, ub catch_len);

#endif

