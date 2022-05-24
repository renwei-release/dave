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
#include "dave_os.h"
#include "thread_parameter.h"
#include "thread_mem.h"
#include "thread_struct.h"
#include "thread_thread.h"
#include "thread_tools.h"
#include "thread_quit.h"
#include "thread_statistics.h"
#include "thread_call.h"
#include "thread_log.h"

#define THREAD_BUSY_MSG_NOTIFY_MIN 1024

#define CFG_THREAD_BUSY_MSG_MIN (s8 *)"ThreadBusyMsgMin"

typedef struct {
	ThreadId thread_id;
	ub msg_id;
	ub msg_number;
	void *next;
} ThreadBusyIdleList;

typedef struct {
	ThreadStruct *pThread;
	ThreadBusyIdleList *pList;
} ThreadBusyIdle;

static ThreadBusyIdle _thread_busy_idle[THREAD_MAX];
static ub _thread_busy_msg_notify_min = THREAD_BUSY_MSG_NOTIFY_MIN;

static void
_thread_busy_idle_free_list(ThreadBusyIdle *pThread)
{
	ThreadBusyIdleList *pList = pThread->pList;
	ThreadBusyIdleList *pTemp;

	while(pList != NULL)
	{
		pTemp = pList->next;

		THREADDEBUG("thread_id:%d msg_id:%d pList:%x", pList->thread_id, pList->msg_id, pList);

		dave_free(pList);

		pList = pTemp;
	}

	pThread->pList = NULL;
}

static ThreadBusyIdleList *
_thread_busy_idle_malloc_list(ThreadId thread_id, ub msg_id)
{
	ThreadBusyIdleList *pList;

	pList = dave_ralloc(sizeof(ThreadBusyIdleList));

	pList->thread_id = thread_id;
	pList->msg_id = msg_id;
	pList->msg_number = 1;
	pList->next = NULL;

	THREADDEBUG("thread_id:%d msg_id:%d pList:%x", pList->thread_id, pList->msg_id, pList);

	return pList;
}

static void
_thread_busy_idle_reset(ThreadStruct *thread_struct)
{
	ub thread_index;

	for(thread_index=0; thread_index<THREAD_MAX; thread_index++)
	{
		dave_memset(&_thread_busy_idle[thread_index], 0x00, sizeof(ThreadBusyIdle));

		_thread_busy_idle[thread_index].pThread = &thread_struct[thread_index];
		_thread_busy_idle[thread_index].pList = NULL;
	}
}

static void
_thread_busy_idle_clear(void)
{
	ub thread_index;

	for(thread_index=0; thread_index<THREAD_MAX; thread_index++)
	{
		_thread_busy_idle_free_list(&_thread_busy_idle[thread_index]);
	}
}

static dave_bool
_thread_busy_idle_on_list(ThreadBusyIdleList *pList, ThreadId thread_id, ub msg_id)
{
	while(pList != NULL)
	{
		if((pList->thread_id == thread_id) && (pList->msg_id == msg_id))
		{
			pList->msg_number ++;

			return dave_true;
		}

		pList = pList->next;
	}

	return dave_false;
}

static ThreadBusyIdleList *
_thread_busy_idle_add_list(ThreadBusyIdleList *pList, ThreadId thread_id, ub msg_id)
{
	ThreadBusyIdleList *pNewList = _thread_busy_idle_malloc_list(thread_id, msg_id);
	ThreadBusyIdleList *pNextList;

	if(pList == NULL)
	{
		pList = pNewList;
	}
	else
	{
		pNextList = pList;

		while(pNextList->next != NULL)
		{
			pNextList = pNextList->next;
		}

		pNextList->next = pNewList;
	}

	return pList;
}

static ThreadBusyIdleList *
_thread_busy_idle_read_on_msg(ThreadQueue *pQueue)
{
	ub list_index;
	ThreadMsg *pTemp;
	ThreadBusyIdleList *pList;

	if(pQueue->list_number < _thread_busy_msg_notify_min)
	{
		return NULL;
	}

	pTemp = pQueue->queue_head;
	pList = NULL;

	for(list_index=0; list_index<pQueue->list_number; list_index++)
	{
		if(pTemp == NULL)
			break;

		if(_thread_busy_idle_on_list(pList, pTemp->msg_body.msg_src, pTemp->msg_body.msg_id) == dave_false)
		{
			pList = _thread_busy_idle_add_list(pList, pTemp->msg_body.msg_src, pTemp->msg_body.msg_id);
		}

		pTemp = (ThreadMsg *)(pTemp->next);
	}

	return pList;
}

static ThreadBusyIdleList *
_thread_busy_idle_read_on_queue(ThreadQueue *pQueue_ptr, ub queue_number)
{
	ub queue_index;
	ThreadBusyIdleList *pMsgList = NULL, *pNewList, *pTempList;

	for(queue_index=0; queue_index<queue_number; queue_index++)
	{
		pNewList = _thread_busy_idle_read_on_msg(&(pQueue_ptr[queue_index]));

		if(pMsgList == NULL)
		{
			pMsgList = pNewList;
		}
		else
		{
			pTempList = pMsgList;
			while(pTempList->next != NULL) pTempList = pTempList->next;
			pTempList->next = pNewList;
		}
	}

	return pMsgList;
}

static void
_thread_busy_idle_build_list(ThreadBusyIdle *pThread)
{
	ThreadBusyIdleList *pMsgList = NULL, *pSeqList = NULL, *pNextList;

	pMsgList = _thread_busy_idle_read_on_queue(pThread->pThread->msg_queue, THREAD_MSG_QUEUE_NUM);
	pSeqList = _thread_busy_idle_read_on_queue(pThread->pThread->seq_queue, THREAD_SEQ_QUEUE_NUM);

	pThread->pList = pMsgList;

	if(pThread->pList == NULL)
	{
		pThread->pList = pSeqList;
	}
	else
	{
		pNextList = pThread->pList;
		while(pNextList->next != NULL) pNextList = pNextList->next;
		pNextList->next = pSeqList;
	}
}

static void
_thread_busy_idle_notify_busy(ThreadBusyIdle *pThread)
{
	dave_bool busy_flag = dave_false;
	ThreadBusyIdleList *pList;
	ThreadBusy *pBusy;
	ub msg_queue_total, seq_queue_total;

	msg_queue_total = thread_queue_list(pThread->pThread->msg_queue, THREAD_MSG_QUEUE_NUM);
	seq_queue_total = thread_queue_list(pThread->pThread->seq_queue, THREAD_SEQ_QUEUE_NUM);

	if((msg_queue_total > _thread_busy_msg_notify_min)
		|| (seq_queue_total > _thread_busy_msg_notify_min))
	{
		busy_flag = dave_true;
	}

	if(busy_flag == dave_true)
	{
		THREADDEBUG("thread:%s busy start .......", pThread->pThread->thread_name);

		_thread_busy_idle_build_list(pThread);

		pList = pThread->pList;

		while(pList != NULL)
		{
			pBusy = thread_msg(pBusy);
	
			pBusy->thread_id = pThread->pThread->thread_id;
			dave_strcpy(pBusy->thread_name, pThread->pThread->thread_name, DAVE_THREAD_NAME_LEN);
			pBusy->msg_id = pList->msg_id;
			pBusy->msg_number = pList->msg_number;

			THREADLOG("thread:%s msg:%d number:%d busy to %s",
				pBusy->thread_name, pBusy->msg_id, pBusy->msg_number,
				thread_name(pList->thread_id));

			id_msg(pList->thread_id, MSGID_THREAD_BUSY, pBusy);
	
			pList = pList->next;
		}
	}
}

static void
_thread_busy_idle_notify_idle(ThreadBusyIdle *pThread)
{
	dave_bool idle_flag = dave_false;
	ThreadBusyIdleList *pList;
	ThreadIdle *pIdle;
	ub msg_queue_total, seq_queue_total;

	msg_queue_total = thread_queue_list(pThread->pThread->msg_queue, THREAD_MSG_QUEUE_NUM);
	seq_queue_total = thread_queue_list(pThread->pThread->seq_queue, THREAD_SEQ_QUEUE_NUM);

	if((msg_queue_total == 0)
		&& (seq_queue_total == 0))
	{
		idle_flag = dave_true;
	}

	if(idle_flag == dave_true)
	{
		pList = pThread->pList;

		THREADDEBUG("thread:%s idle start ...... msg:%d list:%d pList:%x",
			pThread->pThread->thread_name,
			msg_queue_total,
			seq_queue_total,
			pList);

		while(pList != NULL)
		{
			pIdle = thread_msg(pIdle);
	
			pIdle->thread_id = pThread->pThread->thread_id;
			dave_strcpy(pIdle->thread_name, pThread->pThread->thread_name, DAVE_THREAD_NAME_LEN);

			THREADLOG("thread:%s idle to %s", pIdle->thread_name, thread_name(pList->thread_id));

			id_msg(pList->thread_id, MSGID_THREAD_IDLE, pIdle);
	
			pList = pList->next;
		}
	
		_thread_busy_idle_free_list(pThread);
	}
}

static void
_thread_busy_idle_notify(ThreadBusyIdle *pThread)
{
	if(pThread->pList == NULL)
	{
		_thread_busy_idle_notify_busy(pThread);
	}
	else
	{
		_thread_busy_idle_notify_idle(pThread);
	}
}

static void
_thread_busy_idle_check(void)
{
	ub thread_index;

	for(thread_index=0; thread_index<THREAD_MAX; thread_index++)
	{
		if((_thread_busy_idle[thread_index].pThread != NULL)
			&& (_thread_busy_idle[thread_index].pThread->thread_id != INVALID_THREAD_ID))
		{
			_thread_busy_idle_notify(&_thread_busy_idle[thread_index]);
		}
	}
}

static void
_thread_busy_idle_cfg_reset(void)
{
	_thread_busy_msg_notify_min = base_cfg_get_ub(CFG_THREAD_BUSY_MSG_MIN);

	if(_thread_busy_msg_notify_min == 0)
	{
		_thread_busy_msg_notify_min = THREAD_BUSY_MSG_NOTIFY_MIN;
		base_cfg_set_ub(CFG_THREAD_BUSY_MSG_MIN, _thread_busy_msg_notify_min);
	}

	THREADDEBUG("_thread_busy_msg_notify_min:%d", _thread_busy_msg_notify_min);
}

// =====================================================================

void
thread_busy_idle_init(ThreadStruct *thread_struct)
{
	_thread_busy_idle_cfg_reset();

	_thread_busy_idle_reset(thread_struct);
}

void
thread_busy_idle_exit(void)
{
	_thread_busy_idle_clear();
}

void
thread_busy_idle_cfg_update(CFGUpdate *pUpdate)
{
	if(dave_strcmp(pUpdate->cfg_name, CFG_THREAD_BUSY_MSG_MIN) == dave_true)
	{
		_thread_busy_idle_cfg_reset();
	}
}

void
thread_busy_idle_check(void)
{
	_thread_busy_idle_check();
}

#endif

