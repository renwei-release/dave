/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_TOOLS_H__
#define __THREAD_TOOLS_H__
#include "base_macro.h"
#include "thread_struct.h"
#include "thread_must_in_local_table.h"
#include "thread_lock.h"
#include "thread_statistics.h"
#include "thread_map.h"
#include "thread_id.h"
#include "thread_queue_opt.h"

void thread_tools_init(ThreadStruct *thread_struct);

void thread_tools_exit(void);

ThreadStruct * __thread_find_busy_thread__(ThreadId thread_id, s8 *fun, ub line);
#define thread_find_busy_thread(thread_id) __thread_find_busy_thread__(thread_id, (s8 *)__func__, (ub)__LINE__)

ub __thread_find_busy_index__(ThreadId thread_id, s8 *fun, ub line);
#define thread_find_busy_index(thread_id) __thread_find_busy_index__(thread_id, (s8 *)__func__, (ub)__LINE__)

ub thread_find_free_index(s8 *thread_name);

ub thread_show_all_info(ThreadStruct *pThread, DateStruct *pWorkDate, s8 *msg, ub msg_len, dave_bool base_flag);

void thread_check_pair_msg(ub req_id, ub rsp_id);

void thread_reset_sync(ThreadSync *pSync);

void __thread_clean_user_input_data__(void *msg_body, ub msg_id, s8 *fun, ub line);
#define thread_clean_user_input_data(msg_body, msg_id) __thread_clean_user_input_data__(msg_body, msg_id, (s8 *)__func__, (ub)__LINE__)

dave_bool __thread_enable_coroutine__(ThreadStruct *pThread, ub msg_id, s8 *fun, ub line);
#define thread_enable_coroutine(pThread, msg_id) __thread_enable_coroutine__(pThread, msg_id, (s8 *)__func__, (ub)__LINE__)

ThreadMsg * thread_build_msg(
	ThreadStruct *pThread,
	void *msg_chain, void *msg_router,
	s8 *src_gid, s8 *src_name,
	ThreadId src_id, ThreadId dst_id,
	ub msg_id, ub data_len, u8 *data,
	BaseMsgType msg_type,
	s8 *fun, ub line);

void thread_clean_msg(ThreadMsg *pMsg);

ThreadChain * thread_current_chain(void);

ThreadRouter * thread_current_router(void);

s8 * thread_id_to_name(ThreadId thread_id);

TaskAttribute thread_id_to_attrib(ThreadId thread_id);

dave_bool thread_internal_msg(ub msg_id);

void thread_local_ready_notify(s8 *thread_name);

void thread_local_remove_notify(s8 *thread_name);

#endif

