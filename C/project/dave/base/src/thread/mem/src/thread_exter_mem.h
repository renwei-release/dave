/*
 * ================================================================================
 * (c) Copyright 2020 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.01.11.
 * ================================================================================
 */

#ifndef __THREAD_EXTER_MEM_H__
#define __THREAD_EXTER_MEM_H__
#include "base_define.h"

void thread_exter_mem_init(void);

void thread_exter_mem_exit(void);

void * thread_exter_malloc(ub len, s8 *file, ub line);

dave_bool thread_exter_free(void *ptr, s8 *file, ub line);

dave_bool thread_exter_memory(void *ptr, s8 *file, ub line);

ub thread_exter_memory_info(s8 *info, ub info_len, dave_bool base_flag);

#endif

