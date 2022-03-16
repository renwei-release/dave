/*
 * ================================================================================
 * (c) Copyright 2018 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2018.07.09.
 * ================================================================================
 */

#ifndef __THREAD_THREAD_H__
#define __THREAD_THREAD_H__
#include "base_macro.h"
#include "thread_struct.h"

typedef ub (* schedule_thread_fun)(ub thread_index, ThreadId thread_id, s8 *thread_name, ub wakeup_index, dave_bool enable_stack);

void thread_thread_init(schedule_thread_fun schedule_fun);

void thread_thread_exit(void);

dave_bool thread_thread_creat(s8 *thread_name, ub thread_index, ThreadId thread_id, ub thread_number);

void thread_thread_die(ub thread_index);

dave_bool thread_thread_wakeup(ub thread_index);

dave_bool thread_thread_is_main(void);

ThreadId thread_thread_self(ub *wakeup_index, ThreadSync **ppSync);

ThreadSync * thread_thread_sync(ub thread_index, ub wakeup_index);

#endif

