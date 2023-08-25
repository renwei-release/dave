/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "dave_tools.h"
#include "thread_tools.h"
#include "thread_msg_buffer.h"
#include "thread_log.h"

static void *_pKV = NULL;

static inline s8 *
_thread_gid_key(s8 *key_ptr, ub key_len, s8 *gid, s8 *thread_name)
{
	dave_snprintf(key_ptr, key_len, "%s-%s", gid, thread_name);
	return key_ptr;
}

// =====================================================================

void
thread_gid_table_init(void)
{
	_pKV = kv_malloc((s8 *)"threadgidtable", 0, NULL);
}

void
thread_gid_table_exit(void)
{
	if(_pKV != NULL)
	{
		kv_free(_pKV, NULL);
		_pKV = NULL;
	}
}

void
thread_gid_table_add(s8 *gid, s8 *thread_name, ThreadId remote_id)
{
	s8 key[1024];

	if((gid[0] == '\0') || (thread_name[0] == '\0'))
	{
		THREADLOG("empty gid:%s or thread_name:%s", gid, thread_name);
		return;
	}

	kv_add_key_ptr(_pKV, _thread_gid_key(key, sizeof(key), gid, thread_name), (void *)remote_id);

	thread_msg_buffer_gid_pop(gid, thread_name);
}

void
thread_gid_table_del(s8 *gid, s8 *thread_name)
{
	s8 key[1024];

	kv_del_key_ptr(_pKV, _thread_gid_key(key, sizeof(key), gid, thread_name));
}

ThreadId
thread_gid_table_inq(s8 *gid, s8 *thread_name)
{
	s8 key[1024];
	void *ptr;

	if((gid[0] == '\0') || (thread_name[0] == '\0'))
	{
		THREADLOG("empty gid:%s or thread_name:%s", gid, thread_name);
		return INVALID_THREAD_ID;
	}

	ptr = kv_inq_key_ptr(_pKV, _thread_gid_key(key, sizeof(key), gid, thread_name));
	if(ptr == NULL)
	{
		return INVALID_THREAD_ID;
	}

	return (ThreadId)ptr;
}

#endif

