/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_ID_H__
#define __THREAD_ID_H__

#define LOCAL_ID_MASK 0xffff
#define THREAD_INDEX_MASK 0xffff
#define NET_INDEX_MASK 0xffff
#define WAKEUP_INDEX_MASK 0xfff

#define SYNC_FLAG 0x2000000000000000
#define WAKEUP_FLAG 0x4000000000000000
#define REMOTE_FLAG 0x8000000000000000

static inline ThreadId
thread_set_local(ThreadId thread_id, ThreadId local_id)
{
	return (thread_id & 0xffffffffffff0000) | (local_id & LOCAL_ID_MASK);
}
static inline ThreadId
thread_get_local(ThreadId thread_id)
{
	if(thread_id == INVALID_THREAD_ID)
	{
		return INVALID_THREAD_ID;
	}

	return (thread_id & LOCAL_ID_MASK);
}
ThreadId thread_set_thread(ThreadId thread_id, ub thread_index);
ub __thread_get_thread__(ThreadId thread_id, s8 *fun, ub line);
#define thread_get_thread(thread_id) __thread_get_thread__(thread_id, (s8 *)__func__, (ub)__LINE__)
ThreadId thread_set_net(ThreadId thread_id, ub net_index);
ub __thread_get_net__(ThreadId thread_id, s8 *fun, ub line);
#define thread_get_net(thread_id) __thread_get_net__(thread_id, (s8 *)__func__, (ub)__LINE__)
ThreadId thread_set_wakeup(ThreadId thread_id, ub wakeup_index);
ub thread_get_wakeup(ThreadId thread_id);
ub thread_clean_wakeup(ThreadId thread_id);
ThreadId thread_set_remote(ThreadId local_id, ub thread_index, ub net_index);
void thread_get_remote(ThreadId thread_id, ThreadId *local_id, ub *thread_index, ub *net_index);
ThreadId thread_set_sync(ThreadId thread_id);
dave_bool thread_is_sync(ThreadId thread_id);
dave_bool thread_is_wakeup(ThreadId thread_id);
dave_bool thread_is_remote(ThreadId thread_id);

#endif

