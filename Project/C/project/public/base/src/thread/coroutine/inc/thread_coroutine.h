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

void thread_coroutine_malloc(ThreadStruct *pThread);
void thread_coroutine_free(ThreadStruct *pThread);

dave_bool thread_coroutine_running_setp_go(ThreadStruct *pThread, base_thread_fun thread_fun, MSGBODY *msg);
void * thread_coroutine_running_setp_setup(ThreadStruct *pSrcThread, ThreadId *sync_src_id, ub wait_msg, u8 *wait_body, ub wait_len);
void * thread_coroutine_running_step_yield(void *param);
dave_bool thread_coroutine_running_step_resume(ThreadId wait_thread, ThreadId dst_id, ub wait_msg, void *catch_body, ub catch_len);

#endif

