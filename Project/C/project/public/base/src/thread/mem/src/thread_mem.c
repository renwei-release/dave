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
#include "thread_tools.h"
#include "thread_mem.h"
#include "thread_exter_mem.h"
#include "thread_log.h"

extern ub base_thread_info(s8 *msg_ptr, ub msg_len);

static dave_bool _thread_mem_init_flag_ = dave_false;

static void *
__thread_malloc__(ub len, ub msg_id, s8 *file, ub line)
{
	void *ptr;

	ptr = thread_exter_malloc(len, file, line);

	return ptr;
}

static dave_bool
__thread_free__(void *ptr, ub msg_id, s8 *file, ub line)
{
	dave_bool ret;

	ret = thread_exter_free(ptr, file, line);

	return ret;
}

static inline dave_bool
__thread_memory_at_here__(void *ptr)
{
	if(ptr == NULL)
	{
		return dave_false;
	}

	if(thread_exter_memory(ptr, (s8 *)__func__, (ub)__LINE__) == dave_true)
	{
		return dave_true;
	}
	else
	{
		return dave_false;
	}
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
	thread_exter_mem_init();
}

static void
_thread_memory_exit(void)
{
	thread_exter_mem_exit();
}

// =====================================================================

void
thread_memory_init(void)
{
	dave_bool init_flag = dave_false;

	thread_lock();
	if(_thread_mem_init_flag_ == dave_false)
	{
		_thread_mem_init_flag_ = dave_true;
		init_flag = dave_true;	
	}
	thread_unlock();

	if(init_flag == dave_true)
	{
		_thread_memory_init();
	}
}

void
thread_memory_exit(void)
{
	dave_bool exit_flag = dave_false;

	thread_lock();
	if(_thread_mem_init_flag_ == dave_true)
	{
		_thread_mem_init_flag_ = dave_false;
		exit_flag = dave_true;	
	}
	thread_unlock();

	if(exit_flag == dave_true)
	{
		_thread_memory_exit();
	}
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

	info_index += thread_exter_memory_info(&info_ptr[info_index], info_len-info_index, base_flag);

	return info_index;
}

#endif

