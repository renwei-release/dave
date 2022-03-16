/*
 * ================================================================================
 * (c) Copyright 2018 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2018.08.23.
 * ================================================================================
 */

#ifndef __THREAD_SYNC_CALL_H__
#define __THREAD_SYNC_CALL_H__
#include "base_macro.h"
#include "base_define.h"
#include "thread_struct.h"

void thread_sync_call_init(void);

void thread_sync_call_exit(void);

void thread_sync_call_check(void);

ThreadSync * thread_sync_call_step_1_pre(ThreadStruct *pSrcThread, ThreadId *sync_src_id, ThreadStruct *pDstThread, ub wait_msg, u8 *wait_body, ub wait_len);

void * thread_sync_call_step_2_wait(ThreadStruct *pSrcThread, ThreadStruct *pDstThread, ThreadSync *pSync);

dave_bool thread_sync_call_step_3_catch(ThreadStruct *pDstThread, ThreadId dst_id, ThreadId wait_thread, ub wait_msg, void *catch_body, ub catch_len);

#endif

