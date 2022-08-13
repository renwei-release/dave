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
#include "dave_verno.h"
#include "thread_struct.h"
#include "thread_quit.h"
#include "thread_tools.h"
#include "thread_thread.h"
#include "thread_mem.h"
#include "thread_wait_msg_show.h"
#include "thread_chain.h"
#include "thread_log.h"

#define NOTIFY_MSG_MIN 16

static ThreadStruct *_thread = NULL;

static ub
_thread_msg_wait_info(ThreadStruct *pThread, s8 *msg_ptr, ub msg_len)
{
	ub msg_index, thread_index;

	msg_index = 0;

	for(thread_index=0; thread_index<THREAD_MAX; thread_index++)
	{
		if(pThread[thread_index].thread_name[0] != '\0')
		{
			msg_index += thread_wait_msg_show(&pThread[thread_index], NULL, &msg_ptr[msg_index], msg_len-msg_index);
		}
	}

	return msg_index;
}

static ub
_thread_info(ThreadStruct *pThread, s8 *msg_ptr, ub msg_len)
{
	ub msg_index, thread_index, printf_len;
	ub msg_list_total, msg_received_counter, msg_processed_counter;
	ub seq_list_total, seq_received_counter, seq_processed_counter;

	msg_index = 0;

	msg_index += dave_snprintf(&msg_ptr[msg_index], msg_len-msg_index,
		"===========%s============\n",
		VERSION_PRODUCT"."VERSION_MISC"."VERSION_MAIN"."VERSION_SUB"."VERSION_REV"."VERSION_DATE_TIME"."VERSION_LEVEL"\0");

	msg_index += dave_snprintf(&msg_ptr[msg_index], msg_len-msg_index, "THREAD INFORMATION:\n");
	for(thread_index=0; thread_index<THREAD_MAX; thread_index++)
	{
		if(pThread[thread_index].thread_id != INVALID_THREAD_ID)
		{
			thread_queue_total(&msg_list_total, &msg_received_counter, &msg_processed_counter, pThread[thread_index].msg_queue, THREAD_MSG_QUEUE_NUM);
			thread_queue_total(&seq_list_total, &seq_received_counter, &seq_processed_counter, pThread[thread_index].seq_queue, THREAD_SEQ_QUEUE_NUM);

			printf_len = dave_snprintf(&msg_ptr[msg_index], msg_len-msg_index, " %s(%02x)<%s%s><%s,%d>",
				pThread[thread_index].thread_name, pThread[thread_index].thread_id,
				pThread[thread_index].attrib==LOCAL_TASK_ATTRIB ? (pThread[thread_index].thread_flag&THREAD_PRIVATE_FLAG ? "P" : "L") : "R",
				pThread[thread_index].thread_flag&THREAD_COROUTINE_FLAG ? "-CO" : "",
				pThread[thread_index].thread_flag&THREAD_THREAD_FLAG ? "C" : "M", pThread[thread_index].level_number);
			msg_index += printf_len;

			msg_index += dave_snprintf(&msg_ptr[msg_index], msg_len-msg_index, "\t%s%s%sm%s:%lu/%lu:%lu %ss%s:%lu/%lu:%lu f:%s t:%s i:%lx/%lu/%lu\n",
				printf_len >= 24 ? "" : "\t",
				printf_len < 16 ? "\t" : "",
				(msg_list_total) >= NOTIFY_MSG_MIN ? "\033[31m" : "",
				(msg_list_total) >= NOTIFY_MSG_MIN ? "\033[0m" : "",
				msg_received_counter, msg_processed_counter,
				msg_list_total,
				(seq_list_total) >= NOTIFY_MSG_MIN ? "\033[31m" : "",
				(seq_list_total) >= NOTIFY_MSG_MIN ? "\033[0m" : "",
				seq_received_counter, seq_processed_counter,
				seq_list_total,
				thread_name(pThread[thread_index].father),
				pThread[thread_index].trace_on==dave_true ? "on" : "off",
				pThread[thread_index].message_idle_time, pThread[thread_index].message_idle_total, pThread[thread_index].message_wakeup_counter);
		}
	}

	return msg_index;
}

// =====================================================================

void
thread_tools_init(ThreadStruct *thread_struct)
{
	_thread = thread_struct;

	thread_map_init();
}

void
thread_tools_exit(void)
{
	thread_map_exit();
}

ThreadStruct *
__thread_find_busy_thread__(ThreadId thread_id, s8 *fun, ub line)
{
	ub thread_index;

	thread_id = thread_get_local(thread_id);

	if((thread_id == INVALID_THREAD_ID) || (thread_id < 0))
	{
		THREADABNOR("get invalid thread_id:%d <%s:%d>", thread_id, fun, line);
		return NULL;
	}

	thread_index = (ub)(thread_id % THREAD_MAX);
	if(_thread[thread_index].thread_name[0] == '\0')
		return NULL;

	return &_thread[thread_index];
}

ub
__thread_find_busy_index__(ThreadId thread_id, s8 *fun, ub line)
{
	ub thread_index;

	thread_id = thread_get_local(thread_id);

	if((thread_id == INVALID_THREAD_ID) || (thread_id < 0))
	{
		THREADABNOR("get invalid thread_id:%d <%s:%d>",
			thread_id, fun, line);
		return THREAD_MAX;
	}

	thread_index = (ub)(thread_id % THREAD_MAX);
	if(_thread[thread_index].thread_name[0] == '\0')
		return THREAD_MAX;

	return thread_index;
}

ub
thread_find_free_index(s8 *thread_name)
{
	ub thread_index = 0;
	ub name_index;
	ub task_count;

	if(thread_name != NULL)
	{
		thread_index = 0;

		for(name_index=0; name_index<THREAD_NAME_MAX; name_index++)
		{
			if(thread_name[name_index] == '\0')
				break;

			thread_index *= 10;

			thread_index += (ub)(thread_name[name_index]);
		}

		thread_index = thread_index % THREAD_MAX;
	}

	for(task_count=0; task_count<THREAD_MAX; task_count++)
	{
		if(thread_index >= THREAD_MAX)
		{
			thread_index = 0;
		}

		if(thread_name != NULL)
		{
			if(dave_strcmp(thread_name, (s8 *)(_thread[thread_index].thread_name)) == dave_true)
			{
				break;
			}
		}

		if(_thread[thread_index].thread_id == INVALID_THREAD_ID)
		{
			break;
		}

		thread_index ++;
	}

	THREADDEBUG("name:%s index:%d", thread_name, thread_index);

	if(task_count >= THREAD_MAX)
	{
		return THREAD_MAX;
	}
	else
	{
		return thread_index % THREAD_MAX;
	}
}

ub
thread_show_all_info(ThreadStruct *pThread, DateStruct *pWorkDate, s8 *msg_ptr, ub msg_len, dave_bool base_flag)
{
	ub msg_index;

	msg_index = 0;

	if(base_flag == dave_true)
	{
		msg_index += _thread_info(pThread, &msg_ptr[msg_index], msg_len-msg_index);
		msg_index += dave_snprintf(&msg_ptr[msg_index], msg_len-msg_index,
			"============================================================\n");
	}
	msg_index += _thread_msg_wait_info(pThread, &msg_ptr[msg_index], msg_len-msg_index);
	msg_index += base_mem_info(&msg_ptr[msg_index], msg_len-msg_index, base_flag);
	msg_index += thread_memory_info(&msg_ptr[msg_index], msg_len-msg_index, base_flag);
	if(pWorkDate != NULL)
	{
		msg_index += dave_snprintf(&msg_ptr[msg_index], msg_len-msg_index,
			"SYSTEM BOOT TIME:%s\n", t_a2b_date_str(pWorkDate));
	}

	if(msg_index == 0)
	{
		msg_index += dave_snprintf(&msg_ptr[msg_index], msg_len-msg_index,
			"Empty info!\n");
	}

	return msg_index;
}

void
thread_check_pair_msg(ub req_id, ub rsp_id)
{
	if((req_id + 1) != rsp_id)
	{
		THREADLOG("We want the request and response messages to be contiguous.<%d/%d>",
			req_id, rsp_id);
	}
}

void
thread_reset_sync(ThreadSync *pSync)
{
	pSync->wait_thread = INVALID_THREAD_ID;
	pSync->wait_msg = MSGID_INVALID;
	pSync->wait_body = NULL;
	pSync->wait_len = 0;
}

void
__thread_clean_user_input_data__(void *msg_body, ub msg_id, s8 *fun, ub line)
{
	if(thread_memory_at_here(msg_body) == dave_true)
	{
		thread_free((void *)msg_body, msg_id, fun, line);
	}
}

dave_bool
thread_enable_coroutine(ThreadStruct *pThread)
{
	if((pThread->thread_flag & THREAD_THREAD_FLAG)
		&& (pThread->thread_flag & THREAD_COROUTINE_FLAG))
		return dave_true;
	else
		return dave_false;
}

ThreadMsg *
thread_build_msg(
	ThreadStruct *pThread,
	void *msg_chain,
	ThreadId src_id, ThreadId dst_id,
	ub msg_id, ub msg_len, u8 *msg_body,
	BaseMsgType msg_type,
	s8 *fun, ub line)
{
	dave_bool data_is_here = thread_memory_at_here((void *)msg_body);
	ThreadMsg *thread_msg;

	thread_msg = (ThreadMsg *)thread_malloc(sizeof(ThreadMsg), msg_id, fun, line);
	if(data_is_here == dave_false)
	{
		THREADDEBUG("Please use thread_msg function to build the msg:%s buffer! <%s:%d>",
			msgstr(msg_id), fun, line);
		thread_msg->msg_body.msg_body = thread_malloc(msg_len, msg_id, fun, line);
		dave_memcpy(thread_msg->msg_body.msg_body, msg_body, msg_len);
	}
	else
	{
		thread_msg->msg_body.msg_body = (void *)msg_body;
	}

	thread_msg->msg_body.msg_src = src_id;
	if(dst_id != INVALID_THREAD_ID)
	{
		thread_msg->msg_body.msg_dst = dst_id;
	}
	else
	{
		if(pThread == NULL)
			thread_msg->msg_body.msg_dst = INVALID_THREAD_ID;
		else
			thread_msg->msg_body.msg_dst = pThread->thread_id;
	}
	thread_msg->msg_body.msg_id = msg_id;
	thread_msg->msg_body.msg_type = msg_type;
	thread_msg->msg_body.src_attrib = base_thread_attrib(src_id);
	if(dst_id != INVALID_THREAD_ID)
	{
		thread_msg->msg_body.dst_attrib = REMOTE_TASK_ATTRIB;
	}
	else
	{
		if(pThread == NULL)
			thread_msg->msg_body.dst_attrib = LOCAL_TASK_ATTRIB;
		else
			thread_msg->msg_body.dst_attrib = pThread->attrib;
	}
	thread_msg->msg_body.msg_len = msg_len;
	thread_msg->msg_body.mem_state = MsgMemState_uncaptured;

	thread_msg->msg_body.msg_build_time = 0;
	thread_msg->msg_body.msg_build_serial = 0;

	thread_msg->msg_body.user_ptr = NULL;

	thread_msg->pQueue = NULL;
	thread_msg->next = NULL;

	thread_msg->msg_body.msg_chain = thread_chain_build_msg(
			msg_chain,
			src_id, dst_id, msg_id,
			fun, line);

	return thread_msg;
}

void
thread_clean_msg(ThreadMsg *pMsg)
{
	ThreadQueue *pQueue;

	if(pMsg != NULL)
	{
		pQueue = (ThreadQueue *)(pMsg->pQueue);
		if(pQueue != NULL)
		{
			thread_queue_reset_process(pQueue);
		}

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

		thread_chain_clean_msg(&(pMsg->msg_body));

		thread_free((void *)pMsg, pMsg->msg_body.msg_id, (s8 *)__func__, (ub)__LINE__);
	}
}

ThreadChain *
thread_current_chain(void)
{
	ThreadId current_id;

	if(thread_thread_is_main() == dave_true)
	{
		current_id = self();
		if(current_id == INVALID_THREAD_ID)
		{
			return NULL;
		}

		return &(_thread[current_id].chain);
	}
	else
	{
		return thread_thread_chain();
	}
}

s8 *
thread_id_to_name(ThreadId thread_id)
{
	thread_id = thread_get_local(thread_id);

	if(thread_id >= THREAD_MAX)
	{
		return "NULL";
	}

	return _thread[thread_id].thread_name;
}

TaskAttribute
thread_id_to_attrib(ThreadId thread_id)
{
	thread_id = thread_get_local(thread_id);

	if(thread_id >= THREAD_MAX)
	{
		return RESERVED_TASK_ATTRIB;
	}

	return _thread[thread_id].attrib;
}

#endif

