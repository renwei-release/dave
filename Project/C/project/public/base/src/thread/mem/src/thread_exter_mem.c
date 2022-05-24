/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "dave_base.h"
#if defined(__DAVE_BASE__)
#include "dave_tools.h"
#include "base_tools.h"

#define THREAD_EXTER_MEM_MAX 4
#define THREAD_EXTER_MEM_NAME "THREAD"

static BlockMem _thread_exter_mem[THREAD_EXTER_MEM_MAX];

// =====================================================================

void
thread_exter_mem_init(void)
{
	block_mem_reset(_thread_exter_mem, THREAD_EXTER_MEM_MAX);
}

void
thread_exter_mem_exit(void)
{
	block_info_write(THREAD_EXTER_MEM_NAME, _thread_exter_mem);
}

void *
thread_exter_malloc(ub len, s8 *file, ub line)
{
	return block_malloc(_thread_exter_mem, len, file, line);
}

dave_bool
thread_exter_free(void *ptr, s8 *file, ub line)
{
	return block_free(_thread_exter_mem, ptr, file, line);
}

dave_bool
thread_exter_memory(void *ptr, s8 *file, ub line)
{
	return block_memory(_thread_exter_mem, ptr, file, line);
}

ub
thread_exter_memory_info(s8 *info_ptr, ub info_len, dave_bool base_flag)
{
	return block_info(THREAD_EXTER_MEM_NAME, _thread_exter_mem, info_ptr, info_len, base_flag, dave_false);
}

#endif

