/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "thread_struct.h"
#include "thread_mem.h"
#include "thread_log.h"

static inline void
_thread_queue_msg_clean(ThreadMsg *pMsg)
{
	if(pMsg != NULL)
	{
		if(pMsg->msg_body.msg_body != NULL)
		{
			if(pMsg->msg_body.mem_state == MsgMemState_uncaptured)
			{
				thread_free(pMsg->msg_body.msg_body, pMsg->msg_body.msg_id, (s8 *)__func__, (ub)__LINE__);
			}
			else if(pMsg->msg_body.mem_state == MsgMemState_captured)
			{
				pMsg->msg_body.mem_state = MsgMemState_uncaptured;
			}
		}

		thread_free((void *)pMsg, pMsg->msg_body.msg_id, (s8 *)__func__, (ub)__LINE__);
	}
}

static inline void
_thread_queue_booting(ThreadQueue *pQueue_ptr, ub queue_number)
{
	ub queue_index;
	ThreadQueue *pQueue;

	for(queue_index=0; queue_index<queue_number; queue_index++)
	{
		pQueue = &(pQueue_ptr[queue_index]);

		dave_memset(pQueue, 0x00, sizeof(ThreadQueue));

		t_lock_reset(&(pQueue->queue_opt_pv));
	}
}

static inline void
_thread_queue_malloc(ThreadQueue *pQueue_ptr, ub queue_number)
{
	ub queue_index;
	ThreadQueue *pQueue;

	for(queue_index=0; queue_index<queue_number; queue_index++)
	{
		pQueue = &(pQueue_ptr[queue_index]);

		SAFECODEv2W(pQueue->queue_opt_pv, {

			pQueue->on_queue_process = dave_false;

			pQueue->msg_number = 0;
			pQueue->queue_head = pQueue->queue_tail = NULL;

			pQueue->queue_received_counter = 0;
			pQueue->queue_processed_counter = 0;

		} );
	}
}

static inline void
_thread_queue_free(ThreadQueue *pQueue_ptr, ub queue_number)
{
	ub queue_index, list_index;
	ThreadQueue *pQueue;
	ThreadMsg *pNext;

	for(queue_index=0; queue_index<queue_number; queue_index++)
	{
		pQueue = &(pQueue_ptr[queue_index]);
	
		SAFECODEv2W(pQueue->queue_opt_pv, {

			for(list_index=0; list_index<pQueue->msg_number; list_index++)
			{
				if(pQueue->queue_head == NULL)
					break;

				pNext = pQueue->queue_head->next;
				_thread_queue_msg_clean(pQueue->queue_head);
				pQueue->queue_head = pNext;
			}

			if(list_index < pQueue->msg_number)
			{
				THREADLOG("Arithmetic error:%d/%d/%x",
					list_index, pQueue->msg_number,
					pQueue->queue_head);
			}

			pQueue->msg_number = 0;
			pQueue->queue_head = pQueue->queue_tail = NULL;

			pQueue->queue_received_counter = 0;
			pQueue->queue_processed_counter = 0;

		} );
	}
}

static inline void
_thread_queue_sum_total(ub *unprocessed, ub *received, ub *processed, ThreadQueue *pQueue_ptr, ub queue_number)
{
	ub queue_index, unprocessed_total_counter, list_received_counter, list_processed_counter;

	unprocessed_total_counter = list_received_counter = list_processed_counter = 0;

	for(queue_index=0; queue_index<queue_number; queue_index++)
	{
		unprocessed_total_counter += (pQueue_ptr[queue_index].msg_number);
		list_received_counter += (pQueue_ptr[queue_index].queue_received_counter);
		list_processed_counter += (pQueue_ptr[queue_index].queue_processed_counter);
	}

	*unprocessed = unprocessed_total_counter;
	*received = list_received_counter;
	*processed = list_processed_counter;
}

static inline ub
_thread_queue_list_total(ThreadQueue *pQueue_ptr, ub queue_number)
{
	ub queue_index, list_total_counter;
	ThreadQueue *pQueue;

	list_total_counter = 0;

	for(queue_index=0; queue_index<queue_number; queue_index++)
	{
		pQueue = &(pQueue_ptr[queue_index]);

		list_total_counter += (pQueue->msg_number);
	}

	return list_total_counter;
}

static inline ub
_thread_queue_num_msg_any(ThreadQueue *pQueue_ptr, ub queue_number)
{
	return _thread_queue_list_total(pQueue_ptr, queue_number);
}

static inline ub
_thread_queue_num_msg_id(ThreadQueue *pQueue_ptr, ub queue_number, ub msg_id)
{
	ub number, queue_index, list_index;
	ThreadQueue *pQueue;
	ThreadMsg *pNext;

	number = 0;

	for(queue_index=0; queue_index<queue_number; queue_index++)
	{
		pQueue = &(pQueue_ptr[queue_index]);

		SAFECODEv2TR(pQueue->queue_opt_pv, {

			pNext = pQueue->queue_head;

			for(list_index=0; list_index<pQueue->msg_number; list_index++)
			{
				if(pNext == NULL)
					break;

				if(pNext->msg_body.msg_id == msg_id)
				{
					number ++;
				}

				pNext = (ThreadMsg *)(pNext->next);
			}

			if(list_index < pQueue->msg_number)
			{
				THREADLOG("Arithmetic error:%d/%d/%x",
					list_index, pQueue->msg_number,
					pQueue->queue_head);
			}

		} );
	}

	return number;
}

static inline ub
_thread_queue_write(ThreadQueue *pQueue, ThreadMsg *pMsg)
{
	pQueue->msg_number ++;

	if(pQueue->queue_head == NULL)
	{
		pQueue->queue_head = pQueue->queue_tail = pMsg;
	}
	else
	{
		pQueue->queue_tail->next = pMsg;

		pQueue->queue_tail = pMsg;
	}

	pMsg->msg_body.msg_build_serial = pQueue->queue_received_counter ++;

	return pQueue->msg_number;
}

static inline ThreadMsg *
_thread_queue_read(ThreadQueue *pQueue)
{
	ThreadMsg *pMsg;

	pMsg = pQueue->queue_head;
	if(pMsg == NULL)
	{
		return NULL;
	}

	if(pQueue->queue_head == pQueue->queue_tail)
	{
		pQueue->queue_head = pQueue->queue_tail = NULL;
	}
	else
	{
		pQueue->queue_head = (ThreadMsg *)(pQueue->queue_head->next);
	}

	if(pQueue->msg_number == 0)
	{
		THREADLOG("Arithmetic error");
	}
	else
	{
		pQueue->msg_number --;
	}

	pQueue->queue_processed_counter ++;

	return pMsg;
}

static inline ThreadMsg *
_thread_queue_clone(ThreadQueue *pQueue)
{
	ThreadMsg *pMsg;

	if(pQueue->queue_head == NULL)
	{
		return NULL;
	}

	pMsg = dave_malloc(sizeof(ThreadMsg));

	*pMsg = *pQueue->queue_head;

	return pMsg;
}

// =====================================================================

void
thread_queue_booting(ThreadQueue *pQueue, ub queue_number)
{
	_thread_queue_booting(pQueue, queue_number);

	_thread_queue_malloc(pQueue, queue_number);
}

void
thread_queue_reset(ThreadQueue *pQueue, ub queue_number)
{
	_thread_queue_free(pQueue, queue_number);

	_thread_queue_malloc(pQueue, queue_number);
}

void
thread_queue_malloc(ThreadQueue *pQueue, ub queue_number)
{
	_thread_queue_malloc(pQueue, queue_number);
}

void
thread_queue_free(ThreadQueue *pQueue, ub queue_number)
{
	_thread_queue_free(pQueue, queue_number);
}

ub
thread_queue_write(ThreadQueue *pQueue, ThreadMsg *pMsg)
{
	ub msg_number = 0;

	SAFECODEv2W(pQueue->queue_opt_pv, msg_number = _thread_queue_write(pQueue, pMsg); );

	return msg_number;
}

ThreadMsg *
thread_queue_read(ThreadQueue *pQueue)
{
	ThreadMsg *pMsg = NULL;

	SAFECODEv2TW(pQueue->queue_opt_pv, {
	
		pMsg = _thread_queue_read(pQueue);
	
	});

	return pMsg;
}

ThreadMsg *
thread_queue_clone(ThreadQueue *pQueue)
{
	ThreadMsg *pMsg = NULL;

	SAFECODEv2TW(pQueue->queue_opt_pv, {
	
		pMsg = _thread_queue_clone(pQueue);
	
	});

	return pMsg;
}

ThreadMsg *
thread_queue_on_process_up(ThreadQueue *pQueue)
{
	ThreadMsg *pMsg = NULL;

	SAFECODEv2TW(pQueue->queue_opt_pv, {

		if(pQueue->on_queue_process == dave_false)
		{
			pMsg = _thread_queue_read(pQueue);
	
			if(pMsg != NULL)
			{
				pQueue->on_queue_process = dave_true;
	
				pMsg->pQueue = pQueue;
			}
		}

	});

	return pMsg;
}

void
thread_queue_on_process_down(ThreadQueue *pQueue)
{
	SAFECODEv2W(pQueue->queue_opt_pv, { pQueue->on_queue_process = dave_false; } );
}

void
thread_queue_total_detail(ub *unprocessed, ub *received, ub *processed, ThreadQueue *pQueue_ptr, ub queue_number)
{
	_thread_queue_sum_total(unprocessed, received, processed, pQueue_ptr, queue_number);
}

ub
thread_queue_total_number(ThreadQueue *pQueue_ptr, ub queue_number)
{
	return _thread_queue_list_total(pQueue_ptr, queue_number);
}

ub
thread_queue_total_msg(ThreadQueue *pQueue_ptr, ub queue_number, ub msg_id)
{
	if(msg_id == MSGID_RESERVED)
		return _thread_queue_num_msg_any(pQueue_ptr, queue_number);
	else
		return _thread_queue_num_msg_id(pQueue_ptr, queue_number, msg_id);
}

#endif

