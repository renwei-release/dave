/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include <pthread.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_verno.h"
#include "thread_struct.h"
#include "thread_thread.h"
#include "thread_parameter.h"
#include "thread_quit.h"
#include "thread_tools.h"
#include "thread_tools.h"
#include "thread_log.h"

typedef enum {
	ThreadState_INIT,
	ThreadState_RUNNING,
	ThreadState_SLEEP,
	ThreadState_STOPPED,
	ThreadState_MAX,
} ThreadState;

typedef struct {
	s8 thread_name[THREAD_NAME_MAX];
	ub thread_index;
	ThreadId thread_id;

	volatile ThreadState state;

	pthread_t thr_id;
	pthread_mutex_t m_mutex_t;
	pthread_cond_t  m_cond_t;

	ub wakeup_index;

	ThreadSync sync;
} ThreadThread;

typedef struct {
	pthread_t self;
	ThreadThread *pTThread;
} ThreadSelfMap;

typedef struct {
	volatile ub current_wakeup_tthread_index;
	ThreadThread *pTThread[THREAD_THREAD_MAX];
} ThreadIndexMap;

static pthread_t _main_thread = (pthread_t)NULL;

static ThreadThread _thread_thread[THREAD_THREAD_MAX];
static ThreadSelfMap _self_map[THREAD_THREAD_MAX];
static ThreadIndexMap _index_map[THREAD_MAX];
static schedule_thread_fun _schedule_thread_fun = NULL;
static TLock _thread_thread_pv;

static void
_tthread_reset(ThreadThread *pTThread)
{
	dave_memset(pTThread->thread_name, 0x00, sizeof(pTThread->thread_name));
	pTThread->thread_index = THREAD_MAX;
	pTThread->thread_id = INVALID_THREAD_ID;

	pTThread->state = ThreadState_INIT;

	pTThread->wakeup_index = 0;

	thread_reset_sync(&(pTThread->sync));
}

static void
_tthread_reset_all(void)
{
	ub tthread_index;

	for(tthread_index=0; tthread_index<THREAD_THREAD_MAX; tthread_index++)
	{		
		dave_memset(&_thread_thread[tthread_index], 0x00, sizeof(ThreadThread));

		t_lock_reset(&(_thread_thread[tthread_index].sync.sync_pv));

		_tthread_reset(&_thread_thread[tthread_index]);
	}
}

static void
_tthread_reset_self_map(ThreadSelfMap *pMap)
{
	if(pMap != NULL)
	{
		dave_memset(pMap, 0x00, sizeof(ThreadSelfMap));
	}
}

static void
_tthread_reset_self_map_all(void)
{
	ub map_index;

	for(map_index=0; map_index<THREAD_THREAD_MAX; map_index++)
	{
		_tthread_reset_self_map(&_self_map[map_index]);
	}
}

static void
_tthread_reset_index_map(ThreadIndexMap *pMap)
{
	if(pMap != NULL)
	{
		dave_memset(pMap, 0x00, sizeof(ThreadIndexMap));

		pMap->current_wakeup_tthread_index = 0;
	}
}

static void
_tthread_reset_index_map_all(void)
{
	ub map_index;

	for(map_index=0; map_index<THREAD_MAX; map_index++)
	{
		_tthread_reset_index_map(&_index_map[map_index]);
	}
}

static void
_tthread_setup_thread_name(ThreadThread *pTThread)
{
	prctl(PR_SET_NAME, pTThread->thread_name);
}

static void
_tthread_sleep_thread(ThreadThread *pTThread)
{
	pthread_mutex_lock(&(pTThread->m_mutex_t));
	if(pTThread->state == ThreadState_RUNNING)
	{
		pTThread->state = ThreadState_SLEEP;

		while(pTThread->state == ThreadState_SLEEP)
		{
			pthread_cond_wait(&(pTThread->m_cond_t), &(pTThread->m_mutex_t));
		}
	}
	pthread_mutex_unlock(&(pTThread->m_mutex_t));
}

static dave_bool
_tthread_wakeup_thread(ThreadThread *pTThread)
{
	pthread_mutex_lock(&(pTThread->m_mutex_t));
	pTThread->state = ThreadState_RUNNING;
	pthread_mutex_unlock(&(pTThread->m_mutex_t));

	if(pthread_cond_broadcast(&(pTThread->m_cond_t)) < 0)
		return dave_false;
	else
		return dave_true;
}

static void
_tthread_running_thread(ThreadThread *pTThread)
{
	sb loop_counter;

	if(_schedule_thread_fun != NULL)
	{
		loop_counter = _schedule_thread_fun(pTThread->thread_index, pTThread->thread_id, pTThread->thread_name, pTThread->wakeup_index, dave_false);

		while(((loop_counter --) >= 0) && (pTThread->state == ThreadState_RUNNING))
		{
			_schedule_thread_fun(pTThread->thread_index, pTThread->thread_id, pTThread->thread_name, pTThread->wakeup_index, dave_false);
		}
	}
}

static void *
_tthread_function_thread(void *arg)
{
	ThreadThread *pTThread = (ThreadThread *)arg;

	_tthread_setup_thread_name(pTThread);

	THREADDEBUG("Start thread:%s", pTThread->thread_name);

	pTThread->state = ThreadState_RUNNING;

	while(pTThread->state == ThreadState_RUNNING)
	{
		_tthread_running_thread(pTThread);

		_tthread_sleep_thread(pTThread);
	}

	_tthread_reset(pTThread);

	return NULL;
}

static void
_tthread_die_thread(ThreadThread *pTThread)
{
	if(pTThread == NULL)
	{
		THREADABNOR("pTThread is NULL!");
		return;
	}

	pthread_mutex_lock(&(pTThread->m_mutex_t));
	pTThread->state = ThreadState_STOPPED;
	pthread_mutex_unlock(&(pTThread->m_mutex_t));

	pthread_cond_broadcast(&(pTThread->m_cond_t));
}

static dave_bool
_tthread_creat_thread(ThreadThread *pTThread)
{
	pthread_attr_t process_attr;

	if(pTThread == NULL)
	{
		THREADABNOR("pTThread is NULL!");
		return dave_false;
	}

	pthread_mutex_init(&(pTThread->m_mutex_t), NULL);
	pthread_cond_init(&(pTThread->m_cond_t), NULL); 

	pthread_attr_init(&process_attr);
	pthread_attr_setscope(&process_attr, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate(&process_attr, PTHREAD_CREATE_DETACHED);

	if(pthread_create(&(pTThread->thr_id), &process_attr, _tthread_function_thread, (void *)pTThread) != 0)
	{
		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

static pthread_t
_tthread_self_thread(void)
{
	return pthread_self();
}

static void
_tthread_init(schedule_thread_fun schedule_fun)
{
	_main_thread = _tthread_self_thread();

	_tthread_reset_all();

	_tthread_reset_self_map_all();

	_tthread_reset_index_map_all();

	_schedule_thread_fun = schedule_fun;

	t_lock_reset(&_thread_thread_pv);
}

static void
_tthread_exit(void)
{
	ub tthread_index;

	for(tthread_index=0; tthread_index<THREAD_THREAD_MAX; tthread_index++)
	{
		if(_thread_thread[tthread_index].thread_name[0] != '\0')
		{
			_tthread_die_thread(&_thread_thread[tthread_index]);
		}
	}
}

static ThreadThread *
_tthread_find_ptr(ub thread_index, ub wakeup_index, dave_bool find_new)
{
	ub tthread_index, safe_counter;
	ThreadThread *pTThread;

	tthread_index = (thread_index % THREAD_THREAD_MAX);

	pTThread = NULL;

	for(safe_counter=0; safe_counter<THREAD_THREAD_MAX; safe_counter++)
	{
		if(tthread_index >= THREAD_THREAD_MAX)
		{
			tthread_index = 0;
		}

		if((find_new == dave_true) && (_thread_thread[tthread_index].thread_name[0] == '\0'))
		{
			pTThread = &_thread_thread[tthread_index];
			break;
		}

		if((find_new == dave_false) && (thread_index == _thread_thread[tthread_index].thread_index))
		{
			if(wakeup_index >= THREAD_THREAD_MAX)
			{
				pTThread = &_thread_thread[tthread_index];
				break;
			}
			else if(wakeup_index == _thread_thread[tthread_index].wakeup_index)
			{
				pTThread = &_thread_thread[tthread_index];
				break;
			}
		}

		tthread_index ++;
	}

	return pTThread;
}

static ThreadSelfMap *
_tthread_find_self_map(pthread_t self, ThreadThread *pTThread, dave_bool find_new)
{
	ub map_index, safe_counter;
	ThreadSelfMap *pMap;

	if(_main_thread == self)
	{
		return NULL;
	}

	map_index = (((ub)self) % THREAD_THREAD_MAX);

	pMap = NULL;

	for(safe_counter=0; safe_counter<THREAD_THREAD_MAX; safe_counter++)
	{
		if(map_index >= THREAD_THREAD_MAX)
		{
			map_index = 0;
		}

		if(((find_new == dave_true) && (_self_map[map_index].pTThread == NULL))
			|| ((find_new == dave_false) && (_self_map[map_index].self == self) && (_self_map[map_index].pTThread != NULL)))
		{
			pMap = &_self_map[map_index];
			break;
		}

		map_index ++;
	}

	if((find_new == dave_true) && (pMap != NULL))
	{
		pMap->self = self;
		pMap->pTThread = pTThread;
	}

	return pMap;
}

static void
_tthread_find_index_map(ub thread_index, dave_bool add, ThreadThread *pTThread)
{
	ub tthread_index, move_tthread_index;

	if(thread_index >= THREAD_MAX)
	{
		THREADABNOR("invalid thread_index:%d", thread_index);
		return;
	}

	if(add == dave_true)
	{
		for(tthread_index=0; tthread_index<THREAD_THREAD_MAX; tthread_index++)
		{
			if(_index_map[thread_index].pTThread[tthread_index] == NULL)
			{
				_index_map[thread_index].pTThread[tthread_index] = pTThread;

				break;
			}
		}
	}
	else
	{
		for(tthread_index=0; tthread_index<THREAD_THREAD_MAX; tthread_index++)
		{
			if(_index_map[thread_index].pTThread[tthread_index] == pTThread)
			{
				_index_map[thread_index].pTThread[tthread_index] = NULL;
			}
		}

		for(tthread_index=0; tthread_index<THREAD_THREAD_MAX; tthread_index++)
		{
			if(_index_map[thread_index].pTThread[tthread_index] == NULL)
			{
				for(move_tthread_index=tthread_index+1; move_tthread_index<THREAD_THREAD_MAX; move_tthread_index++)
				{
					_index_map[thread_index].pTThread[move_tthread_index - 1] = _index_map[thread_index].pTThread[move_tthread_index];	
				}
			}
		}
	}
}

static ThreadThread *
_tthread_find_wakeup_thread(ub thread_index)
{
	ThreadIndexMap *pMap;
	ub safe_counter;
	ThreadThread *pTThread;

	if(thread_index >= THREAD_MAX)
	{
		THREADABNOR("invalid thread_index:%d", thread_index);
		return NULL;
	}

	pMap = &_index_map[thread_index];

	safe_counter = 0;

	pTThread = NULL;

	thread_lock();

	while(((++ safe_counter) <= THREAD_THREAD_MAX) && (pTThread == NULL))
	{
		if(pMap->current_wakeup_tthread_index >= THREAD_THREAD_MAX)
		{
			pMap->current_wakeup_tthread_index = 0;
		}

		pTThread = pMap->pTThread[pMap->current_wakeup_tthread_index ++];

		if(pTThread == NULL)
		{
			pMap->current_wakeup_tthread_index = 0;
		}
	}

	thread_unlock();

	return pTThread;
}

static ThreadThread *
_tthread_find_my_thread(ub thread_index, ub wakeup_index)
{
	ThreadIndexMap *pMap;
	ub tthread_index;

	if(thread_index >= THREAD_MAX)
	{
		THREADABNOR("invalid thread_index:%d", thread_index);
		return NULL;
	}

	pMap = &_index_map[thread_index];

	for(tthread_index=0; tthread_index<THREAD_THREAD_MAX; tthread_index++)
	{
		if(pMap->pTThread[tthread_index] == NULL)
		{
			break;
		}

		if(pMap->pTThread[tthread_index]->wakeup_index == wakeup_index)
		{
			return pMap->pTThread[tthread_index];
		}
	}

	return NULL;
}

static dave_bool
_tthread_creat(s8 *thread_name, ub thread_index, ThreadId thread_id, ub number_index)
{
	ThreadThread *pTThread;
	dave_bool ret;

	pTThread = _tthread_find_ptr(thread_index, THREAD_THREAD_MAX, dave_true);

	ret = dave_true;

	if(pTThread == NULL)
	{
		THREADABNOR("Not enough resources! %s,%d,%d,%d",
			thread_name, thread_index, thread_id, number_index);

		ret = dave_false;
	}

	if(pTThread->thread_name[0] != '\0')
	{
		THREADABNOR("Algorithm exception! %s,%d,%d,%d",
			thread_name, thread_index, thread_id, number_index);

		ret = dave_false;
	}

	if(ret == dave_true)
	{
		dave_snprintf(pTThread->thread_name, THREAD_NAME_MAX, "%s%s%d", dave_verno_my_product(), thread_name, number_index);
		pTThread->thread_index = thread_index;
		pTThread->thread_id = thread_id;

		pTThread->wakeup_index = number_index;
	}

	if(ret == dave_true)
	{
		ret = _tthread_creat_thread(pTThread);

		if(ret == dave_true)
		{
			_tthread_find_self_map(pTThread->thr_id, pTThread, dave_true);

			_tthread_find_index_map(thread_index, dave_true, pTThread);
		}
	}

	if((ret == dave_false) && (pTThread != NULL))
	{
		_tthread_reset(pTThread);
	}

	return ret;
}

static dave_bool
_tthread_die(ub thread_index)
{
	ThreadThread *pTThread;

	pTThread = _tthread_find_ptr(thread_index, THREAD_THREAD_MAX, dave_false);

	if(pTThread != NULL)
	{
		_tthread_reset_self_map(_tthread_find_self_map(pTThread->thr_id, pTThread, dave_false));

		_tthread_find_index_map(thread_index, dave_false, pTThread);

		_tthread_die_thread(pTThread);

		return dave_true;
	}
	else
	{
		return dave_false;
	}
}

static dave_bool
_thread_safe_creat(s8 *thread_name, ub thread_index, ThreadId thread_id, ub thread_number)
{
	ub number_index;
	dave_bool ret;

	ret = dave_false;

	for(number_index=0; number_index<thread_number; number_index++)
	{
		if(_tthread_creat(thread_name, thread_index, thread_id, number_index) == dave_true)
		{
			ret = dave_true;
		}
		else
		{
			break;
		}
	}

	return ret;
}

static void
_thread_safe_die(ub thread_index)
{
	ub number_index;

	for(number_index=0; number_index<THREAD_THREAD_MAX; number_index++)
	{
		if(_tthread_die(thread_index) == dave_false)
		{
			break;
		}
	}
}

// =====================================================================

void
thread_thread_init(schedule_thread_fun schedule_fun)
{
	_tthread_init(schedule_fun);
}

void
thread_thread_exit(void)
{
	_tthread_exit();
}

dave_bool
thread_thread_creat(s8 *thread_name, ub thread_index, ThreadId thread_id, ub thread_number)
{
	dave_bool ret = dave_false;

	SAFEZONEv3(_thread_thread_pv, {

		ret = _thread_safe_creat(thread_name, thread_index, thread_id, thread_number);

	} );

	return ret;
}

void
thread_thread_die(ub thread_index)
{
	SAFEZONEv3(_thread_thread_pv, {

		_thread_safe_die(thread_index);		

	} );
}

dave_bool
thread_thread_wakeup(ub thread_index)
{
	ThreadThread *pTThread;
	dave_bool ret;

	pTThread = _tthread_find_wakeup_thread(thread_index);

	if(pTThread != NULL)
	{
		ret = _tthread_wakeup_thread(pTThread);
	}
	else
	{
		ret = dave_false;
	}

	return ret;
}

dave_bool
thread_thread_is_main(void)
{
	if(_main_thread == _tthread_self_thread())
		return dave_true;
	else
		return dave_false;
}

ThreadId
thread_thread_self(ub *wakeup_index, ThreadSync **ppSync)
{
	ThreadSelfMap *pMap;
	ThreadId thread_id;

	pMap = _tthread_find_self_map(_tthread_self_thread(), NULL, dave_false);

	if(pMap == NULL)
	{
		thread_id = INVALID_THREAD_ID;
		if(wakeup_index != NULL)
		{
			*wakeup_index = 0;
		}
		if(ppSync != NULL)
		{
			*ppSync = NULL;
		}
	}
	else
	{
		thread_id = pMap->pTThread->thread_id;
		if(wakeup_index != NULL)
		{
			*wakeup_index = pMap->pTThread->wakeup_index;
		}
		if(ppSync != NULL)
		{
			*ppSync = &(pMap->pTThread->sync);
		}
	}

	return thread_id;
}

ThreadSync *
thread_thread_sync(ub thread_index, ub wakeup_index)
{
	ThreadThread *pTThread;

	pTThread = _tthread_find_my_thread(thread_index, wakeup_index);
	if(pTThread == NULL)
	{
		return NULL;
	}

	return &(pTThread->sync);
}

#endif

