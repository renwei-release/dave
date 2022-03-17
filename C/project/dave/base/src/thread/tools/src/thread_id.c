/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_tools.h"
#include "dave_verno.h"
#include "thread_tools.h"
#include "thread_log.h"

#define LOCAL_ID_MASK 0xffff
#define THREAD_INDEX_MASK 0xffff
#define NET_INDEX_MASK 0xffff
#define WAKEUP_INDEX_MASK 0xfff

#define SYNC_FLAG 0x2000000000000000
#define WAKEUP_FLAG 0x4000000000000000
#define REMOTE_FLAG 0x8000000000000000

#define SET_FLAG _thread_set_flag()

typedef union {
	ThreadId thread_id;
	struct {
		ThreadId local_id:16;							/* 线程的本地ID */
		ThreadId thread_index:16;						/* 线程链路索引 */
		ThreadId net_index:16;							/* 传输链路索引 */
		ThreadId wakeup_index:15;						/* 唤醒索引，用于同步消息 */
		ThreadId reserve_1:1;
		ThreadId sync_flag:1;							/* SYNC转发消息的标记 */
		ThreadId wakeup_flag:1;							/* 唤醒消息的标记 */
		ThreadId remote_flag:1;							/* 远程消息的标记 */
	} bit;
} ThreadIdUnion;

static inline dave_bool
_thread_sync_flag(void)
{
	s8 *product = dave_verno_product(NULL, NULL, 0);

	if(dave_strcmp(product, "SYNC") == dave_true)
	{
		return dave_true;
	}
	else
	{
		return dave_false;
	}
}

static inline ThreadId
_thread_set_flag(void)
{
	static ThreadId flag = 0;

	if(flag == 0)
	{
		if(_thread_sync_flag() == dave_true)
		{
			flag = (SYNC_FLAG | REMOTE_FLAG);
		}
		else
		{
			flag = (REMOTE_FLAG);
		}
	}

	return flag;
}

// =====================================================================

ThreadId
thread_set_local(ThreadId thread_id, ThreadId local_id)
{
	return (thread_id & 0xffffffffffff0000) | (local_id & LOCAL_ID_MASK);
}

ThreadId
thread_get_local(ThreadId thread_id)
{
	if(thread_id == INVALID_THREAD_ID)
	{
		return INVALID_THREAD_ID;
	}

	return (thread_id & LOCAL_ID_MASK);
}

ThreadId
thread_set_thread(ThreadId thread_id, ub thread_index)
{
	return (thread_id & 0xffffffff0000ffff) | ((thread_index & THREAD_INDEX_MASK) << 16) | SET_FLAG;
}

ub
__thread_get_thread__(ThreadId thread_id, s8 *fun, ub line)
{
	if(thread_id == INVALID_THREAD_ID)
	{
		THREADLOG("invalid thread_id:%x <%s:%d>", thread_id, fun, line);
		return INVALID_THREAD_ID;
	}

	if(thread_is_remote(thread_id) == dave_false)
	{
		return INVALID_THREAD_ID;
	}

	return ((thread_id >> 16) & THREAD_INDEX_MASK);
}

ThreadId
thread_set_net(ThreadId thread_id, ub net_index)
{
	return (thread_id & 0xffff0000ffffffff) | ((net_index & NET_INDEX_MASK) << 32) | SET_FLAG;
}

ub
__thread_get_net__(ThreadId thread_id, s8 *fun, ub line)
{
	if(thread_id == INVALID_THREAD_ID)
	{
		THREADLOG("invalid thread_id:%x <%s:%d>", thread_id, fun, line);
		return INVALID_THREAD_ID;
	}

	if(thread_is_remote(thread_id) == dave_false)
	{
		return INVALID_THREAD_ID;
	}

	return ((thread_id >> 32) & NET_INDEX_MASK);
}

ThreadId
thread_set_wakeup(ThreadId thread_id, ub wakeup_index)
{
	if(wakeup_index >= WAKEUP_INDEX_MASK)
	{
		THREADLOG("invalid wakeup_index:%lx", wakeup_index);
	}

	return (thread_id & 0xf000ffffffffffff) | ((wakeup_index << 48) & 0x0fff000000000000) | WAKEUP_FLAG;
}

ub
thread_get_wakeup(ThreadId thread_id)
{
	if(thread_id == INVALID_THREAD_ID)
	{
		THREADLOG("invalid thread_id:%x", thread_id);
		return INVALID_THREAD_ID;
	}

	if(thread_is_wakeup(thread_id) == dave_false)
	{
		return INVALID_THREAD_ID;
	}

	return (thread_id >> 48) & WAKEUP_INDEX_MASK;
}

ub
thread_clean_wakeup(ThreadId thread_id)
{
	return (thread_id & 0xb000ffffffffffff);
}

ThreadId
thread_set_remote(ThreadId thread_id, ThreadId local_id, ub thread_index, ub net_index)
{
	thread_id = thread_set_local(thread_id, local_id);
	thread_id = thread_set_thread(thread_id, thread_index);
	thread_id = thread_set_net(thread_id, net_index);
	if(thread_is_wakeup(local_id) == dave_true)
	{
		thread_id = thread_id | (local_id & 0x0fff000000000000) | WAKEUP_FLAG;
	}

	return thread_id;
}

void
thread_get_remote(ThreadId thread_id, ThreadId *local_id, ub *thread_index, ub *net_index)
{
	if(local_id != NULL)
	{
		*local_id = thread_get_local(thread_id);
	}
	if(thread_index != NULL)
	{
		*thread_index = thread_get_thread(thread_id);
	}
	if(net_index != NULL)
	{
		*net_index = thread_get_net(thread_id);
	}
}

ThreadId
thread_set_sync(ThreadId thread_id)
{
	return (thread_id | SYNC_FLAG);
}

dave_bool
thread_is_sync(ThreadId thread_id)
{
	if((thread_id & SYNC_FLAG) == 0x00)
	{
		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

dave_bool
thread_is_wakeup(ThreadId thread_id)
{
	if((thread_id & WAKEUP_FLAG) == 0x00)
	{
		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

dave_bool
thread_is_remote(ThreadId thread_id)
{
	if((thread_id & REMOTE_FLAG) == 0x00)
	{
		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

#endif

