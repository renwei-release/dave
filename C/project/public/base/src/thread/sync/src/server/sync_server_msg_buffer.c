/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_SERVER)
#include "dave_verno.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "sync_base_package.h"
#include "sync_param.h"
#include "sync_server_msg_buffer.h"
#include "sync_test.h"
#include "sync_lock.h"
#include "sync_log.h"

#define SYNC_SERVER_MSG_BUFFER_MAX (819200)
#define SYNC_SERVER_MSG_BUFFER_LEFT (128)

typedef struct {
	SyncClient *pClient;
	ub frame_len;
	u8 *frame;

	sync_server_run_thread_fun fun;
	sb msg_left;
} SyncServerMsgBuffer;

static ub _sync_server_msg_buffer_push_index = 0;
static ub _sync_server_msg_buffer_pop_index = 0;
static ub _sync_server_msg_buffer_number = 0;
static TLock _sync_server_msg_buffer_pv;
static SyncServerMsgBuffer _sync_server_msg_buffer[SYNC_SERVER_MSG_BUFFER_MAX];

static void
_sync_server_msg_buffer_reset(SyncServerMsgBuffer *pBuffer)
{
	dave_memset(pBuffer, 0x00, sizeof(SyncServerMsgBuffer));

	pBuffer->pClient = NULL;
}

static void
_sync_server_msg_buffer_clear(SyncServerMsgBuffer *pBuffer)
{
	if(pBuffer->frame != NULL)
	{
		dave_free(pBuffer->frame);
	}

	_sync_server_msg_buffer_reset(pBuffer);
}

static void
_sync_server_msg_buffer_reset_all(void)
{
	ub buffer_index;
	SyncServerMsgBuffer *pBuffer;

	t_lock_reset(&_sync_server_msg_buffer_pv);

	for(buffer_index=0; buffer_index<SYNC_SERVER_MSG_BUFFER_MAX; buffer_index++)
	{
		pBuffer = &_sync_server_msg_buffer[buffer_index];

		_sync_server_msg_buffer_reset(pBuffer);
	}
}

static void
_sync_server_msg_buffer_clear_all(void)
{
	ub buffer_index;
	SyncServerMsgBuffer *pBuffer;

	for(buffer_index=0; buffer_index<SYNC_SERVER_MSG_BUFFER_MAX; buffer_index++)
	{
		pBuffer = &_sync_server_msg_buffer[buffer_index];

		_sync_server_msg_buffer_clear(pBuffer);
	}
}

static dave_bool
_sync_server_msg_buffer_push(
	SyncClient *pClient,
	ub frame_len,
	u8 *frame,
	sync_server_run_thread_fun fun)
{
	dave_bool ret = dave_false;
	ub buffer_index, safe_counter;
	SyncServerMsgBuffer *pBuffer;

	if(_sync_server_msg_buffer_number >= SYNC_SERVER_MSG_BUFFER_MAX)
	{
		return dave_false;
	}

	buffer_index = (_sync_server_msg_buffer_push_index ++) % SYNC_SERVER_MSG_BUFFER_MAX;

	for(safe_counter=0; safe_counter<SYNC_SERVER_MSG_BUFFER_MAX; safe_counter++)
	{
		if(buffer_index >= SYNC_SERVER_MSG_BUFFER_MAX)
			buffer_index = 0;

		pBuffer = &_sync_server_msg_buffer[buffer_index ++];
	
		if(pBuffer->pClient == NULL)
		{
			pBuffer->pClient = pClient;
			pBuffer->frame_len = frame_len;
			pBuffer->frame = dave_malloc(pBuffer->frame_len);
			dave_memcpy(pBuffer->frame, frame, pBuffer->frame_len);
			pBuffer->msg_left = SYNC_SERVER_MSG_BUFFER_LEFT;
			pBuffer->fun = fun;

			ret = dave_true;

			_sync_server_msg_buffer_push_index = ++ buffer_index;
			_sync_server_msg_buffer_number ++;

			break;
		}
	}

	return ret;
}

static void
_sync_server_msg_buffer_pop(void)
{
	ub buffer_index, safe_counter;
	SyncServerMsgBuffer *pBuffer;

	if(_sync_server_msg_buffer_number == 0)
	{
		return;
	}

	buffer_index = (_sync_server_msg_buffer_pop_index ++) % SYNC_SERVER_MSG_BUFFER_MAX;

	for(safe_counter=0; safe_counter<SYNC_SERVER_MSG_BUFFER_MAX; safe_counter++)
	{
		if(buffer_index >= SYNC_SERVER_MSG_BUFFER_MAX)
			buffer_index = 0;

		pBuffer = &_sync_server_msg_buffer[buffer_index ++];

		if(pBuffer->pClient != NULL)
		{
			if(pBuffer->fun(
				pBuffer->pClient,
				pBuffer->frame_len,
				pBuffer->frame,
				dave_true) == RetCode_OK)
			{
				SYNCTRACE("%s buffer pop success!", pBuffer->pClient->verno);

				_sync_server_msg_buffer_clear(pBuffer);

				_sync_server_msg_buffer_number --;
			}
			else if((-- pBuffer->msg_left) < 0)
			{
				SYNCLTRACE(10,1,"%s buffer msg_id:%d remove!",
					pBuffer->pClient->verno,
					sync_msg_unpacket_msg_id(pBuffer->frame, pBuffer->frame_len));

				_sync_server_msg_buffer_clear(pBuffer);

				_sync_server_msg_buffer_number --;
			}
			else
			{
				/*
				 * 考虑到for循环的效率问题，只要发现一个节点还没就绪，
				 * 不能发送的情况时，就退出这个操作。等待下次超时检测。
				 */
				_sync_server_msg_buffer_pop_index = buffer_index;

				safe_counter = SYNC_SERVER_MSG_BUFFER_MAX;

				break;
			}
		}
	}
}

static dave_bool
_sync_server_msg_buffer_safe_push(
	SyncClient *pClient,
	ub frame_len,
	u8 *frame,
	sync_server_run_thread_fun fun)
{
	dave_bool ret = dave_false;

	SAFECODEv1(_sync_server_msg_buffer_pv, {

			ret = _sync_server_msg_buffer_push(
				pClient,
				frame_len,
				frame,
				fun);
		}

	);

	return ret;
}

static void
_sync_server_msg_buffer_safe_pop(void)
{
	SAFECODEidlev1(_sync_server_msg_buffer_pv, { _sync_server_msg_buffer_pop(); } );
}

static void
_sync_server_msg_buffer_timer(TIMERID timer_id, ub thread_index)
{
	_sync_server_msg_buffer_safe_pop();
}

// =====================================================================

void
sync_server_msg_buffer_init(void)
{
	_sync_server_msg_buffer_push_index = 0;
	_sync_server_msg_buffer_pop_index = 0;
	_sync_server_msg_buffer_number = 0;

	_sync_server_msg_buffer_reset_all();

	base_timer_creat("scmbt", _sync_server_msg_buffer_timer, 3000);

	SYNCDEBUG("msg buffer size:%dMB", sizeof(_sync_server_msg_buffer)/(1024*1024));
}

void
sync_server_msg_buffer_exit(void)
{
	_sync_server_msg_buffer_clear_all();
}

void
sync_server_msg_buffer_push(
	SyncClient *pClient,
	ub frame_len, u8 *frame,
	sync_server_run_thread_fun fun)
{
	if(_sync_server_msg_buffer_safe_push(
		pClient,
		frame_len, frame,
		fun) == dave_false)
	{
		SYNCLTRACE(60,1,"%s buffer push failed! buffer number:%d",
			pClient->verno, _sync_server_msg_buffer_number);
	}
	else
	{
		SYNCDEBUG("%s buffer push success!", pClient->verno);
	}
}

ub
sync_server_msg_buffer_info(s8 *info, ub info_len)
{
	return dave_snprintf(info, info_len,
		"MESSAGE BUFFER NUMBER:%d/%d\n",
		_sync_server_msg_buffer_number,
		SYNC_SERVER_MSG_BUFFER_MAX);
}

#endif

