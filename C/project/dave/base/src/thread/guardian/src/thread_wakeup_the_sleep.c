/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "thread_parameter.h"
#include "thread_mem.h"
#include "thread_struct.h"
#include "thread_thread.h"
#include "thread_tools.h"
#include "thread_quit.h"
#include "thread_statistics.h"
#include "thread_msg_buffer.h"
#include "thread_call.h"
#include "thread_log.h"

#define WAIT_SOME_TIME_THEN_WAKEUP 3

static void
_thread_wakeup_the_wakeup_failed_thread(ThreadStruct *thread_struct)
{
	ub thread_index;
	ThreadStruct *pThread;

	for(thread_index=0; thread_index<THREAD_MAX; thread_index++)
	{
		pThread = &thread_struct[thread_index];

		if((pThread->thread_id != INVALID_THREAD_ID)
			&& (pThread->thread_flag & THREAD_THREAD_FLAG)
			&& (pThread->has_not_wakeup_flag == dave_true))
		{
			thread_thread_wakeup(pThread->thread_index);
		}
	}
}

static void
_thread_wakeup_the_idle_thread(ThreadStruct *thread_struct)
{
	ub thread_index;
	ub msg_list_total, seq_list_total, current_idle_total;
	ThreadStruct *pThread;
	ub current_time = dave_os_time_s();

	for(thread_index=0; thread_index<THREAD_MAX; thread_index++)
	{
		pThread = &thread_struct[thread_index];

		if((pThread->thread_id != INVALID_THREAD_ID)
			&& (pThread->thread_flag & THREAD_THREAD_FLAG))
		{
			msg_list_total = thread_queue_list(pThread->msg_queue, THREAD_MSG_QUEUE_NUM);
			seq_list_total = thread_queue_list(pThread->seq_queue, THREAD_SEQ_QUEUE_NUM);

			current_idle_total = msg_list_total + seq_list_total;

			if((current_idle_total == 0) || (current_idle_total != pThread->message_idle_total))
			{
				pThread->message_idle_time = current_time;
				pThread->message_idle_total = current_idle_total;
			}
			else
			{
				if((current_time - pThread->message_idle_time) > WAIT_SOME_TIME_THEN_WAKEUP)
				{
					pThread->message_idle_time = current_time;
					pThread->message_idle_total = current_idle_total;
					pThread->message_wakeup_counter ++;

					thread_thread_wakeup(pThread->thread_index);
				}
			}
		}
	}
}

// =====================================================================

void
thread_wakeup_the_sleep(ThreadStruct *thread_struct)
{
	_thread_wakeup_the_wakeup_failed_thread(thread_struct);

	_thread_wakeup_the_idle_thread(thread_struct);
}

#endif

