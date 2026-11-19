/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include <errno.h>
#include "dave_tools.h"
#include "dave_os.h"
#include "thread_tools.h"
#include "thread_parameter.h"
#include "thread_mem.h"
#include "thread_struct.h"
#include "thread_thread.h"
#include "thread_sync.h"
#include "thread_call.h"
#include "thread_sync_call.h"
#include "thread_running.h"
#include "thread_log.h"

static inline dave_bool
_thread_sync_deadline_expired(const struct timespec *deadline)
{
	struct timespec now;

	if(clock_gettime(CLOCK_MONOTONIC, &now) != 0)
	{
		THREADABNOR("sync clock read failed:%d", errno);
		return dave_true;
	}

	return ((now.tv_sec > deadline->tv_sec)
		|| ((now.tv_sec == deadline->tv_sec) && (now.tv_nsec >= deadline->tv_nsec)))
		? dave_true : dave_false;
}

static inline ThreadSync *
_thread_sync_call_load_sync(ThreadStruct *pThread, ub wakeup_index)
{
	if((pThread->thread_flag & THREAD_THREAD_FLAG) == 0)
		return &(pThread->sync);

	return thread_thread_sync(pThread->thread_index, wakeup_index);
}

static inline ThreadId
_thread_sync_set_index(ThreadId thread_id, ub *wakeup_index)
{
	if(thread_thread_self(wakeup_index) != thread_id)
		*wakeup_index = 0;

	return thread_set_wakeup(thread_id, *wakeup_index);
}

static inline void
_thread_sync_call_release(ThreadSync *pSync)
{
	thread_reset_sync(pSync);
	pthread_cond_broadcast(&(pSync->sync_cond));
}

// =====================================================================

void
thread_sync_call_init(void)
{
}

void
thread_sync_call_exit(void)
{
}

void
thread_sync_call_check(void)
{
	/* Each waiter enforces its own deadline without a guardian wakeup. */
}

void *
thread_sync_call_step_1_pre(ThreadStruct *pSrcThread, ThreadId *sync_src_id, ThreadStruct *pDstThread, ub wait_msg, u8 *wait_body, ub wait_len)
{
	ub wakeup_index;
	ThreadSync *pSync;
	struct timespec deadline;
	int ret;

	if((pSrcThread == NULL) || (pDstThread == NULL))
	{
		THREADABNOR("sync source or destination thread is NULL!");
		return NULL;
	}

	if(clock_gettime(CLOCK_MONOTONIC, &deadline) != 0)
	{
		THREADABNOR("sync clock read failed:%d", errno);
		return NULL;
	}
	deadline.tv_sec += thread_running_cfg_life();

	*sync_src_id = _thread_sync_set_index(*sync_src_id, &wakeup_index);
	pSync = _thread_sync_call_load_sync(pSrcThread, wakeup_index);
	if((pSync == NULL) || (pSync->sync_cond_ready == dave_false))
	{
		THREADABNOR("%s has no initialized sync on wait_msg:%d!", pSrcThread->thread_name, wait_msg);
		return NULL;
	}

	/* Use the raw mutex with cond_wait; TLock owner tracking cannot follow its unlock/relock. */
	pthread_mutex_lock(&(pSync->sync_pv.m_mutex_t));
	while(pSync->sync_busy == dave_true)
	{
		ret = pthread_cond_timedwait(&(pSync->sync_cond), &(pSync->sync_pv.m_mutex_t), &deadline);
		if(ret != 0)
		{
			THREADABNOR("sync slot wait failed:%d %s->%s:%d",
				ret, pSrcThread->thread_name, pDstThread->thread_name, wait_msg);
			pthread_mutex_unlock(&(pSync->sync_pv.m_mutex_t));
			return NULL;
		}
	}

	if(_thread_sync_deadline_expired(&deadline) == dave_true)
	{
		THREADABNOR("sync preparation time out:%s->%s:%d",
			pSrcThread->thread_name, pDstThread->thread_name, wait_msg);
		pthread_mutex_unlock(&(pSync->sync_pv.m_mutex_t));
		return NULL;
	}

	pSync->sync_busy = dave_true;
	pSync->deadline = deadline;
	dave_strcpy(pSync->wait_name, thread_name(pDstThread->thread_id), THREAD_NAME_MAX);
	pSync->wait_thread = pDstThread->thread_id;
	pSync->wait_msg = wait_msg;
	pSync->wait_body = wait_body;
	pSync->wait_len = wait_len;
	pthread_mutex_unlock(&(pSync->sync_pv.m_mutex_t));

	return pSync;
}

void
thread_sync_call_step_1_clean(void *param)
{
	ThreadSync *pSync = (ThreadSync *)param;

	if(pSync == NULL)
		return;

	pthread_mutex_lock(&(pSync->sync_pv.m_mutex_t));
	_thread_sync_call_release(pSync);
	pthread_mutex_unlock(&(pSync->sync_pv.m_mutex_t));
}

void *
thread_sync_call_step_2_wait(ThreadStruct *pSrcThread, ThreadStruct *pDstThread, void *param)
{
	ThreadSync *pSync = (ThreadSync *)param;
	void *ret_body = NULL;
	int ret = 0;
	ub start_time, end_time;

	pthread_mutex_lock(&(pSync->sync_pv.m_mutex_t));
	if(pSync->sync_busy == dave_false)
	{
		THREADABNOR("sync wait without a reservation:%s->%s",
			pSrcThread->thread_name, pDstThread->thread_name);
		pthread_mutex_unlock(&(pSync->sync_pv.m_mutex_t));
		return NULL;
	}

	start_time = dave_os_time_s();

	while((pSync->wait_thread != INVALID_THREAD_ID) && (ret == 0))
	{
		ret = pthread_cond_timedwait(&(pSync->sync_cond), &(pSync->sync_pv.m_mutex_t), &(pSync->deadline));
	}

	if((pSync->wait_thread == INVALID_THREAD_ID) && ((ret == 0) || (ret == ETIMEDOUT)))
	{
		ret_body = pSync->wait_body;
	}
	else
	{
		end_time = dave_os_time_s();
	
		THREADABNOR("%s wait %s gave msg:%s sync wait failed:%d wait_time:%ds",
			pSrcThread->thread_name, pDstThread->thread_name,
			msgstr(pSync->wait_msg), ret, end_time - start_time);
		dave_memset(pSync->wait_body, 0x00, pSync->wait_len);
	}

	_thread_sync_call_release(pSync);
	pthread_mutex_unlock(&(pSync->sync_pv.m_mutex_t));

	return ret_body;
}

dave_bool
thread_sync_call_step_3_catch(ThreadStruct *pDstThread, ThreadId dst_id, ThreadId wait_thread, ub wait_msg, void *wait_body, ub wait_len)
{
	ThreadSync *pSync;
	ub wakeup_index;
	dave_bool caught = dave_false;

	wakeup_index = thread_get_wakeup(dst_id);
	if((pDstThread == NULL)
		|| (wakeup_index == INVALID_THREAD_ID)
		|| (pDstThread->attrib != LOCAL_TASK_ATTRIB)
		|| (pDstThread->thread_id == thread_map_id(SYNC_CLIENT_THREAD_NAME)))
		return dave_false;

	pSync = _thread_sync_call_load_sync(pDstThread, wakeup_index);
	if((pSync == NULL) || (pSync->sync_cond_ready == dave_false))
		return dave_false;

	pthread_mutex_lock(&(pSync->sync_pv.m_mutex_t));
	if((pSync->sync_busy == dave_true)
		&& (pSync->wait_thread != INVALID_THREAD_ID)
		&& (pSync->wait_msg == wait_msg)
		&& ((thread_get_local(pSync->wait_thread) == thread_get_local(wait_thread))
			|| (thread_id(pSync->wait_name) == thread_get_local(wait_thread)))
		&& (_thread_sync_deadline_expired(&(pSync->deadline)) == dave_false))
	{
		if(pSync->wait_len != wait_len)
		{
			THREADABNOR("wait_len:%d and sync wait_len:%d is mismatch! thread:%s/%s msg:%s",
				wait_len, pSync->wait_len,
				pDstThread->thread_name, thread_name(wait_thread), msgstr(wait_msg));
		}
		if(pSync->wait_len > wait_len)
			pSync->wait_len = wait_len;

		dave_memcpy(pSync->wait_body, wait_body, pSync->wait_len);
		pSync->wait_thread = INVALID_THREAD_ID;
		pSync->wait_msg = MSGID_INVALID;
		pthread_cond_broadcast(&(pSync->sync_cond));
		caught = dave_true;
	}
	pthread_mutex_unlock(&(pSync->sync_pv.m_mutex_t));

	return caught;
}

#endif
