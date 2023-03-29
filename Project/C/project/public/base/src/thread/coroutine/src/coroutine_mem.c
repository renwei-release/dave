/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_parameter.h"
#if defined(__DAVE_BASE__) && defined(ENABLE_THREAD_COROUTINE)
#include "dave_base.h"
#include "dave_tools.h"
#include "thread_log.h"

// #define CO_MEM_DEBUG

#ifdef CO_MEM_DEBUG
#define CO_MEM_MAX_LENGTH 256
#define CO_MEM_MAX_NUMBER 4096

static ub _cocore_buffer_index = 0;
static s8 _cocore_buffer_ptr[CO_MEM_MAX_NUMBER][CO_MEM_MAX_LENGTH];
#endif

// =====================================================================

void
coroutine_mem_init(void)
{
#ifdef CO_MEM_DEBUG
	_cocore_buffer_index = 0;
	dave_memset(_cocore_buffer_ptr, 0x00, sizeof(_cocore_buffer_ptr));
#endif
}

void
coroutine_mem_exit(void)
{

}

void *
coroutine_malloc(ub length)
{
#ifdef CO_MEM_DEBUG
	void *ptr;

	THREADLOG("length:%d", length);

	t_lock;
	ptr = _cocore_buffer_ptr[_cocore_buffer_index ++];
	t_unlock;

	return ptr;
#else
	return dave_malloc(length);
#endif
}

void
coroutine_free(void *ptr)
{
#ifndef CO_MEM_DEBUG
	dave_free(ptr);
#endif
}

#endif

