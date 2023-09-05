/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_THREAD_H__
#define __THREAD_THREAD_H__
#include "base_macro.h"
#include "thread_struct.h"

typedef ub (* schedule_thread_fun)(void *pTThread, ThreadId thread_id, s8 *thread_name, ub wakeup_index, dave_bool enable_stack);

void thread_thread_init(schedule_thread_fun schedule_fun);

void thread_thread_exit(void);

dave_bool thread_thread_creat(s8 *thread_name, ub thread_index, ThreadId thread_id, ub thread_number);

void thread_thread_die(ub thread_index);

dave_bool thread_thread_wakeup(ub thread_index);

dave_bool thread_thread_is_main(void);

ThreadId thread_thread_self(ub *wakeup_index);

ThreadChain * thread_thread_chain(void);

ThreadRouter * thread_thread_router(void);

ThreadSync * thread_thread_sync(ub thread_index, ub wakeup_index);

void __thread_thread_write__(void *msg_chain, void *msg_router, ub thread_index, ub wakeup_index, ub msg_id, ub msg_len, u8 *msg_body, s8 *fun, ub line);
#define thread_thread_write(msg_chain, msg_router, thread_index, wakeup_index, msg_id, msg_body) __thread_thread_write__(msg_chain, msg_router, thread_index, wakeup_index, msg_id, sizeof(*msg_body), (void *)msg_body, (s8 *)__func__, (ub)__LINE__)

ThreadMsg * thread_thread_read(void *param);

void thread_thread_set_coroutine_site(ub thread_index, ub wakeup_index, void *point);

void * thread_thread_get_coroutine_site(ub thread_index, ub wakeup_index);

void thread_thread_clean_coroutine_site(ub thread_index, ub wakeup_index);

ub thread_thread_total_number(ub thread_index);

void thread_thread_total_detail(ub *unprocessed, ub *received, ub *processed, ub thread_index);

#endif

