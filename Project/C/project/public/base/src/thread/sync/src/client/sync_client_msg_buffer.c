/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_CLIENT)
#include "dave_verno.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "sync_base_package.h"
#include "sync_param.h"
#include "sync_client_msg_buffer.h"
#include "sync_test.h"
#include "sync_lock.h"
#include "sync_log.h"

#define SYNC_CLIENT_MSG_BUFFER_MAX (81920)
#define SYNC_CLIENT_MSG_BUFFER_BASE_TIMER (1000)
#define SYNC_CLIENT_MSG_BUFFER_LEFT (128)

typedef struct {
	SyncServer *pServer;
	void *pChainBson;
	s8 *src;
	ThreadId route_src;
	s8 *dst;
	ThreadId route_dst;
	ub msg_id;
	BaseMsgType msg_type;
	ub msg_len;
	u8 *msg_body;

	sync_client_run_thread_fun fun;

	sb msg_left;
} SyncClientMsgBuffer;

static TLock _sync_client_msg_buffer_pv;
static TIMERID _sync_client_msg_buffer_timer = INVALID_TIMER_ID;
static sb _sync_client_msg_buffer_number = 0;
static SyncClientMsgBuffer _sync_client_msg_buffer[SYNC_CLIENT_MSG_BUFFER_MAX];

static void _sync_client_msg_buffer_safe_pop(void);

static void
_sync_client_msg_buffer_reset(SyncClientMsgBuffer *pBuffer)
{
	dave_memset(pBuffer, 0x00, sizeof(SyncClientMsgBuffer));

	pBuffer->pServer = NULL;
	pBuffer->pChainBson = NULL;
	pBuffer->src = NULL;
	pBuffer->dst = NULL;
	pBuffer->msg_body = NULL;

	pBuffer->fun = NULL;
}

static void
_sync_client_msg_buffer_build(SyncClientMsgBuffer *pBuffer)
{
	if(pBuffer->src == NULL)
	{
		pBuffer->src = dave_malloc(SYNC_THREAD_NAME_LEN);
	}
	else
	{
		SYNCLOG("Arithmetic error!");
	}
	if(pBuffer->dst == NULL)
	{
		pBuffer->dst = dave_malloc(SYNC_THREAD_NAME_LEN);
	}
	else
	{
		SYNCLOG("Arithmetic error!");
	}
}

static void
_sync_client_msg_buffer_clear(SyncClientMsgBuffer *pBuffer)
{
	if(pBuffer->src != NULL)
	{
		dave_free(pBuffer->src);
	}
	if(pBuffer->dst != NULL)
	{
		dave_free(pBuffer->dst);
	}

	if(pBuffer->pChainBson != NULL)
	{
		t_bson_free_object(pBuffer->pChainBson);
	}

	if(pBuffer->msg_body != NULL)
	{
		thread_msg_release(pBuffer->msg_body);
	}
	
	_sync_client_msg_buffer_reset(pBuffer);
}

static void
_sync_client_msg_buffer_reset_all(void)
{
	ub buffer_index;
	SyncClientMsgBuffer *pBuffer;

	_sync_client_msg_buffer_timer = INVALID_TIMER_ID;
	_sync_client_msg_buffer_number = 0;

	for(buffer_index=0; buffer_index<SYNC_CLIENT_MSG_BUFFER_MAX; buffer_index++)
	{
		pBuffer = &_sync_client_msg_buffer[buffer_index];

		_sync_client_msg_buffer_reset(pBuffer);
	}
}

static void
_sync_client_msg_buffer_clear_all(void)
{
	ub buffer_index;
	SyncClientMsgBuffer *pBuffer;

	if(_sync_client_msg_buffer_timer != INVALID_TIMER_ID)
	{
		base_timer_die(_sync_client_msg_buffer_timer);
		_sync_client_msg_buffer_timer = INVALID_TIMER_ID;
	}
	_sync_client_msg_buffer_number = 0;

	for(buffer_index=0; buffer_index<SYNC_CLIENT_MSG_BUFFER_MAX; buffer_index++)
	{
		pBuffer = &_sync_client_msg_buffer[buffer_index];

		_sync_client_msg_buffer_clear(pBuffer);
	}
}

static void
_sync_client_msg_buffer_timer_out(TIMERID timer_id, ub thread_index)
{
	_sync_client_msg_buffer_safe_pop();
}

static dave_bool
_sync_client_msg_buffer_push(
	SyncServer *pServer,
	void *pChainBson,
	s8 *src, ThreadId route_src,
	s8 *dst, ThreadId route_dst,
	ub msg_id, BaseMsgType msg_type,
	ub msg_len, void *msg_body,
	sync_client_run_thread_fun fun)
{
	dave_bool ret = dave_false;
	ub buffer_index;
	SyncClientMsgBuffer *pBuffer;

	for(buffer_index=0; buffer_index<SYNC_CLIENT_MSG_BUFFER_MAX; buffer_index++)
	{
		pBuffer = &_sync_client_msg_buffer[buffer_index];
	
		if(pBuffer->pServer == NULL)
		{
			_sync_client_msg_buffer_build(pBuffer);
		
			dave_strcpy(pBuffer->src, src, SYNC_THREAD_NAME_LEN);
			pBuffer->route_src = route_src;
			dave_strcpy(pBuffer->dst, dst, SYNC_THREAD_NAME_LEN);
			pBuffer->route_dst = route_dst;
			pBuffer->msg_id = msg_id;
			pBuffer->msg_type = msg_type;
			pBuffer->msg_len = msg_len;
			pBuffer->msg_body = msg_body;
			pBuffer->msg_left = SYNC_CLIENT_MSG_BUFFER_LEFT;
			pBuffer->fun = fun;
			pBuffer->pServer = pServer;
			pBuffer->pChainBson = pChainBson;

			ret = dave_true;

			_sync_client_msg_buffer_number ++;

			break;
		}
	}

	if(_sync_client_msg_buffer_number > 0)
	{
		if(_sync_client_msg_buffer_timer == INVALID_TIMER_ID)
		{
			_sync_client_msg_buffer_timer = base_timer_creat("scmbt", _sync_client_msg_buffer_timer_out, SYNC_CLIENT_MSG_BUFFER_BASE_TIMER);
		}
	}

	return ret;
}

static void
_sync_client_msg_buffer_pop(void)
{
	ub buffer_index;
	SyncClientMsgBuffer *pBuffer;

	if(_sync_client_msg_buffer_number <= 0)
	{
		if(_sync_client_msg_buffer_timer != INVALID_TIMER_ID)
		{
			base_timer_die(_sync_client_msg_buffer_timer);
			_sync_client_msg_buffer_timer = INVALID_TIMER_ID;
		}
		return;
	}

	for(buffer_index=0; buffer_index<SYNC_CLIENT_MSG_BUFFER_MAX; buffer_index++)
	{
		pBuffer = &_sync_client_msg_buffer[buffer_index];

		if(pBuffer->pServer != NULL)
		{
			if((pBuffer->pServer->server_ready == dave_true)
				&& (pBuffer->pServer->shadow_index < SERVER_DATA_MAX)
				&& (thread_id(pBuffer->src) != INVALID_THREAD_ID)
				&& (thread_id(pBuffer->dst) != INVALID_THREAD_ID)
				&& (pBuffer->fun != NULL))
			{
				if(pBuffer->fun(
					pBuffer->pServer,
					pBuffer->pChainBson,
					pBuffer->src, pBuffer->route_src,
					pBuffer->dst, pBuffer->route_dst,
					pBuffer->msg_id, pBuffer->msg_type,
					pBuffer->msg_len, pBuffer->msg_body,
					dave_true) == dave_true)
				{
					SYNCDEBUG("%s->%s:%s buffer pop success!",
						pBuffer->src, pBuffer->dst,
						msgstr(pBuffer->msg_id));

					pBuffer->pChainBson = NULL;
					pBuffer->msg_body = NULL;

					_sync_client_msg_buffer_clear(pBuffer);

					_sync_client_msg_buffer_number --;
				}
			}
			else if((-- pBuffer->msg_left) < 0)
			{
				SYNCLOG("%s->%s:%s buffer remove! fun:%x",
					pBuffer->src, pBuffer->dst, msgstr(pBuffer->msg_id),
					pBuffer->fun);

				_sync_client_msg_buffer_clear(pBuffer);
			
				_sync_client_msg_buffer_number --;
			}
		}
	}

	if(_sync_client_msg_buffer_number < 0)
	{
		SYNCABNOR("Arithmetic error! %d", _sync_client_msg_buffer_number)
	}
}

static dave_bool
_sync_client_msg_buffer_safe_push(
	SyncServer *pServer,
	void *pChainBson,
	s8 *src, ThreadId route_src,
	s8 *dst, ThreadId route_dst,
	ub msg_id, BaseMsgType msg_type,
	ub msg_len, void *msg_body,
	sync_client_run_thread_fun fun)
{
	dave_bool ret = dave_false;

	SAFECODEv1(_sync_client_msg_buffer_pv, {

			ret = _sync_client_msg_buffer_push(
				pServer,
				pChainBson,
				src, route_src,
				dst, route_dst,
				msg_id, msg_type,
				msg_len, msg_body,
				fun);

		}
	);

	SYNCDEBUG("%s<%lx>->%s<%lx> %d", src, route_src, dst, route_dst, msg_id);

	return ret;
}

static void
_sync_client_msg_buffer_safe_pop(void)
{
	SAFECODEv1(_sync_client_msg_buffer_pv, { _sync_client_msg_buffer_pop(); } );
}

static void
_sync_client_msg_buffer_safe_reset_all(void)
{
	SAFECODEv1(_sync_client_msg_buffer_pv, _sync_client_msg_buffer_reset_all(););
}

static void
_sync_client_msg_buffer_safe_clear_all(void)
{
	SAFECODEv1(_sync_client_msg_buffer_pv, _sync_client_msg_buffer_clear_all(););
}

// =====================================================================

void
sync_client_msg_buffer_init(void)
{
	t_lock_reset(&_sync_client_msg_buffer_pv);

	SYNCDEBUG("msg buffer size:%dMB", sizeof(_sync_client_msg_buffer) / (1024 * 1024));

	_sync_client_msg_buffer_safe_reset_all();
}

void
sync_client_msg_buffer_exit(void)
{
	_sync_client_msg_buffer_safe_clear_all();
}

void
sync_client_msg_buffer_push(
	SyncServer *pServer,
	void *pChainBson,
	s8 *src, ThreadId route_src,
	s8 *dst, ThreadId route_dst,
	ub msg_id, BaseMsgType msg_type,
	ub msg_len, void *msg_body,
	sync_client_run_thread_fun fun)
{
	if(_sync_client_msg_buffer_safe_push(
		pServer,
		pChainBson,
		src, route_src,
		dst, route_dst,
		msg_id, msg_type,
		msg_len, msg_body,
		fun) == dave_false)
	{
		SYNCABNOR("%s->%s:%d buffer push failed! shadow_index:%d thread_id:%lx",
			src, dst, msg_id, pServer->shadow_index, thread_id(src));
	}
	else
	{
		SYNCTRACE("%s->%s %d buffer push success!", src, dst, msg_id);
	}
}

ub
sync_client_msg_buffer_info(s8 *info, ub info_len)
{
	return dave_snprintf(info, info_len,
		"MESSAGE BUFFER NUMBER:%d/%d\n",
		_sync_client_msg_buffer_number,
		SYNC_CLIENT_MSG_BUFFER_MAX);
}

#endif

