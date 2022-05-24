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

#define THREAD_MSG_BUFFER_LIFE_MAX 60

typedef struct {
	s8 src_thread[THREAD_NAME_MAX];
	s8 dst_thread[THREAD_NAME_MAX];
	BaseMsgType msg_type;
	ub msg_id;
	ub msg_len;
	u8 *msg_body;
	s8 *fun;
	ub line;

	sb life;
	void *next;
} MsgBuffer;

typedef struct {
	MsgBuffer *head;
	MsgBuffer *tail;

	sb life;
} MsgList;

static ub __init_flag__ = 0x00;
static TLock _thread_msg_buf_pv;
static void *_thread_msg_buf_ramkv = NULL;

static MsgBuffer *
_thread_msg_buffer_malloc(ThreadId src_id, s8 *dst_thread, BaseMsgType msg_type, ub msg_id, ub msg_len, u8 *msg_body, s8 *fun, ub line)
{
	s8 *src_thread;
	MsgBuffer *pBuffer;

	if(src_id == INVALID_THREAD_ID)
		src_id = self();
	src_thread = thread_name(src_id);

	pBuffer = dave_malloc(sizeof(MsgBuffer));

	dave_strcpy(pBuffer->src_thread, src_thread, sizeof(pBuffer->src_thread));
	dave_strcpy(pBuffer->dst_thread, dst_thread, sizeof(pBuffer->dst_thread));
	pBuffer->msg_type = msg_type;
	pBuffer->msg_id = msg_id;
	pBuffer->msg_len = msg_len;
	pBuffer->msg_body = base_thread_msg(msg_len, dave_false, fun, line);;
	dave_memcpy(pBuffer->msg_body, msg_body, msg_len);
	pBuffer->fun = fun;
	pBuffer->line = line;

	pBuffer->life = 0;
	pBuffer->next = NULL;

	return pBuffer;
}

static void
_thread_msg_buffer_free(MsgBuffer *pBuffer)
{
	dave_free(pBuffer);
}

static MsgList *
_thread_msg_buffer_list_malloc(void *ramkv, s8 *dst_thread)
{
	MsgList *pMsgList = dave_ralloc(sizeof(MsgList));

	base_ramkv_add_key_ptr(ramkv, dst_thread, pMsgList);

	pMsgList->head = pMsgList->tail = NULL;
	pMsgList->life = THREAD_MSG_BUFFER_LIFE_MAX;

	THREADDEBUG("%s", dst_thread);

	return pMsgList;
}

static RetCode
_thread_msg_buffer_list_free(void *ramkv, s8 *dst_thread)
{
	MsgList *pMsgList = (MsgList *)base_ramkv_del_key_ptr(ramkv, dst_thread);
	MsgBuffer *pBuffer, *pNextBuffer;

	if(pMsgList == NULL)
		return RetCode_empty_data;

	pBuffer = pMsgList->head;

	while(pBuffer != NULL)
	{
		pNextBuffer = pBuffer->next;
		_thread_msg_buffer_free(pBuffer);
		pBuffer = pNextBuffer;
	}

	dave_free(pMsgList);

	THREADDEBUG("%s", dst_thread);

	return RetCode_OK;
}

static void
_thread_msg_buffer_timer_out(MsgBuffer *pBuffer)
{
	ProcessMsgTimerOutMsg *pMsg = base_thread_msg(sizeof(ProcessMsgTimerOutMsg)+pBuffer->msg_len+64, dave_false, (s8 *)__func__, (ub)__LINE__);

	pMsg->msg_id = pBuffer->msg_id;
	pMsg->msg_len = pBuffer->msg_len;
	pMsg->msg_body = &(((u8 *)pMsg)[sizeof(ProcessMsgTimerOutMsg)]);

	THREADLOG("msg_id:%s timer out to:%s", msgstr(pMsg->msg_id), pBuffer->src_thread);

	name_msg(pBuffer->src_thread, MSGID_PROCESS_MSG_TIMER_OUT, pMsg);
}

static void
_thread_msg_buffer_tick(void *ramkv, s8 *dst_thread)
{
	MsgList *pMsgList = (MsgList *)base_ramkv_inq_key_ptr(ramkv, dst_thread);
	MsgBuffer *pRecycle;

	if((pMsgList == NULL) || (pMsgList->head == NULL))
	{
		_thread_msg_buffer_list_free(ramkv, dst_thread);
	}
	else
	{
		THREADDEBUG("%s->%s:%s life:%d/%d",
			pMsgList->head->src_thread, pMsgList->head->dst_thread,
			msgstr(pMsgList->head->msg_id),
			pMsgList->head->life, pMsgList->life);

		if((pMsgList->head->life --) < 0)
		{
			pRecycle = pMsgList->head;
		
			if(pMsgList->head == pMsgList->tail)
				pMsgList->head = pMsgList->tail = NULL;
			else
				pMsgList->head = pMsgList->head->next;

			_thread_msg_buffer_timer_out(pRecycle);

			_thread_msg_buffer_free(pRecycle);
		}

		if((++ pMsgList->life) > THREAD_MSG_BUFFER_LIFE_MAX)
			pMsgList->life = THREAD_MSG_BUFFER_LIFE_MAX;
	}
}

static dave_bool
_thread_msg_buffer_push(void *ramkv, MsgBuffer *pBuffer)
{
	MsgList *pMsgList = (MsgList *)base_ramkv_inq_key_ptr(ramkv, pBuffer->dst_thread);

	if(pMsgList == NULL)
	{
		pMsgList = _thread_msg_buffer_list_malloc(ramkv, pBuffer->dst_thread);
	}

	pBuffer->life = pMsgList->life;
	pMsgList->life = 0;

	if(pMsgList->head == NULL)
	{
		pMsgList->head = pMsgList->tail = pBuffer;
	}
	else
	{
		pMsgList->tail->next = pBuffer;
		pMsgList->tail = pBuffer;
	}

	return dave_true;
}

static MsgBuffer *
_thread_msg_buffer_pop(void *ramkv, s8 *dst_thread)
{
	MsgList *pMsgList = (MsgList *)base_ramkv_inq_key_ptr(ramkv, dst_thread);
	MsgBuffer *pBuffer;

	if(pMsgList == NULL)
		return NULL;

	pBuffer = pMsgList->head;

	if((pMsgList->head == NULL)
		|| (pMsgList->head->next == NULL)
		|| (pMsgList->head == pMsgList->tail))
		pMsgList->head = pMsgList->tail = NULL;
	else
		pMsgList->head = pMsgList->head->next;

	return pBuffer;
}

static void
_thread_msg_buffer_safe_tick(void *ramkv, s8 *dst_thread)
{
	SAFECODEv1(_thread_msg_buf_pv, _thread_msg_buffer_tick(ramkv, dst_thread); );
}

static void
_thread_msg_buffer_safe_push(void *ramkv, MsgBuffer *pBuffer)
{
	SAFECODEv1(_thread_msg_buf_pv, _thread_msg_buffer_push(ramkv, pBuffer); );
}

static MsgBuffer *
_thread_msg_buffer_safe_pop(void *ramkv, s8 *dst_thread)
{
	MsgBuffer *pBuffer = NULL;

	SAFECODEv1(_thread_msg_buf_pv, pBuffer = _thread_msg_buffer_pop(ramkv, dst_thread); );

	return pBuffer;
}

// =====================================================================

void
thread_msg_buffer_init(void)
{
	dave_bool init_flag = dave_false;

	t_lock_spin(NULL);
	if(__init_flag__ != 0x0123456789)
	{
		__init_flag__ = 0x0123456789;
		init_flag = dave_true;
	}
	t_unlock_spin(NULL);

	if(init_flag == dave_true)
	{
		t_lock_reset(&_thread_msg_buf_pv);
		_thread_msg_buf_ramkv = base_ramkv_malloc("tmbk", KvAttrib_ram, 3, _thread_msg_buffer_safe_tick);
	}
}

void
thread_msg_buffer_exit(void)
{
	dave_bool exit_flag = dave_false;

	t_lock_spin(NULL);
	if(__init_flag__ == 0x0123456789)
	{
		__init_flag__ = 0x00;
		exit_flag = dave_true;
	}
	t_unlock_spin(NULL);

	if(exit_flag == dave_true)
	{
		base_ramkv_free(_thread_msg_buf_ramkv, _thread_msg_buffer_list_free);
		_thread_msg_buf_ramkv = NULL;
	}
}

dave_bool
thread_msg_buffer_push(ThreadId src_id, s8 *dst_thread, BaseMsgType msg_type, ub msg_id, ub msg_len, u8 *msg_body, s8 *fun, ub line)
{
	MsgBuffer *pBuffer;

	thread_msg_buffer_init();

	pBuffer = _thread_msg_buffer_malloc(src_id, dst_thread, msg_type, msg_id, msg_len, msg_body, fun, line);

	_thread_msg_buffer_safe_push(_thread_msg_buf_ramkv, pBuffer);

	thread_clean_user_input_data(msg_body, msg_id);

	return dave_true;
}

void
thread_msg_buffer_pop(s8 *dst_thread)
{
	MsgBuffer *pBuffer;
	ThreadId src_id, dst_id;

	thread_msg_buffer_init();

	do {
		pBuffer = _thread_msg_buffer_safe_pop(_thread_msg_buf_ramkv, dst_thread);

		if(pBuffer != NULL)
		{
			src_id = thread_id(pBuffer->src_thread);
			dst_id = thread_id(pBuffer->dst_thread);

			if(base_thread_id_msg(src_id, dst_id,
				pBuffer->msg_type,
				pBuffer->msg_id, pBuffer->msg_len, pBuffer->msg_body,
				0,
				pBuffer->fun, pBuffer->line) == dave_false)
			{
				THREADABNOR("%s->%s:%s send failed! <%s:%d>",
					pBuffer->src_thread, pBuffer->dst_thread, msgstr(pBuffer->msg_id),
					pBuffer->fun, pBuffer->line);
			}

			_thread_msg_buffer_free(pBuffer);
		}

	} while(pBuffer != NULL);

	_thread_msg_buffer_list_free(_thread_msg_buf_ramkv, dst_thread);
}

#endif

