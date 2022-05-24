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
#include "thread_queue.h"

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

void __thread_clean_user_input_data__(void *data, ub msg_id, s8 *fun, ub line);
#define thread_clean_user_input_data(data, msg_id) __thread_clean_user_input_data__(data, msg_id, (s8 *)__func__, (ub)__LINE__)

void thread_run_user_fun(ThreadStack **ppCurrentMsgStack, base_thread_fun thread_fun, ThreadStruct *pThread, MSGBODY *msg, dave_bool enable_stack);

#endif

