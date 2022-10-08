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
#include "thread_parameter.h"
#include "thread_mem.h"
#include "thread_struct.h"
#include "thread_thread.h"
#include "thread_tools.h"
#include "thread_quit.h"
#include "thread_statistics.h"
#include "thread_call.h"
#include "thread_log.h"

#define MAX_WAIT_BUFFER 64

typedef struct {
	ub msg_id;
	ub counter;
} WaitMsgShow;

static WaitMsgShow *
_thread_wait_msg_get(WaitMsgShow *show_ptr, ub show_num, ub msg_id)
{
	ub empty_index, buffer_index;

	empty_index = show_num;

	for(buffer_index=0; buffer_index<show_num; buffer_index++)
	{
		if(show_ptr[buffer_index].msg_id == msg_id)
		{
			return &show_ptr[buffer_index];
		}

		if(show_ptr[buffer_index].msg_id == MSGID_RESERVED)
		{
			empty_index = buffer_index;
		}
	}

	if(empty_index >= show_num)
	{
		THREADLOG("empty(%d) show buffer!", empty_index);
		return NULL;
	}

	show_ptr[empty_index].msg_id = msg_id;
	show_ptr[empty_index].counter = 0;

	return &show_ptr[empty_index];
}

static void
_thread_wait_queue_show(WaitMsgShow *show_ptr, ub show_num, ThreadQueue *pQueue_ptr, ub queue_number)
{
	ub queue_index, list_index;
	ThreadQueue *pQueue;
	ThreadMsg *pMsg;
	WaitMsgShow *pShow;

	for(queue_index=0; queue_index<queue_number; queue_index++)
	{
		pQueue = &(pQueue_ptr[queue_index]);

		pMsg = pQueue->queue_head;

		for(list_index=0; list_index<pQueue->list_number; list_index++)
		{
			if(pMsg == NULL)
				break;

			pShow = _thread_wait_msg_get(show_ptr, show_num, pMsg->msg_body.msg_id);
			if(pShow != NULL)
			{
				pShow->counter ++;
			}

			pMsg = (ThreadMsg *)(pMsg->next);
		}
	}
}

static ub
_thread_wait_msg_show(ThreadStruct *pThread, s8 *msg_ptr, ub msg_len)
{
	WaitMsgShow pShow[MAX_WAIT_BUFFER];
	ub buffer_index, msg_index = 0;

	dave_memset(pShow, 0x00, sizeof(pShow));

	_thread_wait_queue_show(pShow, MAX_WAIT_BUFFER, pThread->msg_queue, THREAD_MSG_QUEUE_NUM);
	_thread_wait_queue_show(pShow, MAX_WAIT_BUFFER, pThread->seq_queue, THREAD_SEQ_QUEUE_NUM);

	for(buffer_index=0; buffer_index<MAX_WAIT_BUFFER; buffer_index++)
	{
		if((pShow[buffer_index].msg_id != MSGID_RESERVED)
			&& (pShow[buffer_index].counter > 0))
		{
			msg_index += dave_snprintf(&msg_ptr[msg_index], msg_len-msg_index,
				" msg_num:%04d msg_id:%s on:%s\n",
				pShow[buffer_index].counter,
				msgstr(pShow[buffer_index].msg_id),
				pThread->thread_name);
		}
	}

	return msg_index;
}

// =====================================================================

ub
thread_wait_msg_show(ThreadStruct *pThread, s8 *thread_name, s8 *msg_ptr, ub msg_len)
{
	ub msg_index, thread_index;

	msg_index = 0;

	if(thread_name == NULL)
	{
		msg_index += _thread_wait_msg_show(pThread, &msg_ptr[msg_index], msg_len-msg_index);
	}
	else
	{
		for(thread_index=0; thread_index<THREAD_MAX; thread_index++)
		{
			if((pThread[thread_index].thread_id != INVALID_THREAD_ID)
				&& (dave_strcmp(pThread[thread_index].thread_name, thread_name) == dave_true))
			{
				msg_index += _thread_wait_msg_show(&pThread[thread_index], &msg_ptr[msg_index], msg_len-msg_index);
				break;
			}
		}
	}

	return msg_index;
}

#endif

