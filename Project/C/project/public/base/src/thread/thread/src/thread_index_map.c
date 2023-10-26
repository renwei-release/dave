/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "dave_tools.h"
#include "thread_struct.h"
#include "thread_self_map.h"
#include "thread_log.h"

static void *_thread_index_thread_kv = NULL;
static void *_thread_index_list_kv = NULL;
static TLock _thread_index_list_pv;

static s8 *
_thread_index_thread_key(s8 *key_ptr, ub key_len, ub thread_index, ub wakeup_index)
{
	dave_snprintf(key_ptr, key_len, "%ld-%ld", thread_index, wakeup_index);
	return key_ptr;
}

static __ThreadThreadList__ *
_thread_index_list_malloc__(ThreadThread *pTThread)
{
	__ThreadThreadList__ *pList = dave_ralloc(sizeof(__ThreadThreadList__));

	pList->pTThread = pTThread;
	pList->next = NULL;

	return pList;
}

static void
_thread_index_list_free__(__ThreadThreadList__ *pList)
{
	dave_free(pList);
}

static ThreadThreadList *
_thread_index_list_malloc(ThreadThread *pTThread)
{
	ThreadThreadList *pList = dave_ralloc(sizeof(ThreadThreadList));

	pList->thread_index = pTThread->thread_index;
	dave_strcpy(pList->thread_name, pTThread->thread_name, sizeof(pList->thread_name));

	pList->pList = _thread_index_list_malloc__(pTThread);
	pList->pLoop = NULL;

	return pList;
}

static void
_thread_index_list_free(ThreadThreadList *pList)
{
	__ThreadThreadList__ *pListData = pList->pList;
	__ThreadThreadList__ *pListBackupNext;

	while(pListData != NULL)
	{
		pListBackupNext = pListData->next;
		_thread_index_list_free__(pListData);
		pListData = pListBackupNext;
	}

	dave_free(pList);
}

static ThreadThreadList *
_thread_index_list_inq_(ub thread_index)
{
	return kv_inq_ub_ptr(_thread_index_list_kv, thread_index);
}

static void
_thread_index_list_add_(ub thread_index, ThreadThreadList *pList)
{
	kv_add_ub_ptr(_thread_index_list_kv, thread_index, pList);
}

static ThreadThreadList *
_thread_index_list_del_(ub thread_index)
{
	return kv_del_ub_ptr(_thread_index_list_kv, thread_index);
}

static void
_thread_index_list_add(ThreadThread *pTThread)
{
	ThreadThreadList *pList;
	__ThreadThreadList__ *pAdd;
	ub safe_counter;

	pList = _thread_index_list_inq_(pTThread->thread_index);
	if(pList == NULL)
	{
		pList = _thread_index_list_malloc(pTThread);
		_thread_index_list_add_(pTThread->thread_index, pList);
	}
	else
	{
		pAdd = pList->pList;
		safe_counter = 0;
		while(((++ safe_counter) < THREAD_THREAD_MAX) && (pAdd->next != NULL)) { pAdd = pAdd->next; }
		if(safe_counter < THREAD_THREAD_MAX)
		{
			pAdd->next = _thread_index_list_malloc__(pTThread);

			pList->pLoop = pList->pList;
		}
		else
		{
			THREADLOG("find invalid safe_counter:%d on thread:%s", safe_counter, pTThread->thread_name);
		}	
	}
}

static void
_thread_index_list_del(ThreadThread *pTThread)
{
	ThreadThreadList *pOldList;
	__ThreadThreadList__ *pList;

	pOldList = _thread_index_list_del_(pTThread->thread_index);
	if(pOldList != NULL)
	{
		pList = pOldList->pList;

		while(pList != NULL)
		{
			if(pList->pTThread != pTThread)
			{
				_thread_index_list_add(pList->pTThread);
			}
			pList = pList->next;
		}

		_thread_index_list_free(pOldList);
	}
}

static ThreadThreadList *
_thread_index_list_inq(ub thread_index)
{
	return _thread_index_list_inq_(thread_index);
}

static ThreadThread *
_thread_index_list_loop(ub thread_index)
{
	ThreadThreadList *pList;
	__ThreadThreadList__ *pLoop;

	pList = _thread_index_list_inq_(thread_index);
	if(pList == NULL)
		return NULL;

	if(pList->pLoop == NULL)
	{
		pList->pLoop = pList->pList;
	}

	pLoop = pList->pLoop;
	if(pLoop == NULL)
	{
		THREADABNOR("Arithmetic error!");
		return pList->pList->pTThread;
	}
	if(pLoop->next == NULL)
	{
		pList->pLoop = pList->pList;
	}
	else
	{
		pList->pLoop = pLoop->next;
	}

	return pLoop->pTThread;
}

static RetCode
_thread_index_list_recycle(void *ramkv, s8 *key)
{
	ThreadThreadList *pList = kv_del_key_ptr(ramkv, key);

	if(pList == NULL)
		return RetCode_empty_data;

	_thread_index_list_free(pList);

	return RetCode_OK;
}

// =====================================================================

void
thread_index_map_init(void)
{
	_thread_index_thread_kv = kv_malloc("titkv", 0, NULL);
	_thread_index_list_kv = kv_malloc("tilkv", 0, NULL);
	t_lock_reset(&_thread_index_list_pv);
}

void
thread_index_map_exit(void)
{
	kv_free(_thread_index_thread_kv, NULL);
	_thread_index_thread_kv = NULL;
	kv_free(_thread_index_list_kv, _thread_index_list_recycle);
	_thread_index_list_kv = NULL;
}

void
thread_index_thread_add(ThreadThread *pTThread)
{
	s8 key[128];

	_thread_index_thread_key(key, sizeof(key), pTThread->thread_index, pTThread->wakeup_index);

	kv_add_key_ptr(_thread_index_thread_kv, key, pTThread);
}

ThreadThread *
thread_index_thread_inq(ub thread_index, ub wakeup_index)
{
	s8 key[128];

	_thread_index_thread_key(key, sizeof(key), thread_index, wakeup_index);

	return (ThreadThread *)kv_inq_key_ptr(_thread_index_thread_kv, key);
}

ThreadThread *
thread_index_thread_del(ThreadThread *pTThread)
{
	s8 key[128];

	_thread_index_thread_key(key, sizeof(key), pTThread->thread_index, pTThread->wakeup_index);

	return kv_del_key_ptr(_thread_index_thread_kv, key);
}

void
thread_index_list_add(ThreadThread *pTThread)
{
	SAFECODEv2W(_thread_index_list_pv, { _thread_index_list_add(pTThread); });
}

void
thread_index_list_del(ThreadThread *pTThread)
{
	SAFECODEv2W(_thread_index_list_pv, { _thread_index_list_del(pTThread); });
}

__ThreadThreadList__ *
thread_index_list_inq(ub thread_index)
{
	ThreadThreadList *pList = NULL;

	SAFECODEv2R(_thread_index_list_pv, { pList = _thread_index_list_inq(thread_index); });

	if(pList == NULL)
		return NULL;

	return pList->pList;
}

ThreadThread *
thread_index_list_loop(ub thread_index)
{
	ThreadThread *pTThread = NULL;

	SAFECODEv2R(_thread_index_list_pv, { pTThread = _thread_index_list_loop(thread_index); });

	return pTThread;
}

#endif

