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
#include "thread_thread_param.h"
#include "thread_self_map.h"
#include "thread_index_map.h"
#include "thread_log.h"

int pthread_setname_np(pthread_t thread, const char *name);

static pthread_t _main_thread = (pthread_t)NULL;

static ThreadThread _thread_thread[THREAD_THREAD_MAX];
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
_tthread_running_fun(ThreadThread *pTThread, ThreadId thread_id, s8 *thread_name, ub wakeup_index, dave_bool enable_stack)
{
	ub msg_number;

	if(_schedule_thread_fun != NULL)
	{
		msg_number = _schedule_thread_fun(pTThread, thread_id, thread_name, wakeup_index, dave_false);

		msg_number += thread_queue_total_number(pTThread->thread_queue, THREAD_THREAD_QUEUE_NUM);
	}
	else
	{
		msg_number = 0;
	}

	return msg_number;
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

		ret = _tthread_creat_thread(pTThread);

		if(ret == dave_true)
		{
			thread_self_map_add(pTThread->thr_id, pTThread);
			thread_index_thread_add(pTThread);
			thread_index_list_add(pTThread);
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
		thread_self_map_del(pTThread->thr_id);
		thread_index_thread_del(pTThread);
		thread_index_list_del(pTThread);

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

// =====================================================================

void
thread_thread_init(schedule_thread_fun schedule_fun)
{
	_tthread_init(schedule_fun);
	thread_self_map_init();
	thread_index_map_init();
}

void
thread_thread_exit(void)
{
	thread_index_map_exit();
	thread_self_map_exit();
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

	pTThread = thread_index_list_loop(thread_index);

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
	ThreadThread *pTThread;
	ThreadId thread_id;

	pTThread = thread_self_map_inq(_tthread_self_thread());
	if(pTThread == NULL)
	{
		thread_id = INVALID_THREAD_ID;
		if(wakeup_index != NULL)
		{
			*wakeup_index = 0;
		}
	}
	else
	{
		thread_id = pTThread->thread_id;
		if(wakeup_index != NULL)
		{
			*wakeup_index = pTThread->wakeup_index;
		}
	}

	return thread_id;
}

ThreadChain *
thread_thread_chain(void)
{
	ThreadThread *pTThread;

	pTThread = thread_self_map_inq(_tthread_self_thread());
	if(pTThread == NULL)
	{
		return NULL;
	}

	return &(pTThread->chain);
}

ThreadRouter *
thread_thread_router(void)
{
	ThreadThread *pTThread;

	pTThread = thread_self_map_inq(_tthread_self_thread());
	if(pTThread == NULL)
	{
		return NULL;
	}

	return &(pTThread->router);
}

ThreadSync *
thread_thread_sync(ub thread_index, ub wakeup_index)
{
	ThreadThread *pTThread;

	pTThread = thread_index_thread_inq(thread_index, wakeup_index);
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

	pTThread = thread_index_thread_inq(thread_index, wakeup_index);
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

	pTThread = thread_index_thread_inq(thread_index, wakeup_index);
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

	pTThread = thread_index_thread_inq(thread_index, wakeup_index);
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

	pTThread = thread_index_thread_inq(thread_index, wakeup_index);
	if(pTThread == NULL)
	{
		return;
	}

	pTThread->current_coroutine_point = NULL;
}

ub
thread_thread_total_number(ub thread_index)
{
	__ThreadThreadList__ *pList = thread_index_list_inq(thread_index);
	ub total_number;
	ub safe_counter;

	if(pList == NULL)
		return 0;

	total_number = safe_counter = 0;

	while(((++ safe_counter) <= THREAD_THREAD_MAX) && (pList != NULL))
	{
		if(pList->pTThread == NULL)
			break;
	
		total_number += thread_queue_total_number(pList->pTThread->thread_queue, THREAD_THREAD_QUEUE_NUM);

		pList = pList->next;
	}

	if(safe_counter > THREAD_THREAD_MAX)
	{
		THREADABNOR("Arithmetic error!");
	}

	return total_number;
}

void
thread_thread_total_detail(ub *unprocessed, ub *received, ub *processed, ub thread_index)
{
	__ThreadThreadList__ *pList = thread_index_list_inq(thread_index);
	ub msg_unprocessed_counter, msg_received_counter, msg_processed_counter;
	ub safe_counter;

	*unprocessed = *received = *processed = 0;

	if(pList == NULL)
	{
		return;
	}

	safe_counter = 0;

	while(((++ safe_counter) <= THREAD_THREAD_MAX) && (pList != NULL))
	{
		if(pList->pTThread == NULL)
			break;
	
		thread_queue_total_detail(&msg_unprocessed_counter, &msg_received_counter, &msg_processed_counter, pList->pTThread->thread_queue, THREAD_THREAD_QUEUE_NUM);

		*unprocessed += msg_unprocessed_counter;
		*received += msg_received_counter;
		*processed += msg_processed_counter;

		pList = pList->next;
	}

	if(safe_counter > THREAD_THREAD_MAX)
	{
		THREADABNOR("Arithmetic error!");
	}
}

#endif

