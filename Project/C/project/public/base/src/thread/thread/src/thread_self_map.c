/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "thread_self_map.h"

static void *_thread_self_map_kv = NULL;

static inline void
_thread_self_map_init(void)
{
	static volatile sb __safe_pre_flag__ = 0;

	SAFEPre(__safe_pre_flag__, {
		if(_thread_self_map_kv == NULL)
		{
			_thread_self_map_kv = kv_malloc("tsmkv", 0, NULL);
		}
	} );
}

// =====================================================================

void
thread_self_map_init(void)
{
	_thread_self_map_init();
}

void
thread_self_map_exit(void)
{
	kv_free(_thread_self_map_kv, NULL);
	_thread_self_map_kv = NULL;
}

void
thread_self_map_add(pthread_t self, ThreadThread *pTThread)
{
	_thread_self_map_init();

	kv_add_ub_ptr(_thread_self_map_kv, (ub)self, pTThread);
}

ThreadThread *
thread_self_map_inq(pthread_t self)
{
	_thread_self_map_init();

	return (ThreadThread *)kv_inq_ub_ptr(_thread_self_map_kv, (ub)self);
}

ThreadThread *
thread_self_map_del(pthread_t self)
{
	_thread_self_map_init();

	return kv_del_ub_ptr(_thread_self_map_kv, (ub)self);
}

#endif

