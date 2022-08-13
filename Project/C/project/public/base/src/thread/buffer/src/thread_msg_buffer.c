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
#include "thread_gid_table.h"
#include "thread_mem.h"
#include "thread_log.h"

#define THREAD_MSG_BUFFER_LIFE_MAX 60
#define THREAD_MSG_BUFFER_BASE_TIMER 3

typedef struct {
	ThreadId src_id;
	s8 gid[DAVE_GLOBALLY_IDENTIFIER_LEN];
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

static s8 *
_thread_msg_buffer_key(s8 *buffer_key_ptr, ub buffer_ken_len, s8 *gid, s8 *dst_thread)
{
	if(dst_thread == NULL)
	{
		THREADABNOR("gid:%s has empty dst_thread!", gid);
	}
	
	dave_snprintf(buffer_key_ptr, buffer_ken_len, "%s-%s", gid, dst_thread);

	return buffer_key_ptr;
}

static MsgBuffer *
_thread_msg_buffer_malloc(
	ThreadId src_id, s8 *gid, s8 *dst_thread,
	BaseMsgType msg_type,
	ub msg_id, ub msg_len, u8 *msg_body,
	s8 *fun, ub line)
{
	MsgBuffer *pBuffer;

	if(src_id == INVALID_THREAD_ID)
	{
		src_id = self();
	}

	pBuffer = dave_malloc(sizeof(MsgBuffer));

	pBuffer->src_id = src_id;
	if(gid != NULL)
	{
		dave_strcpy(pBuffer->gid, gid, sizeof(pBuffer->gid));
	}
	else
	{
		pBuffer->gid[0] = '\0';
	}
	if(dst_thread != NULL)
	{
		dave_strcpy(pBuffer->dst_thread, dst_thread, sizeof(pBuffer->dst_thread));
	}
	else
	{
		pBuffer->dst_thread[0] = '\0';
	}
	pBuffer->msg_type = msg_type;
	pBuffer->msg_id = msg_id;
	pBuffer->msg_len = msg_len;
	pBuffer->msg_body = base_thread_msg(msg_len, dave_false, fun, line);;
	dave_memcpy(pBuffer->msg_body, msg_body, msg_len);
	pBuffer->fun = fun;
	pBuffer->line = line;

	pBuffer->life = THREAD_MSG_BUFFER_LIFE_MAX;

	pBuffer->next = NULL;

	return pBuffer;
}

static void
_thread_msg_buffer_free(MsgBuffer *pBuffer)
{
	dave_free(pBuffer);
}

static MsgList *
_thread_msg_buffer_list_inq(void *ramkv, s8 *gid, s8 *dst_thread, s8 *buffer_key)
{
	if((gid != NULL) && (gid[0] != '\0'))
	{
		s8 inq_buffer_key[256];
		return (MsgList *)kv_inq_key_ptr(ramkv, _thread_msg_buffer_key(inq_buffer_key, sizeof(inq_buffer_key), gid, dst_thread));
	}
	else if((dst_thread != NULL) && (dst_thread[0] != '\0'))
	{
		return (MsgList *)kv_inq_key_ptr(ramkv, dst_thread);
	}
	else if((buffer_key != NULL) && (buffer_key[0] != '\0'))
	{
		return (MsgList *)kv_inq_key_ptr(ramkv, buffer_key);
	}

	return NULL;
}

static MsgList *
_thread_msg_buffer_list_malloc(void *ramkv, s8 *gid, s8 *dst_thread)
{
	MsgList *pMsgList = dave_ralloc(sizeof(MsgList));

	if((gid != NULL) && (gid[0] != '\0'))
	{
		s8 buffer_key[256];
		kv_add_key_ptr(ramkv, _thread_msg_buffer_key(buffer_key, sizeof(buffer_key), gid, dst_thread), pMsgList);
	}
	else
	{
		kv_add_key_ptr(ramkv, dst_thread, pMsgList);
	}

	pMsgList->head = pMsgList->tail = NULL;
	pMsgList->life = THREAD_MSG_BUFFER_LIFE_MAX;

	THREADDEBUG("%s", dst_thread);

	return pMsgList;
}

static RetCode
_thread_msg_buffer_list_free(void *ramkv, s8 *gid, s8 *dst_thread, s8 *buffer_key)
{
	MsgList *pMsgList;
	MsgBuffer *pBuffer, *pNextBuffer;

	if((gid != NULL) && (gid[0] != '\0'))
	{
		s8 buffer_key[256];
		pMsgList = kv_del_key_ptr(ramkv, _thread_msg_buffer_key(buffer_key, sizeof(buffer_key), gid, dst_thread));
	}
	else if((dst_thread != NULL) && (dst_thread[0] != '\0'))
	{
		pMsgList = kv_del_key_ptr(ramkv, dst_thread);
	}
	else if((buffer_key != NULL) && (buffer_key[0] != '\0'))
	{
		pMsgList = kv_del_key_ptr(ramkv, buffer_key);
	}
	else
	{
		pMsgList = NULL;
	}

	if(pMsgList == NULL)
	{
		return RetCode_empty_data;
	}

	pBuffer = pMsgList->head;

	while(pBuffer != NULL)
	{
		pNextBuffer = pBuffer->next;
		_thread_msg_buffer_free(pBuffer);
		pBuffer = pNextBuffer;
	}

	dave_free(pMsgList);

	return RetCode_OK;
}

static dave_bool
_thread_msg_buffer_push(void *ramkv, MsgBuffer *pBuffer)
{
	MsgList *pMsgList;

	pMsgList = _thread_msg_buffer_list_inq(ramkv, pBuffer->gid, pBuffer->dst_thread, NULL);
	if(pMsgList == NULL)
	{
		pMsgList = _thread_msg_buffer_list_malloc(ramkv, pBuffer->gid, pBuffer->dst_thread);
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
_thread_msg_buffer_pop(void *ramkv, s8 *gid, s8 *dst_thread)
{
	MsgList *pMsgList = _thread_msg_buffer_list_inq(ramkv, gid, dst_thread, NULL);
	MsgBuffer *pBuffer;

	if(pMsgList == NULL)
	{
		return NULL;
	}

	pBuffer = pMsgList->head;

	if((pMsgList->head == NULL)
		|| (pMsgList->head->next == NULL)
		|| (pMsgList->head == pMsgList->tail))
	{
		pMsgList->head = pMsgList->tail = NULL;
	}
	else
	{
		pMsgList->head = pMsgList->head->next;
	}

	return pBuffer;
}

static void
_thread_msg_buffer_thread_pop(void *ramkv, s8 *dst_thread)
{
	MsgBuffer *pBuffer;
	ThreadId dst_id;

	do {
		pBuffer = _thread_msg_buffer_pop(ramkv, NULL, dst_thread);
		if(pBuffer == NULL)
			break;

		dst_id = thread_id(pBuffer->dst_thread);

		if(dst_id != INVALID_THREAD_ID)
		{
			if(base_thread_id_msg(
				NULL,
				pBuffer->src_id, dst_id,
				pBuffer->msg_type,
				pBuffer->msg_id, pBuffer->msg_len, pBuffer->msg_body,
				0,
				pBuffer->fun, pBuffer->line) == dave_false)
			{
				THREADABNOR("%s->%s:%s send failed! <%s:%d>",
					thread_id_to_name(pBuffer->src_id), pBuffer->dst_thread, msgstr(pBuffer->msg_id),
					pBuffer->fun, pBuffer->line);
			}
		}
		else
		{
			THREADABNOR("%s->%s:%s not ready! <%s:%d>",
				thread_id_to_name(pBuffer->src_id), pBuffer->dst_thread, msgstr(pBuffer->msg_id),
				pBuffer->fun, pBuffer->line);
		}

		_thread_msg_buffer_free(pBuffer);
	} while(pBuffer != NULL);
}

static void
_thread_msg_buffer_gid_pop(void *ramkv, s8 *gid, s8 *dst_thread)
{
	MsgBuffer *pBuffer;
	ThreadId dst_id;

	do {
		pBuffer = _thread_msg_buffer_pop(ramkv, gid, dst_thread);
		if(pBuffer == NULL)
			break;

		dst_id = thread_gid_table_inq(gid, dst_thread);

		if(dst_id != INVALID_THREAD_ID)
		{
			if(base_thread_id_msg(
				NULL,
				pBuffer->src_id, dst_id,
				pBuffer->msg_type,
				pBuffer->msg_id, pBuffer->msg_len, pBuffer->msg_body,
				0,
				pBuffer->fun, pBuffer->line) == dave_false)
			{
				THREADABNOR("%s->%s:%s send failed! <%s:%d>",
					thread_id_to_name(pBuffer->src_id), pBuffer->dst_thread, msgstr(pBuffer->msg_id),
					pBuffer->fun, pBuffer->line);
			}
		}
		else
		{
			THREADABNOR("%s->%s:%s not ready! <%s:%d>",
				thread_id_to_name(pBuffer->src_id), pBuffer->dst_thread, msgstr(pBuffer->msg_id),
				pBuffer->fun, pBuffer->line);
		}

		_thread_msg_buffer_free(pBuffer);
	} while(pBuffer != NULL);
}

static void
_thread_msg_buffer_timer_out(MsgBuffer *pBuffer)
{
	ProcessMsgTimerOutMsg *pMsg = base_thread_msg(sizeof(ProcessMsgTimerOutMsg)+pBuffer->msg_len+64, dave_false, (s8 *)__func__, (ub)__LINE__);

	pMsg->msg_id = pBuffer->msg_id;
	pMsg->msg_len = pBuffer->msg_len;
	pMsg->msg_body = &(((u8 *)pMsg)[sizeof(ProcessMsgTimerOutMsg)]);

	THREADLOG("msg_id:%s(%s/%s) timer out to:%s",
		msgstr(pMsg->msg_id), pBuffer->gid, pBuffer->dst_thread,
		thread_id_to_name(pBuffer->src_id));

	id_msg(pBuffer->src_id, MSGID_PROCESS_MSG_TIMER_OUT, pMsg);
}

static MsgBuffer *
_thread_msg_buffer_is_timer_out(MsgList *pMsgList)
{
	MsgBuffer *pRecycle;

	if((pMsgList->head->life --) < 0)
	{
		pRecycle = pMsgList->head;

		if(pMsgList->head == pMsgList->tail)
			pMsgList->head = pMsgList->tail = NULL;
		else
			pMsgList->head = pMsgList->head->next;

		_thread_msg_buffer_timer_out(pRecycle);

		_thread_msg_buffer_free(pRecycle);

		return NULL;
	}

	return pMsgList->head;
}

static void
_thread_msg_buffer_is_ready(void *ramkv, MsgList *pMsgList)
{
	MsgBuffer *pReady = pMsgList->head;

	if((pReady->gid[0] != '\0') && (pReady->dst_thread[0] != '\0'))
	{
		if(thread_gid_table_inq(pReady->gid, pReady->dst_thread) != INVALID_THREAD_ID)
		{
			_thread_msg_buffer_gid_pop(ramkv, pReady->gid, pReady->dst_thread);	
		}
	}
	else if(pReady->dst_thread[0] != '\0')
	{
		if(thread_id(pReady->dst_thread) != INVALID_THREAD_ID)
		{
			_thread_msg_buffer_thread_pop(ramkv, pReady->dst_thread);	
		}
	}
	else
	{
		THREADABNOR("find invalid buffer! %s->%s:%s gid:%s <%s:%d>",
			thread_id_to_name(pReady->src_id), pReady->dst_thread, msgstr(pReady->msg_id),
			pReady->gid,
			pReady->fun, pReady->line);
	}
}

static void
_thread_msg_buffer_tick(void *ramkv, s8 *buffer_key)
{
	MsgList *pMsgList = _thread_msg_buffer_list_inq(ramkv, NULL, NULL, buffer_key);

	if((pMsgList == NULL) || (pMsgList->head == NULL))
	{
		_thread_msg_buffer_list_free(ramkv, NULL, NULL, buffer_key);
	}
	else
	{
		THREADDEBUG("%s->%s:%s life:%d/%d",
			thread_id_to_name(pMsgList->head->src_id), pMsgList->head->dst_thread,
			msgstr(pMsgList->head->msg_id),
			pMsgList->head->life, pMsgList->life);

		if(_thread_msg_buffer_is_timer_out(pMsgList) != NULL)
		{
			_thread_msg_buffer_is_ready(ramkv, pMsgList);
		}

		if((++ pMsgList->life) > THREAD_MSG_BUFFER_LIFE_MAX)
		{
			pMsgList->life = THREAD_MSG_BUFFER_LIFE_MAX;
		}
	}
}

static void
_thread_msg_buffer_safe_tick(void *ramkv, s8 *buffer_key)
{
	SAFECODEv1(_thread_msg_buf_pv, _thread_msg_buffer_tick(ramkv, buffer_key); );
}

static void
_thread_msg_buffer_safe_push(void *ramkv, MsgBuffer *pBuffer)
{
	SAFECODEv1(_thread_msg_buf_pv, _thread_msg_buffer_push(ramkv, pBuffer); );
}

static void
_thread_msg_buffer_safe_thread_pop(s8 *dst_thread)
{
	SAFECODEv1(_thread_msg_buf_pv, _thread_msg_buffer_thread_pop(_thread_msg_buf_ramkv, dst_thread); );
}

static void
_thread_msg_buffer_safe_gid_pop(s8 *gid, s8 *dst_thread)
{
	SAFECODEv1(_thread_msg_buf_pv, _thread_msg_buffer_gid_pop(_thread_msg_buf_ramkv, gid, dst_thread); );
}

static RetCode
_thread_msg_buffer_safe_recycle(void *ramkv, s8 *buffer_key)
{
	RetCode ret;

	SAFECODEv1(_thread_msg_buf_pv, ret = _thread_msg_buffer_list_free(ramkv, NULL, NULL, buffer_key); );

	return ret;
}

static dave_bool
_thread_msg_buffer_public_push(
	ThreadId src_id, s8 *gid, s8 *dst_thread,
	BaseMsgType msg_type,
	ub msg_id, ub msg_len, u8 *msg_body,
	s8 *fun, ub line)
{
	MsgBuffer *pBuffer;

	pBuffer = _thread_msg_buffer_malloc(
		src_id, gid, dst_thread,
		msg_type,
		msg_id, msg_len, msg_body,
		fun, line);

	_thread_msg_buffer_safe_push(_thread_msg_buf_ramkv, pBuffer);

	thread_clean_user_input_data(msg_body, msg_id);

	return dave_true;
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
		_thread_msg_buf_ramkv = base_ramkv_malloc("tmbk", KvAttrib_list, THREAD_MSG_BUFFER_BASE_TIMER, _thread_msg_buffer_safe_tick);
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
		base_ramkv_free(_thread_msg_buf_ramkv, _thread_msg_buffer_safe_recycle);
		_thread_msg_buf_ramkv = NULL;
	}
}

dave_bool
thread_msg_buffer_thread_push(
	ThreadId src_id, s8 *dst_thread,
	BaseMsgType msg_type,
	ub msg_id, ub msg_len, u8 *msg_body,
	s8 *fun, ub line)
{
	thread_msg_buffer_init();

	return _thread_msg_buffer_public_push(
		src_id, NULL, dst_thread,
		msg_type,
		msg_id, msg_len, msg_body,
		fun, line);
}

void
thread_msg_buffer_thread_pop(s8 *dst_thread)
{
	thread_msg_buffer_init();

	_thread_msg_buffer_safe_thread_pop(dst_thread);
}

dave_bool
thread_msg_buffer_gid_push(
	ThreadId src_id, s8 *gid, s8 *dst_thread,
	BaseMsgType msg_type,
	ub msg_id, ub msg_len, u8 *msg_body,
	s8 *fun, ub line)
{
	thread_msg_buffer_init();

	return _thread_msg_buffer_public_push(
		src_id, gid, dst_thread,
		msg_type,
		msg_id, msg_len, msg_body,
		fun, line);
}

void
thread_msg_buffer_gid_pop(s8 *gid, s8 *dst_thread)
{
	thread_msg_buffer_init();

	_thread_msg_buffer_safe_gid_pop(gid, dst_thread);
}

#endif

