/*
 * Copyright (c) 2022 - 2023 Renwei
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
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_verno.h"
#include "thread_struct.h"
#include "thread_thread.h"
#include "thread_parameter.h"
#include "thread_quit.h"
#include "thread_tools.h"
#include "thread_chain.h"
#include "thread_router.h"
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
	ThreadQueue thread_queue[THREAD_THREAD_QUEUE_NUM];
	ThreadChain chain;
	ThreadRouter router;

	volatile ThreadState state;

	pthread_t thr_id;
	pthread_mutex_t m_mutex_t;
	pthread_cond_t  m_cond_t;

	ub wakeup_index;

	ThreadSync sync;
	void *current_coroutine_point;
} ThreadThread;

typedef struct {
	pthread_t self;
	ThreadThread *pTThread;
} ThreadSelfMap;

typedef struct {
	ThreadThread *pTThread;
	void *next;
} ThreadIndexList;

typedef struct {
	TLock pv;

	ThreadIndexList *pList;
	ThreadIndexList *pCurr;

	ThreadThread *pTThread[THREAD_THREAD_MAX];
} ThreadIndexMap;

int pthread_setname_np(pthread_t thread, const char *name);

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
	thread_queue_reset(pTThread->thread_queue, THREAD_THREAD_QUEUE_NUM);
	thread_chain_reset(&(pTThread->chain));
	thread_router_reset(&(pTThread->router));

	pTThread->state = ThreadState_INIT;

	pTThread->wakeup_index = 0;

	thread_reset_sync(&(pTThread->sync));
	pTThread->current_coroutine_point = NULL;
}

static void
_tthread_reset_all(void)
{
	ub tthread_index;

	for(tthread_index=0; tthread_index<THREAD_THREAD_MAX; tthread_index++)
	{		
		dave_memset(&_thread_thread[tthread_index], 0x00, sizeof(ThreadThread));

		thread_queue_booting(_thread_thread[tthread_index].thread_queue, THREAD_THREAD_QUEUE_NUM);

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
	ub tthread_index;

	if(pMap != NULL)
	{
		pMap->pList = pMap->pCurr = NULL;

		for(tthread_index=0; tthread_index<THREAD_THREAD_MAX; tthread_index++)
		{
			pMap->pTThread[tthread_index] = NULL;
		}
	}
}

static void
_tthread_reset_index_map_all(void)
{
	ub map_index;

	dave_memset(_index_map, 0x00, sizeof(_index_map));

	for(map_index=0; map_index<THREAD_MAX; map_index++)
	{
		dave_memset(&_index_map[map_index], 0x00, sizeof(ThreadIndexMap));

		t_lock_reset(&(_index_map[map_index].pv));

		_tthread_reset_index_map(&_index_map[map_index]);
	}
}

static void
_tthread_setup_thread_name(ThreadThread *pTThread)
{
	pthread_setname_np(pTThread->thr_id, (const char *)(pTThread->thread_name));
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

static inline dave_bool
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

static inline ub
_tthread_running_fun(void *pTThread, ThreadId thread_id, s8 *thread_name, ub wakeup_index, dave_bool enable_stack)
{
	if(_schedule_thread_fun != NULL)
	{
		return _schedule_thread_fun(pTThread, thread_id, thread_name, wakeup_index, dave_false);
	}

	return 0;
}

static inline void
_tthread_running_thread(ThreadThread *pTThread)
{
	ub msg_number;

	while(pTThread->state == ThreadState_RUNNING)
	{
		msg_number = _tthread_running_fun(pTThread, pTThread->thread_id, pTThread->thread_name, pTThread->wakeup_index, dave_false);
		if(msg_number == 0)
		{
			break;
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

static inline pthread_t
_tthread_self_thread(void)
{
	return pthread_self();
}

static inline ThreadThread *
_tthread_find_thread(ub thread_index, dave_bool find_new)
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
			pTThread = &_thread_thread[tthread_index];
		}

		tthread_index ++;
	}

	return pTThread;
}

static inline ThreadSelfMap *
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

static inline ThreadSelfMap *
_tthread_find_self_old_map(pthread_t self)
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

		if((_self_map[map_index].self == self) && (_self_map[map_index].pTThread != NULL))
		{
			pMap = &_self_map[map_index];
			break;
		}

		map_index ++;
	}

	return pMap;
}

static inline ThreadThread *
_tthread_find_wakeup_thread(ub thread_index)
{
	ThreadIndexMap *pIndexMap;
	ub safe_counter;
	ThreadThread *pTThread;

	if(thread_index >= THREAD_MAX)
	{
		THREADABNOR("invalid thread_index:%d", thread_index);
		return NULL;
	}

	pIndexMap = &_index_map[thread_index];

	safe_counter = 0;

	pTThread = NULL;

	t_lock_spin(&(_index_map[thread_index].pv));

	while((pTThread == NULL) && ((++ safe_counter) <= THREAD_THREAD_MAX))
	{
		if(pIndexMap->pCurr == NULL)
		{
			pIndexMap->pCurr = pIndexMap->pList;
		}

		if(pIndexMap->pCurr == NULL)
		{
			break;
		}

		pTThread = pIndexMap->pCurr->pTThread;
		pIndexMap->pCurr = pIndexMap->pCurr->next;
	}

	t_unlock_spin(&(_index_map[thread_index].pv));

	return pTThread;
}

static inline void
_tthread_add_index_map(ub thread_index, ThreadThread *pTThread)
{
	ub tthread_index;
	dave_bool find_node = dave_false;

	if(thread_index >= THREAD_MAX)
	{
		THREADABNOR("invalid thread_index:%d", thread_index);
		return;
	}

	t_lock_spin(&(_index_map[thread_index].pv));

	for(tthread_index=0; tthread_index<THREAD_THREAD_MAX; tthread_index++)
	{
		if((_index_map[thread_index].pTThread[tthread_index] == NULL)
			&& (thread_index == tthread_index))
		{
			_index_map[thread_index].pTThread[tthread_index] = pTThread;
			find_node = dave_true;
			break;
		}
	}
	
	if(find_node == dave_false)
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

	t_unlock_spin(&(_index_map[thread_index].pv));
}

static inline void
_tthread_del_index_map(ub thread_index, ThreadThread *pTThread)
{
	ub tthread_index, move_tthread_index;

	if(thread_index >= THREAD_MAX)
	{
		THREADABNOR("invalid thread_index:%d", thread_index);
		return;
	}

	t_lock_spin(&(_index_map[thread_index].pv));

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

	t_unlock_spin(&(_index_map[thread_index].pv));
}

static inline void
_tthread_add_list_map(ub thread_index, ThreadThread *pTThread)
{
	ub safe_counter;
	ThreadIndexList *pList;

	if(thread_index >= THREAD_MAX)
	{
		THREADABNOR("invalid thread_index:%d", thread_index);
		return;
	}

	t_lock_spin(&(_index_map[thread_index].pv));

	pList = _index_map[thread_index].pList;
	for(safe_counter=0; safe_counter<THREAD_THREAD_MAX; safe_counter++)
	{
		if(pList == NULL)
			break;

		if(pList->pTThread == pTThread)
		{
			THREADABNOR("%s duplicate addition!", pTThread->thread_name, thread_index);
			return;
		}

		pList = pList->next;
	}

	pList = dave_malloc(sizeof(ThreadIndexList));
	pList->pTThread = pTThread;
	pList->next = NULL;

	if(_index_map[thread_index].pList == NULL)
	{
		_index_map[thread_index].pList = _index_map[thread_index].pCurr = pList;
	}
	else
	{
		if(_index_map[thread_index].pCurr == NULL)
		{
			THREADABNOR("Arithmetic error:%lx/%lx", _index_map[thread_index].pList, pList);
			_index_map[thread_index].pList = _index_map[thread_index].pCurr = pList;
		}
		else
		{
			_index_map[thread_index].pCurr->next = pList;
			_index_map[thread_index].pCurr = pList;
		}
	}

	t_unlock_spin(&(_index_map[thread_index].pv));
}

static inline void
_tthread_del_list_map(ub thread_index, ThreadThread *pTThread)
{
	ub safe_counter;
	ThreadIndexList *pUp, *pCurr;

	if(thread_index >= THREAD_MAX)
	{
		THREADABNOR("invalid thread_index:%d", thread_index);
		return;
	}

	t_lock_spin(&(_index_map[thread_index].pv));

	pUp = pCurr = _index_map[thread_index].pList;
	for(safe_counter=0; safe_counter<THREAD_THREAD_MAX; safe_counter++)
	{
		if(pCurr == NULL)
			break;

		if(pCurr->pTThread == pTThread)
		{
			if(_index_map[thread_index].pList == pCurr)
			{
				_index_map[thread_index].pList = _index_map[thread_index].pList->next;
			}
			else
			{
				if(pUp->next != pCurr)
				{
					THREADABNOR("Algorithm error!");
				}
				pUp->next = pCurr->next;
			}

			dave_memset(pCurr, 0x00, sizeof(ThreadIndexList));

			dave_free(pCurr);
			break;
		}

		pUp = pCurr;
		pCurr = pCurr->next;
	}

	_index_map[thread_index].pCurr = _index_map[thread_index].pList;

	t_unlock_spin(&(_index_map[thread_index].pv));
}

static inline void
_tthread_clean_list_map(ub thread_index)
{
	ub safe_counter;
	ThreadIndexList *pCurr, *pList;

	if(thread_index >= THREAD_MAX)
	{
		THREADABNOR("invalid thread_index:%d", thread_index);
		return;
	}

	t_lock_spin(&(_index_map[thread_index].pv));

	pCurr = _index_map[thread_index].pList;
	for(safe_counter=0; safe_counter<THREAD_THREAD_MAX; safe_counter++)
	{
		if(pCurr == NULL)
			break;

		pList = pCurr;
		pCurr = pCurr->next;

		dave_memset(pList, 0x00, sizeof(ThreadIndexList));

		dave_free(pList);
	}

	_tthread_reset_index_map(&_index_map[thread_index]);

	t_unlock_spin(&(_index_map[thread_index].pv));
}

static inline ThreadThread *
_tthread_find_my_thread(ub thread_index, ub wakeup_index)
{
	ThreadIndexList *pList;
	ThreadThread *pTThread = NULL;
	ub safe_counter;

	if(thread_index >= THREAD_MAX)
	{
		THREADABNOR("invalid thread_index:%d", thread_index);
		return NULL;
	}

	t_lock_spin(&(_index_map[thread_index].pv));

	pList = _index_map[thread_index].pList;

	for(safe_counter=0; safe_counter<THREAD_THREAD_MAX; safe_counter++)
	{
		if(pList == NULL)
		{
			break;
		}

		if((pList->pTThread != NULL)
			&& (pList->pTThread->wakeup_index == wakeup_index))
		{
			pTThread = pList->pTThread;
			break;
		}

		pList = pList->next;
	}

	t_unlock_spin(&(_index_map[thread_index].pv));

	return pTThread;
}

static inline void
_tthread_opt_index_map(dave_bool add, ub thread_index, ThreadThread *pTThread)
{
	if(thread_index >= THREAD_MAX)
	{
		THREADABNOR("invalid thread_index:%d", thread_index);
		return;
	}

	if(add == dave_true)
	{
		_tthread_add_index_map(thread_index, pTThread);
		_tthread_add_list_map(thread_index, pTThread);
	}
	else
	{
		_tthread_del_index_map(thread_index, pTThread);
		_tthread_del_list_map(thread_index, pTThread);
	}
}

static dave_bool
_tthread_creat(s8 *thread_name, ub thread_index, ThreadId thread_id, ub number_index)
{
	ThreadThread *pTThread;
	dave_bool ret;

	pTThread = _tthread_find_thread(thread_index, dave_true);

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
		thread_queue_malloc(pTThread->thread_queue, THREAD_THREAD_QUEUE_NUM);

		pTThread->wakeup_index = number_index;
	}

	if(ret == dave_true)
	{
		ret = _tthread_creat_thread(pTThread);

		if(ret == dave_true)
		{
			_tthread_find_self_map(pTThread->thr_id, pTThread, dave_true);

			_tthread_opt_index_map(dave_true, thread_index, pTThread);
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

	pTThread = _tthread_find_thread(thread_index, dave_false);

	if(pTThread != NULL)
	{
		_tthread_reset_self_map(_tthread_find_self_map(pTThread->thr_id, pTThread, dave_false));

		_tthread_opt_index_map(dave_false, thread_index, pTThread);

		thread_queue_free(pTThread->thread_queue, THREAD_THREAD_QUEUE_NUM);

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
	ub thread_index, tthread_index;

	for(thread_index=0; thread_index<THREAD_MAX; thread_index++)
	{
		_tthread_clean_list_map(thread_index);
	}

	for(tthread_index=0; tthread_index<THREAD_THREAD_MAX; tthread_index++)
	{
		if(_thread_thread[tthread_index].thread_name[0] != '\0')
		{
			_tthread_die_thread(&_thread_thread[tthread_index]);
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

	SAFECODEv1(_thread_thread_pv, {

		ret = _thread_safe_creat(thread_name, thread_index, thread_id, thread_number);

	} );

	return ret;
}

void
thread_thread_die(ub thread_index)
{
	SAFECODEv1(_thread_thread_pv, {

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
thread_thread_self(ub *wakeup_index)
{
	ThreadSelfMap *pMap;
	ThreadId thread_id;

	pMap = _tthread_find_self_old_map(_tthread_self_thread());

	if(pMap == NULL)
	{
		thread_id = INVALID_THREAD_ID;
		if(wakeup_index != NULL)
		{
			*wakeup_index = 0;
		}
	}
	else
	{
		thread_id = pMap->pTThread->thread_id;
		if(wakeup_index != NULL)
		{
			*wakeup_index = pMap->pTThread->wakeup_index;
		}
	}

	return thread_id;
}

ThreadChain *
thread_thread_chain(void)
{
	ThreadSelfMap *pMap;

	pMap = _tthread_find_self_old_map(_tthread_self_thread());
	if(pMap == NULL)
	{
		return NULL;
	}

	return &(pMap->pTThread->chain);
}

ThreadRouter *
thread_thread_router(void)
{
	ThreadSelfMap *pMap;

	pMap = _tthread_find_self_old_map(_tthread_self_thread());
	if(pMap == NULL)
	{
		return NULL;
	}

	return &(pMap->pTThread->router);
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

void
__thread_thread_write__(
	void *msg_chain, void *msg_router,
	ub thread_index, ub wakeup_index,
	ub msg_id, ub msg_len, u8 *msg_body,
	s8 *fun, ub line)
{
	ThreadThread *pTThread;
	ThreadStruct *pThread;
	ThreadMsg *pMsg;

	pTThread = _tthread_find_my_thread(thread_index, wakeup_index);
	if(pTThread == NULL)
	{
		THREADABNOR("empty thread_index:%d wakeup_index:%d <%s:%d>",
			thread_index, wakeup_index, fun, line);
		return;
	}

	pThread = thread_find_busy_thread(pTThread->thread_id);

	pMsg = thread_build_msg(
		pThread->thread_id, pThread->attrib,
		msg_chain, msg_router,
		NULL, NULL,
		pThread->thread_id, pThread->thread_id,
		msg_id, msg_len, msg_body,
		BaseMsgType_Unicast,
		fun, line);

	THREADDEBUG("%lx/%d/%d %s->%s:%s",
		pTThread, thread_index, wakeup_index,
		thread_name(pMsg->msg_body.msg_src), thread_name(pMsg->msg_body.msg_dst),
		msgstr(pMsg->msg_body.msg_id));

	thread_queue_write(pTThread->thread_queue, pMsg);

	_tthread_wakeup_thread(pTThread);
}

ThreadMsg *
thread_thread_read(void *param)
{
	ThreadThread *pTThread = (ThreadThread *)param;
	ThreadMsg *pMsg;

	pMsg = thread_queue_read(pTThread->thread_queue);
	if(pMsg != NULL)
	{
		THREADDEBUG("%lx %s->%s:%s",
			pTThread,
			thread_name(pMsg->msg_body.msg_src), thread_name(pMsg->msg_body.msg_dst),
			msgstr(pMsg->msg_body.msg_id));		
	}

	return pMsg;
}

void
thread_thread_set_coroutine_site(ub thread_index, ub wakeup_index, void *point)
{
	ThreadThread *pTThread;

	pTThread = _tthread_find_my_thread(thread_index, wakeup_index);
	if(pTThread == NULL)
	{
		return;
	}

	if(pTThread->current_coroutine_point != NULL)
	{
		THREADABNOR("There is on-site information that has not been cleared!");
	}

	pTThread->current_coroutine_point = point;
}

void *
thread_thread_get_coroutine_site(ub thread_index, ub wakeup_index)
{
	ThreadThread *pTThread;
	void *point;

	pTThread = _tthread_find_my_thread(thread_index, wakeup_index);
	if(pTThread == NULL)
	{
		THREADLOG("%d/%d has empty thread!", thread_index, wakeup_index);
		return NULL;
	}

	if(pTThread->current_coroutine_point == NULL)
	{
		THREADABNOR("%s Algorithm exception!", pTThread->thread_name);
	}

	point = pTThread->current_coroutine_point;

	return point;
}

void
thread_thread_clean_coroutine_site(ub thread_index, ub wakeup_index)
{
	ThreadThread *pTThread;

	pTThread = _tthread_find_my_thread(thread_index, wakeup_index);
	if(pTThread == NULL)
	{
		return;
	}

	pTThread->current_coroutine_point = NULL;
}

#endif

