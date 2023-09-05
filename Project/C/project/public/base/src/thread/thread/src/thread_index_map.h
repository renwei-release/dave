/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_INDEX_MAP_H__
#define __THREAD_INDEX_MAP_H__
#include "base_macro.h"
#include "thread_thread_param.h"

void thread_index_map_init(void);
void thread_index_map_exit(void);

void thread_index_thread_add(ThreadThread *pTThread);
ThreadThread * thread_index_thread_inq(ub thread_index, ub wakeup_index);
ThreadThread * thread_index_thread_del(ThreadThread *pTThread);

void thread_index_list_add(ThreadThread *pTThread);
void thread_index_list_del(ThreadThread *pTThread);
__ThreadThreadList__ * thread_index_list_inq(ub thread_index);
ThreadThread * thread_index_list_loop(ub thread_index);

#endif

