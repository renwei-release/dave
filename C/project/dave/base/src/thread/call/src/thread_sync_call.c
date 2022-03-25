/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_tools.h"
#include "dave_os.h"
#include "thread_tools.h"
#include "thread_parameter.h"
#include "thread_mem.h"
#include "thread_struct.h"
#include "thread_thread.h"
#include "thread_sync.h"
#include "thread_call.h"
#include "thread_log.h"

#define SYNC_FOR_CHECK_MAX (THREAD_MAX + THREAD_THREAD_MAX)
#define SYNC_WAIT_MAX_TIME 90	// second

static TLock _sync_data_for_check_pv;
static ThreadSync * _sync_data_for_check[SYNC_FOR_CHECK_MAX];
static ThreadId _syncc_thread_id = INVALID_THREAD_ID;

static inline void
_thread_sync_call_check_reset(ub check_index)
{
	_sync_data_for_check[check_index] = NULL;
}

static inline void
_thread_sync_call_check_reset_all(void)
{
	ub check_index;

	for(check_index=0; check_index<SYNC_FOR_CHECK_MAX; check_index++)
	{
		_thread_sync_call_check_reset(check_index);
	}
}

static inline void
_thread_sync_call_check_add(ThreadSync *pSync)
{
	ub check_index;

	for(check_index=0; check_index<SYNC_FOR_CHECK_MAX; check_index++)
	{
		if(_sync_data_for_check[check_index] == NULL)
		{
			_sync_data_for_check[check_index] = pSync;

			pSync->wait_time = dave_os_time_s();

			return;
		}
	}

	THREADLOG("failed!");
}

static inline void
_thread_sync_call_check_del(ThreadSync *pSync)
{
	ub check_index;

	for(check_index=0; check_index<SYNC_FOR_CHECK_MAX; check_index++)
	{
		if(_sync_data_for_check[check_index] == pSync)
		{
			_thread_sync_call_check_reset(check_index);
			return;
		}
	}
}

static inline void
_thread_sync_call_check(void)
{
	ub current_second = dave_os_time_s();
	ub check_index;
	ThreadSync *pSync;

	for(check_index=0; check_index<SYNC_FOR_CHECK_MAX; check_index++)
	{
		pSync = _sync_data_for_check[check_index];
		if((pSync != NULL) && (pSync->wait_thread != INVALID_THREAD_ID))
		{
			if((current_second - pSync->wait_time) > SYNC_WAIT_MAX_TIME)
			{
				THREADLOG("wait_name:%s wait_thread:%s/%lx wait_msg:%d wait_time:%d->%d",
					pSync->wait_name,
					thread_name(pSync->wait_thread), pSync->wait_thread,
					pSync->wait_msg,
					current_second, pSync->wait_time);

				t_unlock_mutex(&(pSync->sync_pv));
			}
		}
	}
}

static inline ThreadSync *
_thread_sync_call_load_sync(ThreadStruct *pThread, ub wakeup_index)
{
	ThreadSync *pSync;

	if((pThread->thread_flag & THREAD_THREAD_FLAG) == 0)
	{
		pSync = &(pThread->sync);
	}
	else
	{
		pSync = thread_thread_sync(pThread->thread_index, wakeup_index);
	}

	return pSync;
}

static inline ub
_thread_sync_get_wakeup_index(ThreadId thread_id)
{
	return thread_get_wakeup(thread_id);
}

static inline ThreadId
_thread_sync_set_index(ThreadId thread_id, ub *wakeup_index)
{
	if(thread_thread_self(wakeup_index, NULL) == thread_id)
	{
		return thread_set_wakeup(thread_id, *wakeup_index);
	}
	else
	{
		*wakeup_index = 0;

		return thread_set_wakeup(thread_id, *wakeup_index);
	}
}

static inline void
_thread_sync_call_check_add_safe(ThreadSync *pSync)
{
	SAFEZONEv3(_sync_data_for_check_pv, _thread_sync_call_check_add(pSync); );
}

static inline void
_thread_sync_call_check_del_safe(ThreadSync *pSync)
{
	SAFEZONEv3(_sync_data_for_check_pv, _thread_sync_call_check_del(pSync); );
}

static inline void
_thread_sync_call_check_safe(void)
{
	SAFEZONEv3(_sync_data_for_check_pv, _thread_sync_call_check(); );
}

// =====================================================================

void
thread_sync_call_init(void)
{
	t_lock_reset(&_sync_data_for_check_pv);

	_thread_sync_call_check_reset_all();
}

void
thread_sync_call_exit(void)
{

}

void
thread_sync_call_check(void)
{
	_thread_sync_call_check_safe();
}

ThreadSync *
thread_sync_call_step_1_pre(ThreadStruct *pSrcThread, ThreadId *sync_src_id, ThreadStruct *pDstThread, ub wait_msg, u8 *wait_body, ub wait_len)
{
	ub wakeup_index;
	ThreadSync *pSync;
	ub safe_counter;

	if(pSrcThread == NULL)
	{
		THREADABNOR("src thread is NULL!");
		return NULL;
	}

	if(pDstThread == NULL)
	{
		THREADABNOR("dst thread is NULL!");
		return NULL;
	}

	*sync_src_id = _thread_sync_set_index(*sync_src_id, &wakeup_index);

	pSync = _thread_sync_call_load_sync(pSrcThread, wakeup_index);
	if(pSync == NULL)
	{
		THREADABNOR("%s can't find sync on wait_msg:%d!", pSrcThread->thread_name, wait_msg);
		return NULL;
	}

	safe_counter = 0;

	while(t_trylock_mutex(&(pSync->sync_pv)) == dave_false)
	{
		if((++ safe_counter) > 1024)
		{
			THREADLOG("Only one can be locked at a time:%s->%s:%d",
				pDstThread->thread_name, pSrcThread->thread_name, wait_msg);
			return NULL;
		}

		dave_os_sleep(10);
	}

	dave_strcpy(pSync->wait_name, thread_name(pDstThread->thread_id), THREAD_NAME_MAX);
	pSync->wait_thread = pDstThread->thread_id;
	pSync->wait_msg = wait_msg;
	pSync->wait_body = wait_body;
	pSync->wait_len = wait_len;

	_thread_sync_call_check_add_safe(pSync);

	return pSync;
}

void *
thread_sync_call_step_2_wait(ThreadStruct *pSrcThread, ThreadStruct *pDstThread, ThreadSync *pSync)
{
	dave_bool wait_timerout = dave_false;
	ub wait_start_time;

	wait_start_time = dave_os_time_us();

#ifdef __BASE_ALPHA_VERSION__
	/*
	 * 在处理同步消息时，不用做同线程资源竞争检测。
	 */
	pSync->sync_pv.thread_id = -1;
#endif

	SAFEZONEv3(pSync->sync_pv, {

		if(pSync->wait_thread != INVALID_THREAD_ID)
		{
			THREADABNOR("%s wait %s gave msg %d time out(%d) current wait:%s/%lx",
				pSrcThread->thread_name, pDstThread->thread_name,
				pSync->wait_msg,
				dave_os_time_us() - wait_start_time,
				thread_name(pSync->wait_thread), pSync->wait_thread);

			dave_memset(pSync->wait_body, 0x00, pSync->wait_len);

			wait_timerout = dave_true;
		}

		dave_memset(pSync->wait_name, 0x00, THREAD_NAME_MAX);
		pSync->wait_thread = INVALID_THREAD_ID;
		pSync->wait_msg = MSGID_INVALID;

	} );

	_thread_sync_call_check_del_safe(pSync);

	if(wait_timerout == dave_true)
	{
		return NULL;
	}

	return pSync->wait_body;
}

dave_bool
thread_sync_call_step_3_catch(ThreadStruct *pDstThread, ThreadId dst_id, ThreadId wait_thread, ub wait_msg, void *catch_body, ub catch_len)
{
	ThreadSync *pSync;
	ub wakeup_index;
	dave_bool unlock;

	if(_syncc_thread_id == INVALID_THREAD_ID)
	{
		_syncc_thread_id = thread_map_id((s8 *)SYNC_CLIENT_THREAD_NAME);
	}

	wakeup_index = _thread_sync_get_wakeup_index(dst_id);

	if((pDstThread != NULL)
		&& (wakeup_index != INVALID_THREAD_ID)
		&& (pDstThread->attrib == LOCAL_TASK_ATTRIB)
		&& (pDstThread->thread_id != _syncc_thread_id))
	{
		pSync = _thread_sync_call_load_sync(pDstThread, wakeup_index);

		if((pSync != NULL)
			&& ((thread_get_local(pSync->wait_thread) == thread_get_local(wait_thread)) || (thread_id(pSync->wait_name) == thread_get_local(wait_thread)))
			&& (pSync->wait_msg == wait_msg))
		{
			if(pSync->wait_len != catch_len)
			{
				THREADABNOR("catch_len:%d and wait_len:%d is mismatch! thread:%s/%s msg:%d",
					catch_len, pSync->wait_len,
					pDstThread->thread_name, thread_name(wait_thread), wait_msg);
			}
			if(pSync->wait_len > catch_len)
			{
				pSync->wait_len = catch_len;
			}

			dave_memcpy(pSync->wait_body, catch_body, pSync->wait_len);

			unlock = (pSync->wait_thread != INVALID_THREAD_ID) ? dave_true : dave_false;

			pSync->wait_thread = INVALID_THREAD_ID;
			pSync->wait_msg = MSGID_INVALID;

			if(unlock == dave_true)
			{
				t_unlock_mutex(&(pSync->sync_pv));
			}

			return dave_true;
		}

		THREADLOG("The wakeup_index:%d, thread:%s/%d/%d/%d should be a sync message:%lx/%x, but not captured by any thread! wait_thread:%lx/%lx wait_msg:%d/%d",
			wakeup_index,
			pDstThread->thread_name, pDstThread->attrib, pDstThread->thread_id, _syncc_thread_id,
			dst_id, pSync,
			pSync!=NULL?pSync->wait_thread:0, wait_thread,
			pSync!=NULL?pSync->wait_msg:0, wait_msg);
	}

	return dave_false;
}

#endif

