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
#include "dave_base.h"
#include "dave_verno.h"
#include "thread_struct.h"
#include "thread_thread.h"
#include "thread_quit.h"
#include "thread_map.h"
#include "thread_mem.h"
#include "thread_guardian.h"
#include "thread_tools.h"
#include "thread_msg_buffer.h"
#include "thread_call.h"
#include "thread_statistics.h"
#include "thread_broadcast.h"
#include "thread_sync.h"
#include "thread_remote_id_table.h"
#include "thread_gid_table.h"
#include "thread_seq_msg.h"
#include "thread_log.h"

#define THREAD_MSG_MAX_LEN (24 * 1024 * 1024)
#define SYSTEM_READY_COUNTER 64

typedef struct {
	ThreadId thread_id;
	ub thread_level;
	ub thread_index;
} ThreadPriority;

typedef struct {
	volatile void *next_stack;
	ThreadId thread_id;
} ThreadStack;

static volatile dave_bool __system_startup__ = dave_false;
static void *_main_thread_id = NULL;
static volatile sb _system_wakeup_counter = 0;
static TLock _system_thread_pv;
static s8 _system_thread_pv_lock_name[THREAD_NAME_MAX];
static volatile ub _system_schedule_counter = 0;

static ThreadStruct _thread[THREAD_MAX];
static ThreadPriority _msg_priority[THREAD_MAX];
static volatile ThreadStack *_current_msg_stack = NULL;
static ThreadStack _top_msg_stack;
static ThreadId _guardian_thread = INVALID_THREAD_ID;

static void
_thread_reset(ThreadStruct *pThread)
{
	ub thread_index, child_index;

	thread_index = pThread->thread_index;

	pThread->thread_id = INVALID_THREAD_ID;
	dave_memset(pThread->thread_name, 0x00, THREAD_NAME_MAX);
	pThread->level_number = 255;
	pThread->thread_flag = 0x00;
	pThread->attrib = EMPTY_TASK_ATTRIB;
	pThread->thread_init = NULL;
	pThread->thread_main = NULL;
	pThread->thread_exit = NULL;

	pThread->father = INVALID_THREAD_ID;
	for(child_index=0; child_index<THREAD_MAX; child_index++)
	{
		pThread->child[child_index] = INVALID_THREAD_ID;
	}

	pThread->message_idle_time = dave_os_time_s();
	pThread->message_idle_total = 0;
	pThread->message_wakeup_counter = 0;

	thread_queue_all_reset(pThread);

	thread_reset_sync(&(pThread->sync));

	pThread->has_not_wakeup_flag = dave_false;

	pThread->thread_index = thread_index;

	pThread->trace_on = dave_false;
}

static void
_thread_reset_all(void)
{
	ub thread_index, queue_index;

	for(thread_index=0; thread_index<THREAD_MAX; thread_index++)
	{
		dave_memset(&_thread[thread_index], 0x00, sizeof(ThreadStruct));

		_thread[thread_index].thread_index = thread_index;

		for(queue_index=0; queue_index<THREAD_MSG_QUEUE_NUM; queue_index++)
		{
			t_lock_reset(&(_thread[thread_index].msg_queue[queue_index].queue_opt_pv));
		}
		for(queue_index=0; queue_index<THREAD_SEQ_QUEUE_NUM; queue_index++)
		{
			t_lock_reset(&(_thread[thread_index].seq_queue[queue_index].queue_opt_pv));
		}
		t_lock_reset(&(_thread[thread_index].sync.sync_pv));

		_thread_reset(&_thread[thread_index]);
	}
}

static inline TaskAttribute
_thread_msg_attrib(ThreadStruct *pThread)
{
	return pThread->attrib;
}

#define _thread_get_id(name) _thread_get_id_(name, (s8 *)__func__, (ub)__LINE__)
static inline ThreadId
_thread_get_id_(const s8 *name, s8 *fun, ub line)
{
	ThreadStruct *pThread;
	ub thread_index;
	ThreadId thread_id = INVALID_THREAD_ID;

	pThread = __thread_map_name__((s8 *)name, fun, line);
	if(pThread != NULL)
	{
		thread_id = thread_get_local(pThread->thread_id);
	}

	if(thread_id == INVALID_THREAD_ID)
	{
		for(thread_index=0; (thread_index<THREAD_MAX)&&(thread_id==INVALID_THREAD_ID); thread_index++)
		{
			if(_thread[thread_index].thread_id != INVALID_THREAD_ID)
			{
				if(dave_strcmp((s8 *)(_thread[thread_index].thread_name), (s8 *)name) == dave_true)
				{
					thread_id = thread_get_local(_thread[thread_index].thread_id);

					break;
				}
			}
		}
	}

	THREADDEBUG("find thread<%s> id:%d!", name, thread_id);

	return thread_id;
}

static inline TaskAttribute
_thread_attrib(ThreadId thread_id)
{
	ub thread_index;

	thread_id = thread_get_local(thread_id);

	if(thread_id == INVALID_THREAD_ID)
	{
		return EMPTY_TASK_ATTRIB;
	}

	thread_index = thread_find_busy_index(thread_id);
	if(thread_index >= THREAD_MAX)
	{
		THREADDEBUG("");
		return EMPTY_TASK_ATTRIB;
	}

	return _thread[thread_index].attrib;
}

#define _thread_get_name(thread_id) _thread_get_name_(thread_id, (s8 *)__func__, (ub)__LINE__)
static inline s8 *
_thread_get_name_(ThreadId thread_id, s8 *fun, ub line)
{
	static s8 can_not_find[] = "NULL";
	ub thread_index;

	thread_id = thread_sync_thread_id(thread_id);

	thread_id = thread_get_local(thread_id);

	if(thread_id == INVALID_THREAD_ID)
	{
		return (s8 *)can_not_find;
	}

	thread_index = thread_find_busy_index(thread_id);
	if(thread_index >= THREAD_MAX)
	{
		THREADDEBUG("thread_id:%d", thread_id);
		return (s8 *)can_not_find;
	}

	return (s8 *)(_thread[thread_index].thread_name);
}

static inline void
_thread_sleep_main(void)
{
	sb pv = 0;

	thread_lock();
	_system_wakeup_counter --;
	if(_system_wakeup_counter < 0)
	{
		_system_wakeup_counter = 0;
		pv = 1;
	}
	thread_unlock();

	if(pv == 1)
	{
		dave_os_thread_sleep(_main_thread_id);
	}
}

static inline void
_thread_wakeup_main(void)
{
	thread_lock();
	_system_wakeup_counter ++;
	thread_unlock();

	dave_os_thread_wakeup(_main_thread_id);
}

static inline void
_thread_wakeup_thread(ThreadStruct *pThread)
{
	if(thread_thread_wakeup(pThread->thread_index) == dave_false)
	{
		pThread->has_not_wakeup_flag = dave_true;
	}
	else
	{
		pThread->has_not_wakeup_flag = dave_false;
	}
}

static inline void
_thread_sleep(void)
{
	_thread_sleep_main();
}

static inline void
_thread_wakeup(ThreadStruct *pThread)
{
	if(pThread->thread_flag & THREAD_THREAD_FLAG)
	{
		_thread_wakeup_thread(pThread);
	}
	else
	{
		_thread_wakeup_main();
	}
}

static inline ErrCode
_thread_safe_write_seq_queue(ThreadStruct *pThread, ThreadMsg *pMsg)
{
	sb queue_index;
	ThreadQueue *pQueue;

	queue_index = thread_is_seq_msg(pMsg);
	if(queue_index < 0)
	{
		return ERRCODE_not_my_data;
	}
	pQueue = &(pThread->seq_queue[queue_index % THREAD_SEQ_QUEUE_NUM]);

	return thread_queue_write(pQueue, pMsg);
}

static inline ThreadMsg *
_thread_safe_read_seq_queue(ThreadStruct *pThread)
{
	ub queue_index, safe_counter;
	ThreadQueue *pQueue;
	ThreadMsg *pMsg = NULL;

	queue_index = (pThread->seq_queue_read_sequence ++) % THREAD_SEQ_QUEUE_NUM;

	for(safe_counter=0; (pMsg==NULL)&&(safe_counter<THREAD_SEQ_QUEUE_NUM); safe_counter++)
	{
		if(queue_index >= THREAD_SEQ_QUEUE_NUM)
			queue_index = 0;

		pQueue = &(pThread->seq_queue[queue_index ++]);

		if((pQueue->list_number > 0)
			&& (pQueue->on_queue_process == dave_false))
		{
			pMsg = thread_queue_read(pQueue, dave_true);
		}
	}

	return pMsg;
}

static inline ErrCode
_thread_safe_write_msg_queue(ThreadStruct *pThread, ThreadMsg *pMsg)
{
	ub queue_index, safe_counter;
	ThreadQueue *pQueue;
	ErrCode ret = ERRCODE_Send_msg_failed;

	queue_index = (pThread->msg_queue_write_sequence ++) % THREAD_MSG_QUEUE_NUM;

	for(safe_counter=0; (ret!=ERRCODE_OK)&&(safe_counter<THREAD_MSG_QUEUE_NUM); safe_counter++)
	{
		if(queue_index >= THREAD_MSG_QUEUE_NUM)
			queue_index = 0;

		pQueue = &(pThread->msg_queue[queue_index ++]);

		ret = thread_queue_write(pQueue, pMsg);
	}

	return ret;
}

static inline ThreadMsg *
_thread_safe_read_msg_queue(ThreadStruct *pThread)
{
	ub queue_index, safe_counter;
	ThreadQueue *pQueue;
	ThreadMsg *pMsg = NULL;

	queue_index = (pThread->msg_queue_read_sequence ++) % THREAD_MSG_QUEUE_NUM;

	for(safe_counter=0; (pMsg==NULL)&&(safe_counter<THREAD_MSG_QUEUE_NUM); safe_counter++)
	{
		if(queue_index >= THREAD_MSG_QUEUE_NUM)
			queue_index = 0;

		pQueue = &(pThread->msg_queue[queue_index ++]);

		if(pQueue->list_number > 0)
		{
			pMsg = thread_queue_read(pQueue, dave_false);
		}
	}

	return pMsg;
}

static inline ThreadMsg *
_thread_build_msg(
	ThreadStruct *pThread,
	ThreadId src_id, ThreadId route_dst_id,
	ub msg_id, ub data_len, u8 *data,
	BaseMsgType msg_type,
	s8 *fun, ub line)
{
	dave_bool data_is_here = thread_memory_at_here((void *)data);
	ThreadMsg *task_msg;

	task_msg = (ThreadMsg *)thread_malloc(sizeof(ThreadMsg), msg_id, (s8 *)__func__, (ub)__LINE__);
	if(data_is_here == dave_false)
	{
		THREADTRACE("Please use thread_msg function to build the msg:%d buffer! <%s:%d>", msg_id, fun, line);
		task_msg->msg_body.msg_body = thread_malloc(data_len, msg_id, (s8 *)__func__, (ub)__LINE__);
	}

	task_msg->msg_body.msg_src = src_id;
	if(route_dst_id != INVALID_THREAD_ID)
		task_msg->msg_body.msg_dst = route_dst_id;
	else
		task_msg->msg_body.msg_dst = pThread->thread_id;
	task_msg->msg_body.msg_id = msg_id;
	task_msg->msg_body.msg_type = msg_type;
	task_msg->msg_body.src_attrib = _thread_attrib(src_id);
	if(route_dst_id != INVALID_THREAD_ID)
		task_msg->msg_body.dst_attrib = REMOTE_TASK_ATTRIB;
	else
		task_msg->msg_body.dst_attrib = _thread_msg_attrib(pThread);
	task_msg->msg_body.msg_len = data_len;
	if(data_is_here == dave_true)
	{
		task_msg->msg_body.msg_body = (void *)data;
	}
	else
	{
		dave_memcpy(task_msg->msg_body.msg_body, data, data_len);
	}
	task_msg->msg_body.mem_state = MsgMemState_uncaptured;

	task_msg->msg_body.msg_build_time = 0;
	task_msg->msg_body.msg_build_serial = 0;

	task_msg->msg_body.user_ptr = NULL;

	task_msg->pQueue = task_msg->next = NULL;

	return task_msg;
}

static inline void
_thread_clean_msg(ThreadMsg *pMsg, dave_bool carried_out)
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

		thread_free((void *)pMsg, pMsg->msg_body.msg_id, (s8 *)__func__, (ub)__LINE__);
	}
}

static inline ErrCode
_thread_write_msg(
	ThreadId src_id, ThreadId dst_id,
	ub dst_thread_index,
	ub msg_id, ub data_len, u8 *data,
	dave_bool wakeup, BaseMsgType msg_type,
	ThreadId route_dst_id,
	s8 *fun, ub line)
{
	ThreadStruct *pDstThread;
	ThreadMsg *pMsg;
	ErrCode ret;

	pDstThread = &_thread[dst_thread_index];

	if(thread_call_sync_catch(pDstThread, dst_id, src_id, msg_id, data, data_len) == dave_false)
	{
		pMsg = _thread_build_msg(
					pDstThread,
					src_id, route_dst_id,
					msg_id, data_len, data,
					msg_type,
					fun, line);

		thread_statistics_write_msg_time(pMsg);

		ret = _thread_safe_write_seq_queue(pDstThread, pMsg);
		if(ret == ERRCODE_not_my_data)
		{
			ret = _thread_safe_write_msg_queue(pDstThread, pMsg);
		}

		if(ret != ERRCODE_OK)
		{
			THREADLTRACE(60,1,"failed:%s %s->%s %d",
				errorstr(ret),
				_thread_get_name(pMsg->msg_body.msg_src),
				_thread_get_name(pMsg->msg_body.msg_dst),
				pMsg->msg_body.msg_id);

			_thread_clean_msg(pMsg, dave_false);
		}
	}
	else
	{
		ret = ERRCODE_OK;
		thread_clean_user_input_data(data, msg_id);
	}

	if(wakeup == dave_true)
	{
		_thread_wakeup(pDstThread);
	}

	return ret;
}

static inline ThreadMsg *
_thread_read_msg(ThreadStruct *pThread)
{
	ThreadMsg *pMsg;

	pMsg = _thread_safe_read_seq_queue(pThread);

	if(pMsg == NULL)
	{
		pMsg = _thread_safe_read_msg_queue(pThread);
	}

	thread_statistics_read_msg_time(pMsg);

	return pMsg;
}

static inline ub
_thread_num_msg(ThreadStruct *pThread, ub msg_id)
{
	ub number;

	if(pThread == NULL)
	{
		return 0;
	}

	number = thread_queue_num_msg(pThread->msg_queue, THREAD_MSG_QUEUE_NUM, msg_id);
	number += thread_queue_num_msg(pThread->seq_queue, THREAD_SEQ_QUEUE_NUM, msg_id);

	return number;
}

static inline void
_thread_clean_priority(void)
{
	ub priority_index;

	for(priority_index=0; priority_index<THREAD_MAX; priority_index++)
	{
		_msg_priority[priority_index].thread_id = INVALID_THREAD_ID;
		_msg_priority[priority_index].thread_level = 512;
		_msg_priority[priority_index].thread_index = THREAD_MAX;
	}	
}

static inline void
_thread_copy_priority(ThreadPriority *priority, ThreadStruct *task, ub thread_index)
{
	priority->thread_id = task->thread_id;
	if(priority->thread_id == INVALID_THREAD_ID)
	{
		priority->thread_level = 255;
		priority->thread_index = THREAD_MAX;
	}
	else
	{
		if(task->thread_flag & THREAD_THREAD_FLAG)
		{
			priority->thread_level = 255;
		}
		else
		{
			priority->thread_level = task->level_number;
		}
		priority->thread_index = thread_index;
	}
}

static inline void
_thread_sorting_thread(void)
{
	ub index, index2;
	ThreadPriority temp_task;

	_thread_clean_priority();

	for(index=0; index<THREAD_MAX; index++)
	{
		if(!(_thread[index].thread_flag & THREAD_THREAD_FLAG))
		{
			_thread_copy_priority(&_msg_priority[index], &_thread[index], index);
		}
	}

	for(index=0; index<THREAD_MAX; index++)
	{		
		for(index2=(index+1); index2<THREAD_MAX; index2++)
		{
			if((_msg_priority[index].thread_level > _msg_priority[index2].thread_level)
				|| (_msg_priority[index].thread_id == INVALID_THREAD_ID))
			{
				temp_task.thread_id = _msg_priority[index].thread_id;
				temp_task.thread_level = _msg_priority[index].thread_level;
				temp_task.thread_index = _msg_priority[index].thread_index;
				_msg_priority[index].thread_id = _msg_priority[index2].thread_id;
				_msg_priority[index].thread_level = _msg_priority[index2].thread_level;
				_msg_priority[index].thread_index = _msg_priority[index2].thread_index;
				_msg_priority[index2].thread_id = temp_task.thread_id;
				_msg_priority[index2].thread_level = temp_task.thread_level;
				_msg_priority[index2].thread_index = temp_task.thread_index;
			}
		}
	}
}

void
_thread_run_call_back_user_fun(
	base_thread_fun thread_fun, thread_msg_fun msg_fun,
	ThreadStruct *pThread,
	MSGBODY *msg,
	dave_bool enable_stack)
{
	ub run_time = 0;
	ThreadStack *stack = NULL;

	if((pThread == NULL) || (msg == NULL))
	{
		THREADABNOR("pThread:%x or msg:%x is NULL!", pThread, msg);
		return;
	}

	if(pThread->thread_id != thread_get_local(msg->msg_dst))
	{
		THREADLOG("schedule error! thread:%s<%d> msg:%s<%d>->%s<%d> %d %d",
			pThread->thread_name, pThread->thread_id,
			_thread_get_name(msg->msg_src), msg->msg_src,
			_thread_get_name(msg->msg_dst), msg->msg_dst,
			msg->msg_id,
			msg->thread_wakeup_index);
	}

#ifdef DEBUG_THREAD_RUN_INFO
	if(msg->msg_id != MSGID_WAKEUP)
	{
		THREADDEBUG(">>>> run src:%s<%x> dst:%s<%x> msg_id:%lx",
			_thread_get_name(msg->msg_src), msg->msg_src,
			_thread_get_name(msg->msg_dst), msg->msg_dst,
			msg->msg_id);
	}
#endif

	run_time = thread_statistics_run_start();

	if(enable_stack == dave_true)
	{
		stack = thread_malloc(sizeof(ThreadStack), MSGID_RESERVED, (s8 *)__func__, (ub)__LINE__);

		thread_lock();
		stack->next_stack = _current_msg_stack;
		stack->thread_id = pThread->thread_id;
		_current_msg_stack = stack;
		thread_unlock();
	}

	if(thread_fun != NULL)
	{
		thread_fun(msg);
	}
	else if(msg_fun != NULL)
	{
		msg_fun(msg);
	}

	if(enable_stack == dave_true)
	{
		thread_lock();
        if(_current_msg_stack != NULL)
        {
            _current_msg_stack = _current_msg_stack->next_stack;
        }
		thread_unlock();

		thread_free(stack, MSGID_RESERVED, (s8 *)__func__, (ub)__LINE__);
	}

	thread_statistics_run_end(run_time, pThread, msg);

#ifdef DEBUG_THREAD_RUN_INFO
	if(msg->msg_id != MSGID_WAKEUP)
	{
		THREADDEBUG("run task end! <<<<");
	}
#endif
}

static void
_thread_build_genealogy(s8 *child_name, ub child_thread_index, ThreadId child_thread_id)
{
	ub father_msg_index;

	if(child_thread_index >= THREAD_MAX)
	{
		THREADABNOR("invalid child_thread_index:%d", child_thread_index);
		return;
	}

	_thread[child_thread_index].father = get_self();
	if(_thread[child_thread_index].father != INVALID_THREAD_ID)
	{
		father_msg_index = thread_find_busy_index(_thread[child_thread_index].father);
		if(father_msg_index >= THREAD_MAX)
		{
			THREADABNOR("build task genealogy fail! father:%s<%d><%d> child:%s<%d>",
				_thread_get_name(_thread[child_thread_index].father), _thread[child_thread_index].father, child_thread_index,
				_thread_get_name(child_thread_id), child_thread_id);
			return;
		}

		if(_thread[father_msg_index].child[child_thread_index] != INVALID_THREAD_ID)
		{
			THREADABNOR("task system abnormal relationship between genealogy! father:%s child:%s<%d> here thread id:%d",
				_thread_get_name(_thread[child_thread_index].father),
				child_name, child_thread_id,
				_thread[father_msg_index].child[child_thread_index]);
			return;
		}

		_thread[father_msg_index].child[child_thread_index] = child_thread_id;
	}
}

static void
_thread_remove_genealogy(ub child_msg_index)
{
	ThreadStruct *pThread, *pFThread;
	ub father_msg_index, child_index;

	if(child_msg_index >= THREAD_MAX)
	{
		THREADABNOR("invalid child_msg_index:%d", child_msg_index);
		return;
	}

	pThread = &_thread[child_msg_index];

	if(pThread->father == INVALID_THREAD_ID)
	{
		// no father.
		return;
	}

	// Clear relationship between father and son.
	father_msg_index = thread_find_busy_index(pThread->father);
	if(father_msg_index >= THREAD_MAX)
	{
		THREADDEBUG("invalid father_msg_index:%d", father_msg_index);
		return;
	}

	pFThread = &_thread[father_msg_index];

	pFThread->child[child_msg_index] = INVALID_THREAD_ID;

	// Grandfather inherited.
	for(child_index=0; child_index<THREAD_MAX; child_index++)
	{
		if((pFThread->child[child_index] != INVALID_THREAD_ID)
			&& (pThread->child[child_index] != INVALID_THREAD_ID))
		{
			THREADABNOR("interited father %s:%s child %s",
				_thread_get_name(pFThread->thread_id),
				_thread_get_name(pFThread->child[child_index]),
				_thread_get_name(pThread->child[child_index]));
			continue;
		}

		pFThread->child[child_index] = pThread->child[child_index];
	}

	pThread->father = INVALID_THREAD_ID;
	for(child_index=0; child_index<THREAD_MAX; child_index++)
	{
		pThread->child[child_index] = INVALID_THREAD_ID;
	}
}

static void
_thread_build_tick_message(void)
{
	ub priority_index;
	ub thread_index;
	ThreadStruct *pThread;
	WAKEUPMSG *pWakeup;
	
	// Build TICK message. 
	for(priority_index=0; priority_index<THREAD_MAX; priority_index++)
	{
		if(_msg_priority[priority_index].thread_id != INVALID_THREAD_ID)
		{
			thread_index = _msg_priority[priority_index].thread_index;
			if(thread_index < THREAD_MAX)
			{
				pThread = &_thread[thread_index];

				if(pThread->thread_flag & THREAD_TICK_WAKEUP)
				{
					if(_thread_num_msg(pThread, MSGID_RESERVED) == 0)
					{
						pWakeup = thread_msg(pWakeup);

						_thread_write_msg(
							_guardian_thread, pThread->thread_id,
							thread_index,
							MSGID_WAKEUP, sizeof(WAKEUPMSG), (u8 *)pWakeup,
							dave_false, BaseMsgType_Unicast,
							INVALID_THREAD_ID,
							(s8 *)__func__, (ub)__LINE__);
					}
				}
			}
		}
	}
}

static void
_thread_give_wakeup_on_some_time(void)
{
	static ub last_second = 0;
	ub current_second = dave_os_time_s();

	if((base_power_state() == dave_true)
		&& ((current_second - last_second) >= THREAD_WAKEUP_INTERVAL))
	{
		last_second = current_second;

		_thread_build_tick_message();
	}
}

static void
_thread_schedule_predecessor_task(void)
{
	_system_schedule_counter ++;

	_thread_give_wakeup_on_some_time();
}

static ub
_thread_schedule_one_thread(ub thread_index, ThreadId thread_id, s8 *thread_name, ub wakeup_index, dave_bool enable_stack)
{
	ThreadStruct *pThread;
	ThreadMsg *pMsg;
	MSGBODY *msg_body;
	MsgCallFun *msgcall_fun;

	if(thread_index >= THREAD_MAX)
	{
		THREADABNOR("invalid thread_index:%d", thread_index);
		return 0;
	}

	pThread = &_thread[thread_index];

	if(pThread->thread_id == INVALID_THREAD_ID)
	{
		THREADLOG("id:%d index:%d name:%s/%s wakeup:%d has already withdrawn!",
			thread_id, thread_index, pThread->thread_name, thread_name, wakeup_index);

		return _thread_num_msg(pThread, MSGID_RESERVED);
	}

	if(thread_id != pThread->thread_id)
	{
		THREADABNOR("thread_id mismatch! %d,%d/%s,%d",
			thread_id, pThread->thread_id, pThread->thread_name, thread_index);

		return _thread_num_msg(pThread, MSGID_RESERVED);
	}

	if(pThread->has_initialization == dave_true)
	{
		pMsg = _thread_read_msg(pThread);
		if(pMsg != NULL)
		{
			msg_body = &(pMsg->msg_body);

			msg_body->thread_wakeup_index = wakeup_index;

			msgcall_fun = thread_call_msg(msg_body->msg_dst, msg_body->msg_id);

			if(msgcall_fun != NULL)
			{
				msg_body->user_ptr = msgcall_fun->user_ptr;

				_thread_run_call_back_user_fun(
					NULL, msgcall_fun->msg_fun,
					pThread,
					msg_body, enable_stack);
			}
			else
			{
				_thread_run_call_back_user_fun(
					pThread->thread_main, NULL,
					pThread,
					msg_body, enable_stack);
			}

			_thread_clean_msg(pMsg, dave_true);
		}
	}

	return _thread_num_msg(pThread, MSGID_RESERVED);
}

static dave_bool
_thread_schedule(void)
{
	ub priority_index;
	ub thread_index;
	ThreadStruct *pThread;
	dave_bool all_message_empty;

	// If all the queue is not empty, TASK will continue to read the message.
	all_message_empty = dave_true;

	// Process the message until there is no message.
	for(priority_index=0; priority_index<THREAD_MAX; priority_index++)
	{
		if(_msg_priority[priority_index].thread_id == INVALID_THREAD_ID)
		{
			continue;
		}

		thread_index = _msg_priority[priority_index].thread_index;
		if(thread_index < THREAD_MAX)
		{
			pThread = &_thread[thread_index];

			if(pThread->thread_main != NULL)
			{
				if(_msg_priority[priority_index].thread_id != pThread->thread_id)
				{
					THREADDEBUG("sorting thread! (%d %d) %s",
						_msg_priority[priority_index].thread_id,
						pThread->thread_id,
						pThread->thread_name);

					_thread_sorting_thread();

					priority_index = 0;

					continue;
				}

				if(!(pThread->thread_flag & THREAD_THREAD_FLAG))
				{
					if(_thread_schedule_one_thread(thread_index, pThread->thread_id, pThread->thread_name, 0, dave_true) > 0)
					{
						all_message_empty = dave_false;
					}
				}
			}
		}
	}

	return all_message_empty;
}

static void
_thread_guardian_running_function(base_thread_fun thread_fun, base_thread_fun last_fun, ThreadId thread_dst, dave_bool initialization_flag)
{
	RUNFUNCTIONMSG *pRun = thread_msg(pRun);

	THREADDEBUG("%s running:%s", _thread_get_name(thread_dst), initialization_flag==dave_true?"init":"exit");

	pRun->thread_fun = (void *)thread_fun;
	pRun->last_fun = (void *)last_fun;
	pRun->thread_dst = thread_dst;
	pRun->initialization_flag = initialization_flag;

	if(_guardian_thread == INVALID_THREAD_ID)
	{
		_guardian_thread = _thread_get_id(GUARDIAN_THREAD_NAME);
	}

	write_msg(_guardian_thread, MSGID_RUN_FUNCTION, pRun);	
}

static void
_thread_setup_base_thread(ThreadStruct *pThread,
	s8 *thread_name, ub level_number, ub thread_flag, TaskAttribute attrib,
	base_thread_fun thread_init, base_thread_fun thread_main, base_thread_fun thread_exit,
	ThreadId thread_id)
{
	thread_lock();
	_thread_reset(pThread);
	pThread->thread_id = thread_id;
	dave_strcpy((s8 *)pThread->thread_name, thread_name, THREAD_NAME_MAX);
	thread_unlock();
	
	pThread->level_number = level_number;
	pThread->thread_flag = thread_flag|THREAD_MSG_WAKEUP;
	pThread->attrib = attrib;
	pThread->has_initialization = dave_false;
	pThread->thread_init = thread_init;
	pThread->thread_main = thread_main;
	pThread->thread_exit = thread_exit;

	if((thread_flag & THREAD_TRACE_FLAG) == THREAD_TRACE_FLAG)
	{
		pThread->trace_on = dave_true;
	}
	else
	{
		pThread->trace_on = dave_false;
	}
}

static void
_thread_tell_everyone_that_i_already_exist(ThreadId thread_id, s8 *thread_name)
{
#ifdef ENABLE_THREAD_REMOTE_NOTIFY
	ThreadRemoteReadyMsg *pReady = thread_reset_msg(pReady);

	pReady->remote_thread_id = thread_get_local(thread_id);
	dave_strcpy(pReady->remote_thread_name, thread_name, sizeof(pReady->remote_thread_name));

	broadcast_local(MSGID_REMOTE_THREAD_READY, pReady);
#endif
}

static void
_thread_tell_everyone_that_i_already_exit(ThreadId thread_id, s8 *thread_name)
{
#ifdef ENABLE_THREAD_REMOTE_NOTIFY
	ThreadRemoteRemoveMsg *pRemove = thread_reset_msg(pRemove);

	pRemove->remote_thread_id = thread_get_local(thread_id);
	dave_strcpy(pRemove->remote_thread_name, thread_name, sizeof(pRemove->remote_thread_name));

	broadcast_local(MSGID_REMOTE_THREAD_REMOVE, pRemove);
#endif
}

static void
_thread_delay_notify_for_system_ready_timer_out(TIMERID timer_id, ub thread_index, void *param)
{
	ub ub_thread_id = (ub)(param);
	ThreadId thread_id = (ThreadId)ub_thread_id;

	base_timer_die(timer_id);

	_thread_tell_everyone_that_i_already_exist(thread_id, _thread_get_name(thread_id));
}

static void
_thread_delay_notify_for_system_ready(ThreadId thread_id, s8 *name, ub thread_flag)
{
	ub ub_thread_id = (ub)thread_id;
	char timer_name[128];

	if((thread_flag & THREAD_REMOTE_FLAG) == THREAD_REMOTE_FLAG)
	{
		dave_sprintf((s8 *)timer_name, "delaynotify%d", thread_id);

		if(base_timer_param_creat(
			timer_name,
			_thread_delay_notify_for_system_ready_timer_out,
			(void *)ub_thread_id,
			3000) == INVALID_TIMER_ID)
		{
			_thread_tell_everyone_that_i_already_exist(thread_id, name);
		}
	}
}

static ThreadId
_thread_src_id_change(ThreadId src_id, ThreadId dst_id)
{
	/*
	 * 在一个未在Dave的线程空间
	 * 发起了消息请求，这种情况下，src_id会是
	 * 无效的线程ID INVALID_THREAD_ID,
	 * 此刻可直接让src身份由dst身份代替。
	 */
	if(src_id == INVALID_THREAD_ID)
	{
		src_id = get_self();
	}
	if(src_id == INVALID_THREAD_ID)
	{
		src_id = dst_id;
	}

	return src_id;
}

static dave_bool
_thread_sync_function_access(ThreadStruct *pSrcThread, ThreadStruct *pDstThread)
{
	if(((pSrcThread->thread_flag & THREAD_THREAD_FLAG) == 0)
		&& ((pDstThread->thread_flag & THREAD_THREAD_FLAG) == 0))
	{
		return dave_false;
	}

	return dave_true;
}

static void
_thread_wait_dst_thread_ready(ThreadId dst_id, ub msg_id)
{
	ThreadId thread_id_lock = INVALID_THREAD_ID;

	/*
	 * This detection is mainly for the unstable state of the system initialization,
	 * in the process of creating a thread, 
	 * there will be a message to be sent to the thread.
	 */
	if(_system_schedule_counter < SYSTEM_READY_COUNTER)
	{
		if((dst_id == INVALID_THREAD_ID) || (dave_strcmp(_thread_get_name(dst_id), "NULL") == dave_true))
		{
			thread_id_lock = INVALID_THREAD_ID;
		}
		else
		{
			if(_system_thread_pv_lock_name[0] != '\0')
			{
				thread_id_lock = _thread_get_id(_system_thread_pv_lock_name);
			}
		}

		if(thread_id_lock == dst_id)
		{
			dave_os_sleep(10);
		}
	}
}

static void
_thread_del_last_fun(MSGBODY *thread_msg)
{
	ThreadStruct *pThread = (ThreadStruct *)(thread_msg);
	ub thread_index;

	thread_index = thread_find_free_index(pThread->thread_name);
	if(thread_index >= THREAD_MAX)
	{
		THREADABNOR("thread:%s!", pThread->thread_name);
	}
	else
	{
		THREADDEBUG("name:%s id:%d index:%d", pThread->thread_name, pThread->thread_id, thread_index);

		thread_map_name_del(pThread->thread_name);

		_thread_remove_genealogy(thread_index);

		thread_queue_all_free(pThread);

		_thread_sorting_thread();

		_thread_reset(pThread);
	}
}

static void
_thread_creat_frist_fun(ThreadStruct *pThread)
{
	_thread_build_genealogy(pThread->thread_name, pThread->thread_index, _thread_get_id(pThread->thread_name));
	
	thread_queue_all_malloc(pThread);
	
	_thread_sorting_thread();
	
	thread_map_name_add((s8 *)(pThread->thread_name), pThread);
}

static ThreadId
_thread_creat(
	s8 *thread_name,
	ub level_number, ub thread_flag, TaskAttribute attrib,
	base_thread_fun thread_init, base_thread_fun thread_main, base_thread_fun thread_exit)
{
	ub thread_index;
	ThreadStruct *pThread;
	ThreadId thread_id;

	if(thread_main == NULL)
	{
		THREADABNOR("recreat task main function is NULL");
		return INVALID_THREAD_ID;
	}

	thread_id = _thread_get_id(thread_name);
	if(thread_id != INVALID_THREAD_ID)
	{
		// recreat task.
		THREADABNOR("recreat thread:%s", thread_name);
		return thread_id;
	}

	// thread_index is thread_id.
	thread_index = thread_id = thread_find_free_index(thread_name);
	if(thread_index >= THREAD_MAX)
	{
		THREADABNOR("creat thread:%s fail, thread list table is full!", thread_name);
		thread_id = INVALID_THREAD_ID;
	}
	else
	{
		pThread = &_thread[thread_index];

		_thread_setup_base_thread(pThread,
			thread_name, level_number, thread_flag, attrib,
			thread_init, thread_main, thread_exit,
			thread_id);

		_thread_creat_frist_fun(pThread);

		if(pThread->thread_init != NULL)
		{
			if(dave_strcmp(thread_name, GUARDIAN_THREAD_NAME) == dave_true)
			{
				pThread->has_initialization = dave_true;
			}

			_thread_guardian_running_function(pThread->thread_init, NULL, pThread->thread_id, dave_true);
		}
		else
		{
			pThread->has_initialization = dave_true;
		}

		THREADDEBUG("name:%s id:%d index:%d", thread_name, thread_id, thread_index);
	}

	return thread_id;
}

static dave_bool
_thread_del(ThreadId thread_id)
{
	ub thread_index;
	ThreadStruct *pThread;

	if(thread_id == INVALID_THREAD_ID)
	{
		return dave_false;
	}
	
	thread_index = thread_find_busy_index(thread_id);
	if(thread_index >= THREAD_MAX)
	{
		THREADDEBUG("");
		return dave_false;
	}

	pThread = &_thread[thread_index];

	if(pThread->thread_exit != NULL)
	{
		_thread_guardian_running_function(pThread->thread_exit, _thread_del_last_fun, pThread->thread_id, dave_false);
	}
	else
	{
		_thread_del_last_fun((MSGBODY *)pThread);
	}

	return dave_true;
}

static ThreadId
_thread_safe_creat(s8 *name, ub level_number, ub thread_flag, base_thread_fun thread_init, base_thread_fun thread_main, base_thread_fun thread_exit)
{
	ThreadId thread_id;

	if(level_number > 255)
	{
		level_number = 255;
	}

	if((thread_flag & THREAD_REMOTE_FLAG) == THREAD_REMOTE_FLAG)
		thread_id  = _thread_creat(name, level_number, thread_flag, REMOTE_TASK_ATTRIB, thread_init, thread_main, thread_exit);
	else
		thread_id = _thread_creat(name, level_number, thread_flag, LOCAL_TASK_ATTRIB, thread_init, thread_main, thread_exit);

	if(thread_id != INVALID_THREAD_ID)
	{
		_thread_delay_notify_for_system_ready(thread_id, name, thread_flag);
	}
	else
	{
		base_power_off("core showdown!");
	}

	thread_id = thread_get_local(thread_id);

	return thread_id;
}

static dave_bool
_thread_safe_del(ThreadId thread_id)
{
	dave_bool ret;
	TaskAttribute attrib;

	thread_id = thread_get_local(thread_id);

	attrib = _thread_attrib(thread_id);

	ret = _thread_del(thread_id);

	if(ret == dave_true)
	{
		if(attrib == REMOTE_TASK_ATTRIB)
		{
			_thread_tell_everyone_that_i_already_exit(thread_id, _thread_get_name(thread_id));
		}
	}

	return ret;
}

static inline dave_bool
_thread_safe_id_msg(
	ThreadId src_id, ThreadId dst_id, BaseMsgType msg_type,
	ub msg_id, ub msg_len, u8 *msg_body,
	s8 *fun, ub line)
{
	ub thread_index;
	ThreadId local_dst_id, route_dst_id;
	ErrCode ret;

	if(thread_is_remote(dst_id) == dave_true)
	{
		if(thread_remote_id_table_inq(dst_id) == dave_false)
		{
			THREADLTRACE(60, 1, "The remote message ID(%lx/%s->%lx/%s:%d) has expired!",
				src_id, _thread_get_name(src_id),
				dst_id, _thread_get_name(dst_id),
				msg_id);
			return dave_false;
		}

		route_dst_id = dst_id;
	}
	else
	{
		route_dst_id = INVALID_THREAD_ID;
	}

	local_dst_id = thread_get_local(dst_id);

	if(local_dst_id == INVALID_THREAD_ID)
	{
		if(base_power_state() == dave_true)
		{
			THREADLTRACE(60,1,"Parameter error, dst_id:%d msg_id:%d msg_len:%d (%s:%d)",
				local_dst_id, msg_id, msg_len, fun, line);
		}
		return dave_false;
	}

	if((msg_len >= THREAD_MSG_MAX_LEN) || (msg_len == 0))
	{
		THREADLOG("send msg<%d> to %s, the length is invalid(%d/%d)!",
			msg_id, _thread_get_name(local_dst_id), msg_len, THREAD_MSG_MAX_LEN);
		return dave_false;
	}

	thread_index =  thread_find_busy_index(local_dst_id);
	if(thread_index >= THREAD_MAX)
	{
		THREADLTRACE(60,1,"Can not find thread, dst_id:%lx msg_id:%d (%s:%d)",
			local_dst_id, msg_id, fun, line);
		return dave_false;
	}

	src_id = _thread_src_id_change(src_id, local_dst_id);

	ret = _thread_write_msg(
		src_id, dst_id,
		thread_index,
		msg_id, msg_len, msg_body,
		dave_true, msg_type,
		route_dst_id,
		fun, line);

	if(ret != ERRCODE_OK)
	{
		THREADLTRACE(60,1,"%s->%s:%d ret:%s <%s:%d>",
			_thread_get_name(src_id), _thread_get_name(dst_id), msg_id,
			errorstr(ret),
			fun, line);

		return dave_false;
	}

	return dave_true;
}

static void
_thread_init(void)
{
	_thread_reset_all();

	_thread_clean_priority();

	_current_msg_stack = &_top_msg_stack;

	_top_msg_stack.next_stack = NULL;
	_top_msg_stack.thread_id = INVALID_THREAD_ID;
}

static void
_thread_exit(void)
{
	ub thread_index;
	ThreadStruct *pThread;
	MSGBODY msg;

	for(thread_index=0; thread_index<THREAD_MAX; thread_index++)
	{
		pThread = &_thread[thread_index];

		if(pThread->thread_exit != NULL)
		{
			msg.msg_src = pThread->thread_id;
			msg.msg_dst = pThread->thread_id;
			msg.msg_id = MSGID_POWER_OFF;
			msg.msg_len = 0;
			msg.msg_body = NULL;

			_thread_run_call_back_user_fun(
				pThread->thread_exit, NULL,
				pThread,
				&msg, dave_true);
		}

		thread_queue_all_free(pThread);

		_thread_reset(pThread);
	}
}

static dave_bool
_thread_check_sync_param(
	ThreadStruct **ppSrcThread, ThreadStruct **ppDstThread,
	ThreadId *src_id, ThreadId *dst_id,
	ub msg_id, ub msg_len, u8 *msg_body,
	ub sync_id, ub sync_len, u8 *sync_body,
	s8 *fun, ub line)
{
	if((*src_id == *dst_id) || (self() == *dst_id))
	{
		THREADABNOR("Do not send synchronization messages inside a thread! <%d/%s %d/%s %d/%s> <%s:%d>",
			*src_id, _thread_get_name(*src_id),
			*dst_id, _thread_get_name(*dst_id),
			self(), _thread_get_name(self()),
			fun, line);
		return dave_false;
	}

	*src_id = _thread_src_id_change(*src_id, *dst_id);

	if(*src_id != INVALID_THREAD_ID)
	{
		*ppSrcThread = thread_find_busy_thread(*src_id);
	}

	if(*ppSrcThread == NULL)
	{
		THREADABNOR("Synchronization messages without attribution are not supported!");
		return dave_false;
	}

	*ppDstThread = thread_find_busy_thread(*dst_id);

	if(*ppDstThread == NULL)
	{
		THREADABNOR("pDstThread is NULL! msg_id:%d sync_id:%d <%s:%d>", msg_id, sync_id, fun, line);
		return dave_false;
	}

	if(_thread_sync_function_access(*ppSrcThread, *ppDstThread) == dave_false)
	{
		THREADABNOR("Synchronization only happens between two separate threads! msg_id:%d <%s:%d>",
			msg_id, fun, line);
		return dave_false;
	}

	thread_check_pair_msg(msg_id, sync_id);

	return dave_true;
}

static void
_thread_local_ready(char *thread_name)
{
	ub thread_index;
	ThreadId thread_id;
	ThreadStruct *pThread;
	ThreadLocalReadyMsg *pReady;

	thread_index = thread_find_busy_index(thread_id(thread_name));
	if(thread_index >= THREAD_MAX)
	{
		THREADLOG("can't find thread:%s", thread_name);
	}
	else
	{
		pThread = &_thread[thread_index];
		thread_id = pThread->thread_id;
		if(pThread->attrib == LOCAL_TASK_ATTRIB)
		{
			pReady = thread_msg(pReady);

			pReady->local_thread_id = thread_id;
			dave_strcpy(pReady->local_thread_name, thread_name, sizeof(pReady->local_thread_name));

			broadcast_local(MSGID_LOCAL_THREAD_READY, pReady);
		}

		for(thread_index=0; thread_index<THREAD_MAX; thread_index++)
		{
			pThread = &_thread[thread_index];
			if((pThread->thread_id != INVALID_THREAD_ID)
				&& (pThread->thread_id != thread_id)
				&& (pThread->attrib == LOCAL_TASK_ATTRIB))
			{
				pReady = thread_msg(pReady);

				pReady->local_thread_id = thread_id;
				dave_strcpy(pReady->local_thread_name, thread_name, sizeof(pReady->local_thread_name));

				write_msg(pThread->thread_id, MSGID_LOCAL_THREAD_READY, pReady);	
			}
		}
	}
}

static void
_thread_local_remove(ThreadId thread_id)
{
	ub thread_index;
	ThreadStruct *pThread;
	ThreadLocalRemoveMsg *pRemove;

	thread_index = thread_find_busy_index(thread_id);
	if(thread_index >= THREAD_MAX)
	{
		THREADLOG("can't find thread:%s", thread_name(thread_id));
	}
	else
	{
		pThread = &_thread[thread_index];
		if(pThread->attrib == LOCAL_TASK_ATTRIB)
		{
			pRemove = thread_msg(pRemove);
	
			pRemove->local_thread_id = pThread->thread_id;
			dave_strcpy(pRemove->local_thread_name, pThread->thread_name, sizeof(pRemove->local_thread_name));
	
			broadcast_local(MSGID_LOCAL_THREAD_REMOVE, pRemove);
		}
	}
}

// =====================================================================

void
base_thread_init(void *main_thread_id)
{
	__system_startup__ = dave_true;

	_main_thread_id = main_thread_id;

	_system_wakeup_counter = 0;

	t_lock_reset(&_system_thread_pv);

	_system_schedule_counter = 0;

	dave_memset(_thread, 0x00, sizeof(_thread));

	thread_memory_init();

	thread_tools_init(_thread);

	_thread_init();

	thread_call_init();

	thread_quit_init(main_thread_id);

	thread_thread_init(_thread_schedule_one_thread);

	_top_msg_stack.thread_id = _guardian_thread = thread_guardian_init(_thread);
}

void
base_thread_exit(void)
{
	thread_guardian_exit();

	thread_thread_exit();

	thread_quit_exit();

	thread_call_exit();

	_thread_exit();

	thread_tools_exit();

	thread_memory_exit();
}

void
base_thread_schedule(void)
{
	ub safe_counter, while_max;
	dave_bool all_message_empty;

	SAFEZONEv5R(_system_thread_pv, { _thread_schedule_predecessor_task(); } );

	safe_counter = 0;

	while_max = THREAD_MAX * 8;

	all_message_empty = dave_false;

	while(((++ safe_counter) < while_max) && (all_message_empty == dave_false))
	{
		all_message_empty = _thread_schedule();
	}

	if(base_power_state() == dave_true)
	{
		_thread_sleep();
	}
}

ThreadId
base_thread_creat(char *name, ub level_number, ub thread_flag, base_thread_fun thread_init, base_thread_fun thread_main, base_thread_fun thread_exit)
{
	ThreadId thread_id = INVALID_THREAD_ID;

	SAFEZONEv5W(_system_thread_pv, {
		dave_strcpy(_system_thread_pv_lock_name, name, THREAD_NAME_MAX);

		thread_id = _thread_safe_creat((s8 *)name, level_number, thread_flag, thread_init, thread_main, thread_exit);
	} );

	_thread_local_ready(name);

	return thread_id;
}

dave_bool
base_thread_del(ThreadId thread_id)
{
	dave_bool ret = dave_false;

	_thread_local_remove(thread_id);

	SAFEZONEv5W(_system_thread_pv, {
		dave_memset(_system_thread_pv_lock_name, 0x00, THREAD_NAME_MAX);

		ret = _thread_safe_del(thread_id);
	} );

	return ret;
}

ThreadId
base_thread_get_local(ThreadId thread_id)
{
	return thread_get_local(thread_id);
}

ThreadId
base_thread_get_id(const s8 *name, s8 *fun, ub line)
{
	ThreadId thread_id = INVALID_THREAD_ID;

	if((name != NULL) && (name[0] != '\0'))
	{
		if(_system_schedule_counter < SYSTEM_READY_COUNTER)
		{
			SAFEZONEv5R(_system_thread_pv, {

				thread_id = _thread_get_id_(name, fun, line);

			} );
		}
		else
		{
			thread_id = _thread_get_id_(name, fun, line);
		}
	}

	return thread_id;
}

TaskAttribute
base_thread_attrib(ThreadId thread_id)
{
	return _thread_attrib(thread_id);
}

s8 *
base_thread_get_name(ThreadId thread_id, s8 *fun, ub line)
{
	s8 *name = NULL;

	if(_system_schedule_counter < SYSTEM_READY_COUNTER)
	{
		SAFEZONEv5R(_system_thread_pv, {

			name = _thread_get_name_(thread_id, fun, line);

		} );
	}
	else
	{
		name = _thread_get_name_(thread_id, fun, line);
	}

	return name;
}

ThreadId
base_thread_get_self(s8 *fun, ub line)
{
	ThreadId self;

	if(__system_startup__ == dave_true)
	{
		if(thread_thread_is_main() == dave_true)
		{
			thread_lock();

			if(_current_msg_stack != NULL)
			{
				self = _current_msg_stack->thread_id;
			}
			else
			{
				self = _guardian_thread;
			}

			thread_unlock();
		}
		else
		{
			self = thread_thread_self(NULL, NULL);

			if(self == INVALID_THREAD_ID)
			{
				dave_os_thread_self(&self);
			}
		}
	}
	else
	{
		self = INVALID_THREAD_ID;
	}

	return thread_get_local(self);
}

ub
base_thread_name_array(s8 thread_name[][64], ub thread_number)
{
	ub thread_index, name_index, name_len;

	for(thread_index=0,name_index=0; (thread_index<THREAD_MAX)&&(name_index<thread_number); thread_index++)
	{
		if((_thread[thread_index].thread_id != INVALID_THREAD_ID)
			&& (_thread[thread_index].attrib == LOCAL_TASK_ATTRIB))
		{
			name_len = dave_strlen(_thread[thread_index].thread_name);
			if(name_len > (64 - 1))
			{
				THREADABNOR("thread:%s name is too longer!<%d>", _thread[thread_index].thread_name, name_len);
			}
			else
			{
				if(thread_must_in_local((s8 *)(_thread[thread_index].thread_name)) == dave_false)
				{
					dave_strcpy(&thread_name[name_index ++][0], (const s8 *)(_thread[thread_index].thread_name), 64);
				}
			}
		}
	}

	return name_index;
}

dave_bool
__base_thread_trace_state__(s8 *fun, ub line)
{
	ThreadId my_thread = self();
	ub thread_index;

	if(my_thread == INVALID_THREAD_ID)
	{
		return dave_false;
	}

	thread_index = thread_find_busy_index(my_thread);
	if(thread_index >= THREAD_MAX)
	{
		THREADABNOR("%s<%d> can't find thread_index <%s:%d>",
			_thread_get_name(my_thread), my_thread,
			fun, line);
		return dave_false;
	}

	return _thread[thread_index].trace_on;
}

ErrCode
base_thread_msg_register(ThreadId src_id, ub msg_id, thread_msg_fun msg_fun, void *user_ptr)
{
	if(src_id == INVALID_THREAD_ID)
	{
		src_id = self();
	}

	if(thread_call_msg_register(src_id, msg_id, msg_fun, user_ptr) == dave_true)
	{
		return ERRCODE_OK;
	}
	else
	{
		return ERRCODE_invalid_option;
	}
}

void
base_thread_msg_unregister(ub msg_id)
{
	thread_call_msg_unregister(self(), msg_id);
}

ub
base_thread_info(s8 *msg, ub msg_len)
{
	return thread_show_all_info(_thread, NULL, msg, msg_len, dave_true);
}

void *
base_thread_msg(ub msg_len, dave_bool reset, s8 *fun, ub line)
{
	void *ptr;

	ptr = thread_malloc(msg_len, MSGID_RESERVED, fun, line);

	if((reset == dave_true) && (ptr != NULL))
	{
		dave_memset(ptr, 0x00, msg_len);
	}

	return ptr;
}

void
base_thread_msg_release(void *ptr, s8 *fun, ub line)
{
	thread_free(ptr, MSGID_RESERVED, fun, line);
}

dave_bool
base_thread_id_msg(
	ThreadId src_id, ThreadId dst_id,
	BaseMsgType msg_type,
	ub msg_id, ub msg_len, u8 *msg_body,
	ub msg_number,
	s8 *fun, ub line)
{
	dave_bool ret;

	if(msg_id == MSGID_RESERVED)
	{
		THREADLOG("invalid msg_id! <%s:%d>",
			fun, line);
		ret  = dave_false;
		thread_clean_user_input_data(msg_body, msg_id);
	}
	else
	{
		if(src_id == INVALID_THREAD_ID)
		{
			src_id = get_self();
		}

		if((msg_number > 0)
			&& (dst_id != INVALID_THREAD_ID)
			&& (_thread_num_msg(thread_find_busy_thread(dst_id), msg_id) > msg_number))
		{
			ret  = dave_true;
			thread_clean_user_input_data(msg_body, msg_id);			
		}
		else
		{
			_thread_wait_dst_thread_ready(dst_id, msg_id);

			ret  = _thread_safe_id_msg(src_id, dst_id, msg_type, msg_id, msg_len, msg_body, fun, line);
		}
	}

	return ret;
}

dave_bool
base_thread_id_event(
	ThreadId src_id, ThreadId dst_id,
	BaseMsgType msg_type,
	ub req_id, ub msg_len, u8 *msg_body,
	ub rsp_id, thread_msg_fun rsp_fun,
	s8 *fun, ub line)
{
	thread_check_pair_msg(req_id, rsp_id);

	if(base_thread_msg_register(src_id, rsp_id, rsp_fun, NULL) == ERRCODE_OK)
	{
		return base_thread_id_msg(src_id, dst_id, msg_type, req_id, msg_len, msg_body, 0, fun, line);
	}
	else
	{
		thread_clean_user_input_data(msg_body, req_id);
	}

	return dave_false;
}

dave_bool
base_thread_name_msg(
	s8 *thread_name,
	ub msg_id, ub msg_len, u8 *msg_body,
	s8 *fun, ub line)
{
	ThreadId src_id, dst_id;
	dave_bool ret;

	if((thread_name == NULL) || (thread_name[0] == '\0'))
	{
		thread_clean_user_input_data(msg_body, msg_id);
		dst_id = INVALID_THREAD_ID;
		ret = dave_false;
	}
	else
	{
		src_id = self();
		dst_id = _thread_get_id(thread_name);

		if(dst_id != INVALID_THREAD_ID)
		{
			ret = base_thread_id_msg(src_id, dst_id, BaseMsgType_Unicast, msg_id, msg_len, msg_body, 0, fun, line);
		}
		else
		{
			ret = thread_msg_buffer_push(src_id, thread_name, BaseMsgType_Unicast, msg_id, msg_len, msg_body, fun, line);
		}
	}

	if(ret == dave_false)
	{
		THREADLTRACE(60,1,"%lx/%s->%lx/%s:%d <%s:%d>",
			self(), _thread_get_name(self()), dst_id, thread_name, msg_id,
			fun, line);
	}

	return ret;
}

dave_bool
base_thread_name_event(
	s8 *thread_name,
	ub req_id, ub msg_len, u8 *msg_body,
	ub rsp_id, thread_msg_fun rsp_fun,
	s8 *fun, ub line)
{
	thread_check_pair_msg(req_id, rsp_id);

	if(base_thread_msg_register(INVALID_THREAD_ID, rsp_id, rsp_fun, NULL) == ERRCODE_OK)
	{
		if(base_thread_name_msg(thread_name, req_id, msg_len, msg_body, fun, line) == dave_true)
		{
			return dave_true;
		}
	}

	return dave_false;
}

dave_bool
base_thread_gid_msg(
	s8 *gid, s8 *thread_name,
	ub msg_id, ub msg_len, u8 *msg_body,
	s8 *fun, ub line)
{
	ThreadId thread_id = thread_gid_table_inq(gid, thread_name);

	if(thread_id == INVALID_THREAD_ID)
	{
		thread_clean_user_input_data(msg_body, msg_id);
		return dave_false;
	}

	return base_thread_id_msg(INVALID_THREAD_ID, thread_id, BaseMsgType_Unicast, msg_id, msg_len, msg_body, 0, fun, line);
}

dave_bool
base_thread_gid_event(
	s8 *gid, s8 *thread_name,
	ub req_id, ub msg_len, u8 *msg_body,
	ub rsp_id, thread_msg_fun rsp_fun,
	s8 *fun, ub line)
{
	ThreadId thread_id = thread_gid_table_inq(gid, thread_name);

	if(thread_id == INVALID_THREAD_ID)
	{
		thread_clean_user_input_data(msg_body, req_id);
		return dave_false;
	}

	return base_thread_id_event(INVALID_THREAD_ID, thread_id, BaseMsgType_Unicast, req_id, msg_len, msg_body, rsp_id, rsp_fun, fun, line);
}

void *
base_thread_sync_msg(
	ThreadId src_id, ThreadId dst_id,
	ub msg_id, ub msg_len, u8 *msg_body,
	ub sync_id, ub sync_len, u8 *sync_body,
	s8 *fun, ub line)
{
	ThreadStruct *pSrcThread = NULL, *pDstThread = NULL;
	ThreadSync *pSync;
	void *ret_body = NULL;

	if(_thread_check_sync_param(
		&pSrcThread, &pDstThread,
		&src_id, &dst_id,
		msg_id, msg_len, msg_body,
		sync_id, sync_len, sync_body,
		fun, line) == dave_false)
	{
		thread_clean_user_input_data(msg_body, msg_id);
		return NULL;
	}

	pSync = thread_call_sync_pre(pSrcThread, &src_id, pDstThread, sync_id, sync_body, sync_len);
	if(pSync == NULL)
	{
		THREADLTRACE(60,1,"sync thread call failed! %s->%s %d <%s:%d>",
			pSrcThread->thread_name, pDstThread->thread_name, msg_id,
			fun, line);
		thread_clean_user_input_data(msg_body, msg_id);
		return NULL;
	}

	if(base_thread_id_msg(src_id, dst_id, BaseMsgType_Unicast, msg_id, msg_len, msg_body, 0, fun, line) == dave_false)
	{
		sync_body = NULL;
	}

	if(sync_body != NULL)
	{
		ret_body = thread_call_sync_wait(pSrcThread, pDstThread, pSync);
	}

	if(ret_body == NULL)
	{
		THREADLTRACE(60,1,"thread:%s msg_id:%d sync failed! <%s:%d>",
			pSrcThread->thread_name, msg_id,
			fun, line);
	}

	return ret_body;
}

dave_bool
base_thread_broadcast_msg(BaseMsgType type, s8 *dst_name, ub msg_id, ub msg_len, u8 *msg_body, s8 *fun, ub line)
{
	return thread_broadcast_msg(_thread, type, dst_name, msg_id, msg_len, msg_body, fun, line);
}

#endif
