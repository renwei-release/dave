/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_tools.h"
#include "dave_base.h"
#include "base_tools.h"
#include "thread_tools.h"
#include "thread_mem.h"
#include "thread_log.h"

extern ub base_thread_info(s8 *msg_ptr, ub msg_len);

#define THREAD_MEM_MAX 16
#define THREAD_MEM_NAME "THREAD"

static BlockMem _thread_mem[THREAD_MEM_MAX];

static inline void
__thread_mem_init__(void)
{
	static volatile sb __safe_pre_flag__ = 0;

	SAFEPre(__safe_pre_flag__, { block_mem_reset(_thread_mem, THREAD_MEM_MAX); });
}

static inline void *
__thread_malloc__(ub len, ub msg_id, s8 *file, ub line)
{
	__thread_mem_init__();

	return block_malloc(_thread_mem, len, file, line);
}

static inline dave_bool
__thread_free__(void *ptr, ub msg_id, s8 *file, ub line)
{
	__thread_mem_init__();

	return block_free(_thread_mem, ptr, file, line);
}

static inline dave_bool
__thread_memory_at_here__(void *ptr)
{
	if(ptr == NULL)
	{
		return dave_false;
	}

	__thread_mem_init__();

	return block_memory(_thread_mem, ptr, (s8 *)__func__, (ub)__LINE__);
}

static void
_thread_mem_poweroff(s8 *file, ub line, ub len, ub msg_id)
{
	s8 message[16384];
	ub message_index;

	message_index = 0;

	message_index += base_thread_info(&message[message_index], sizeof(message)-message_index);
	message_index += base_mem_info(&message[message_index], sizeof(message)-message_index, dave_false);
	message_index += thread_memory_info(&message[message_index], sizeof(message)-message_index, dave_false);

	message_index += dave_snprintf(&message[message_index], sizeof(message) - message_index,
		"\nThread Limited Memory, %s:%d malloc length:%d msg_id:%d",
		file, line, len, msg_id);

	if(len == 0)
	{
		message_index += dave_snprintf(&message[message_index], sizeof(message) - message_index,
			"\n** Please note that the memory allocation length is 0 **");
	}

	base_power_off(message);
}

static void
_thread_memory_init(void)
{
	__thread_mem_init__();
}

static void
_thread_memory_exit(void)
{
	block_info_write(THREAD_MEM_NAME, _thread_mem);
}

// =====================================================================

void
thread_memory_init(void)
{
	_thread_memory_init();
}

void
thread_memory_exit(void)
{
	_thread_memory_exit();
}

void *
thread_malloc(ub len, ub msg_id, s8 *file, ub line)
{
	u8 *ptr;

	ptr = __thread_malloc__(len, msg_id, file, line);
	if(ptr == NULL)
	{
		_thread_mem_poweroff(file, line, len, msg_id);
	}

	return ptr;
}

dave_bool
thread_free(void *ptr, ub msg_id, s8 *file, ub line)
{
	dave_bool ret;

	if(ptr == NULL)
	{
		return dave_true;
	}

	ret = __thread_free__(ptr, msg_id, file, line);

	if(ret == dave_false)
	{
		THREADLOG("%s:%d free %lx failed!", file, line, ptr);
	}

	return ret;
}

dave_bool
thread_memory_at_here(void *ptr)
{
	return __thread_memory_at_here__(ptr);
}

ub
thread_memory_info(s8 *info_ptr, ub info_len, dave_bool base_flag)
{
	ub info_index = 0;

	info_index += block_info(THREAD_MEM_NAME, _thread_mem, info_ptr, info_len, base_flag, dave_false, 0);

	return info_index;
}

#endif

