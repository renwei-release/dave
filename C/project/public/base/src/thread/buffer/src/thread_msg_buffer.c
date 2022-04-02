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
#include "thread_struct.h"
#include "thread_quit.h"
#include "thread_tools.h"
#include "thread_mem.h"
#include "thread_log.h"

#ifdef PLATFORM_64_BIT
#define base_mask (0xfffffffffffffff8)
#else
#define base_mask (0xfffffffc)
#endif

#define MSG_BUFFER_MAX (20480)
#define MSG_IN_BUFFER_MAX_TIME (1000 * 1000 * 720)	// 720 s
#define WAIT_PUSH_BUFFER_TIME (1000 * 1000 * 1) 	// 1 s
#define MAX_ACTION_THREAD (THREAD_MAX)

typedef struct {
	dave_bool in_use;
	ub tick;
	s8 src_thread_name[THREAD_NAME_MAX];
	s8 dst_thread_name[THREAD_NAME_MAX];
	BaseMsgType msg_type;
	ub msg_id;
	ub msg_len;
	u8 *msg_body;
	s8 *fun;
	ub line;
} MsgBuffer;

static MsgBuffer _msg_buffer[MSG_BUFFER_MAX];
static volatile ub _msg_buffer_w_index;
static volatile sb _msg_buffer_number;
static s8 _msg_action_thread[MAX_ACTION_THREAD][THREAD_NAME_MAX];
static volatile ub _msg_action_thread_number = 0;
static volatile ub _msg_action_wait_times = 0;
static TLock _msg_buffer_pv;

static void
_thread_msg_buffer_reset(MsgBuffer *pMsg)
{
	dave_memset(pMsg, 0x00, sizeof(MsgBuffer));

	pMsg->in_use = dave_false;
	pMsg->msg_body = NULL;
}

static void
_thread_msg_buffer_all_reset(void)
{
	ub msg_index;

	for(msg_index=0; msg_index<MSG_BUFFER_MAX; msg_index++)
	{
		_thread_msg_buffer_reset(&_msg_buffer[msg_index]);
	}
}

static MsgBuffer *
_thread_msg_buffer_malloc(s8 *src_thread_name, s8 *dst_thread_name, BaseMsgType msg_type, ub msg_id, ub msg_len, u8 *msg_body, s8 *fun, ub line)
{
	ub safe_counter;
	MsgBuffer *pMsg;

	for(safe_counter=0; safe_counter<MSG_BUFFER_MAX; safe_counter++)
	{
		pMsg = &_msg_buffer[(_msg_buffer_w_index ++) % MSG_BUFFER_MAX];

		if(pMsg->in_use == dave_false)
		{
			pMsg->tick = dave_os_time_us();

			dave_strcpy(pMsg->src_thread_name, src_thread_name, THREAD_NAME_MAX);
			dave_strcpy(pMsg->dst_thread_name, dst_thread_name, THREAD_NAME_MAX);
			pMsg->msg_type = msg_type;
			pMsg->msg_id = msg_id;
			pMsg->msg_len = msg_len;
			pMsg->msg_body = base_thread_msg(pMsg->msg_len, dave_false, fun, line);
			dave_memcpy(pMsg->msg_body, msg_body, pMsg->msg_len);

			pMsg->fun = fun;
			pMsg->line = line;

			pMsg->in_use = dave_true;

			_msg_buffer_number ++;

			return pMsg;
		}
	}

	THREADLTRACE(30,1,"msg buffer is full! %s->%s %d <%s:%d>",
		src_thread_name, dst_thread_name, msg_id, fun, line);

	return NULL;
}

static void
_thread_msg_buffer_free(MsgBuffer *pMsg)
{
	if(pMsg->msg_body != NULL)
	{
		thread_msg_release(pMsg->msg_body);
		pMsg->msg_body = NULL;
	}

	_thread_msg_buffer_reset(pMsg);

	if(_msg_buffer_number > 0)
	{
		_msg_buffer_number --;
	}
	else
	{
		THREADABNOR("Arithmetic error on remote msg buffer number!");
	}
}

static dave_bool
_thread_msg_safe_buffer(s8 *src_thread_name, s8 *dst_thread_name, BaseMsgType msg_type, ub msg_id, ub msg_len, u8 *msg_body, s8 *fun, ub line)
{
	MsgBuffer *pMsg = NULL;

	SAFEZONEv3(_msg_buffer_pv, {
		pMsg = _thread_msg_buffer_malloc(src_thread_name, dst_thread_name, msg_type, msg_id, msg_len, msg_body, fun, line);
	} );

	if(pMsg == NULL)
	{
		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

static void
_thread_msg_clean_buffer(s8 *thread_name)
{
	ub msg_index;
	ThreadId src_id, dst_id;
	dave_bool process_ret;

	THREADDEBUG("thread:%s", thread_name);

	for(msg_index=0; msg_index<MSG_BUFFER_MAX; msg_index++)
	{
		if(_msg_buffer[msg_index].in_use == dave_true)
		{
			THREADDEBUG("%s->%s msg_id:%d",
				_msg_buffer[msg_index].src_thread_name,
				_msg_buffer[msg_index].dst_thread_name,
				_msg_buffer[msg_index].msg_id);

			if(dave_strcmp(_msg_buffer[msg_index].dst_thread_name, thread_name) == dave_true)
			{
				process_ret = dave_false;

				src_id = thread_id(_msg_buffer[msg_index].src_thread_name);
				dst_id = thread_id(_msg_buffer[msg_index].dst_thread_name);

				if((src_id != INVALID_THREAD_ID) && (dst_id != INVALID_THREAD_ID))
				{
					if(thread_attrib(dst_id) == REMOTE_TASK_ATTRIB)
					{
						if(base_thread_local_msg(src_id, dst_id,
							_msg_buffer[msg_index].msg_type,
							_msg_buffer[msg_index].msg_id, _msg_buffer[msg_index].msg_len, _msg_buffer[msg_index].msg_body,
							0,
							_msg_buffer[msg_index].fun, _msg_buffer[msg_index].line) == dave_true)
						{
							_msg_buffer[msg_index].msg_body = NULL;
						
							_thread_msg_buffer_free(&_msg_buffer[msg_index]);

							process_ret = dave_true;
						}
					}
					else
					{
						_thread_msg_buffer_free(&_msg_buffer[msg_index]);
					}
				}

				if(process_ret == dave_false)
				{
					THREADABNOR("timer out! %s->%s msg_id:%d",
						_msg_buffer[msg_index].src_thread_name,
						_msg_buffer[msg_index].dst_thread_name,
						_msg_buffer[msg_index].msg_id);
				}
			}
		}
	}
}

static void
_thread_msg_safe_action(void)
{
	ub local_time;

	SAFEZONEv3(_msg_buffer_pv, {
		if((_msg_action_thread_number > 0) && (_msg_action_thread_number <= MAX_ACTION_THREAD))
		{
			local_time = dave_os_time_us();

			if((_msg_action_wait_times > local_time)
				|| ((local_time - _msg_action_wait_times) > WAIT_PUSH_BUFFER_TIME))
			{
				_thread_msg_clean_buffer(_msg_action_thread[-- _msg_action_thread_number]);
			}
		}
	} );
}

static void
_thread_msg_buffer_recovery(void)
{
	ub current_tick;
	ub msg_index;
	dave_bool recovery;
	RemoteMsgTimerOutMsg *pMsg;

	SAFEZONEv3(_msg_buffer_pv, {
		current_tick = dave_os_time_us();

		for(msg_index=0; msg_index<MSG_BUFFER_MAX; msg_index++)
		{
			if(_msg_buffer[msg_index].in_use == dave_true)
			{
				recovery = dave_false;

				if(current_tick > _msg_buffer[msg_index].tick)
				{
					if((current_tick - _msg_buffer[msg_index].tick) >= MSG_IN_BUFFER_MAX_TIME)
					{
						recovery = dave_true;
					}
				}
				else
				{
					recovery = dave_true;
				}

				if(recovery == dave_true)
				{
					pMsg = base_thread_msg(sizeof(RemoteMsgTimerOutMsg)+_msg_buffer[msg_index].msg_len+64, dave_false, (s8 *)__func__, (ub)__LINE__);

					pMsg->msg_id = _msg_buffer[msg_index].msg_id;
					pMsg->msg_len = _msg_buffer[msg_index].msg_len;
					pMsg->msg_body = (void *)(((ub)pMsg + sizeof(RemoteMsgTimerOutMsg) + 16) & base_mask);
					dave_memcpy(pMsg->msg_body, _msg_buffer[msg_index].msg_body, pMsg->msg_len);

					if(base_thread_local_msg(
						thread_id(_msg_buffer[msg_index].dst_thread_name),
						thread_id(_msg_buffer[msg_index].src_thread_name),
						_msg_buffer[msg_index].msg_type,
						MSGID_REMOTE_MSG_TIMER_OUT, sizeof(RemoteMsgTimerOutMsg), (void *)pMsg,
						0,
						_msg_buffer[msg_index].fun, _msg_buffer[msg_index].line) == dave_true)
					{
						_thread_msg_buffer_free(&_msg_buffer[msg_index]);
					}
				}
			}
		}
	} );	
}

static void
_thread_msg_buffer_clear(void)
{
	ub msg_index;

	for(msg_index=0; msg_index<MSG_BUFFER_MAX; msg_index++)
	{
		if(_msg_buffer[msg_index].msg_body != NULL)
		{
			_thread_msg_buffer_free(&_msg_buffer[msg_index]);
		}
	}
}

// =====================================================================

void
thread_msg_buffer_init(void)
{
	_thread_msg_buffer_all_reset();

	_msg_buffer_w_index = 0;

	_msg_buffer_number = 0;

	dave_memset(_msg_action_thread, 0x00, MAX_ACTION_THREAD * THREAD_NAME_MAX);

	_msg_action_thread_number = 0;

	t_lock_reset(&_msg_buffer_pv);
}

void
thread_msg_buffer_exit(void)
{
	_thread_msg_buffer_clear();
}

dave_bool
thread_msg_buffer(ThreadId src_id, s8 *dst_thread_name, BaseMsgType msg_type, ub msg_id, ub msg_len, u8 *msg_body, s8 *fun, ub line)
{
	s8 *src_thread_name;
	dave_bool ret = dave_true;

	src_thread_name = NULL;

	if(src_id == INVALID_THREAD_ID)
	{
		src_thread_name = thread_name(get_self());
	}
	else
	{
		if(thread_attrib(src_id) == LOCAL_TASK_ATTRIB)
		{
			src_thread_name = thread_name(src_id);
		}
		else
		{
			src_thread_name = NULL;
		}
	}

	if(src_thread_name == NULL)
	{
		THREADABNOR("src_thread_name is NULL! dst:%s msg:%d <%s:%d>",
			dst_thread_name, msg_id, fun, line);
		ret = dave_false;
	}
	else if(dave_strcmp(src_thread_name, (s8 *)"NULL") == dave_true)
	{
		THREADABNOR("src_thread_name is NULL! dst:%s msg:%d <%s:%d>",
			dst_thread_name, msg_id, fun, line);
		ret = dave_false;
	}

	THREADDEBUG("%s->%s %d", src_thread_name, dst_thread_name, msg_id);

	if(ret == dave_true)
	{
		ret = _thread_msg_safe_buffer(src_thread_name, dst_thread_name, msg_type, msg_id, msg_len, msg_body, fun, line);
	}

	thread_clean_user_input_data(msg_body, msg_id);

	return ret;
}

void
thread_msg_buffer_action(s8 *thread_name)
{
	/* Delay the message so that the system has the appropriate time to initialize. */

	SAFEZONEv3(_msg_buffer_pv, {
		if(_msg_action_thread_number >= MAX_ACTION_THREAD)
		{
			THREADABNOR("action thread:%s overflow!", thread_name);
		}
		else
		{
			dave_strcpy(_msg_action_thread[_msg_action_thread_number ++], thread_name, THREAD_NAME_MAX);
			_msg_action_wait_times = dave_os_time_us();
		}	
	} );

}

void
thread_msg_buffer_tick(void)
{
	if(_msg_buffer_number > 0)
	{
		_thread_msg_buffer_recovery();
	}

	if(_msg_action_thread_number > 0)
	{
		_thread_msg_safe_action();
	}
}

#endif

