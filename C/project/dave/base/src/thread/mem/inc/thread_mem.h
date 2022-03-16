/*
 * ================================================================================
 * (c) Copyright 2017 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2017.08.22.
 * ================================================================================
 */

#ifndef __THREAD_MEM_H__
#define __THREAD_MEM_H__
#include "base_define.h"

void thread_memory_init(void);

void thread_memory_exit(void);

void * thread_malloc(ub len, ub msg_id, s8 *file, ub line);

dave_bool thread_free(void *ptr, ub msg_id, s8 *file, ub line);

dave_bool thread_memory_at_here(void *ptr);

ub thread_memory_info(s8 *info, ub info_len, dave_bool base_flag);

#endif

