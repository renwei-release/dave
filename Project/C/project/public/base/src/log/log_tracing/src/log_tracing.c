/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "log_stack.h"
#ifdef LOG_STACK_SERVER
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_3rdparty.h"
#include "thread_chain.h"
#include "tracing_logic.h"
#include "log_log.h"

#define LOG_TRACING_TIME 10
#define LOG_TRACING_LIFE 3

typedef struct {
	s8 chain_id[DAVE_CHAIN_ID_LEN];

	sb life;

	void *pArrayJson;	
} TracingBody;

static void *_log_tracing_kv = NULL;
static TLock _log_tracing_pv;

static inline TracingBody *
_log_tracing_malloc(s8 *chain_id)
{
	TracingBody *pBody = dave_malloc(sizeof(TracingBody));

	dave_strcpy(pBody->chain_id, chain_id, sizeof(pBody->chain_id));

	pBody->life = LOG_TRACING_LIFE;

	pBody->pArrayJson = dave_json_array_malloc();

	return pBody;
}

static inline void
_log_tracing_free(TracingBody *pBody)
{
	if(pBody == NULL)
		return;

	if(pBody->pArrayJson != NULL)
	{
		dave_json_free(pBody->pArrayJson);
		pBody->pArrayJson = NULL;
	}

	dave_free(pBody);
}

static inline dave_bool
_log_tracing_add(s8 *chain_id, void *pJson)
{
	TracingBody *pBody = NULL;
	dave_bool add_json_ret = dave_false;

	SAFECODEv1(_log_tracing_pv, {

		pBody = kv_inq_key_ptr(_log_tracing_kv, chain_id);
		if(pBody == NULL)
		{
			pBody = _log_tracing_malloc(chain_id);

			kv_add_key_ptr(_log_tracing_kv, pBody->chain_id, pBody);
		}

	} );

	if((pBody != NULL) && (pBody->pArrayJson != NULL))
	{
		add_json_ret = dave_json_array_add_object(pBody->pArrayJson, pJson);
	}

	return add_json_ret;
}

static inline void *
_log_tracing_del(TracingBody *pBody)
{
	SAFECODEv1(_log_tracing_pv, {

		kv_del_key_ptr(_log_tracing_kv, pBody->chain_id);

	} );

	return pBody;
}

static void
_log_tracing_logic(MSGBODY *msg)
{
	MsgInnerLoop *pLoop = (MsgInnerLoop *)(msg->msg_body);
	TracingBody *pBody = (TracingBody *)(pLoop->ptr);

	if(pBody->pArrayJson != NULL)
	{
		tracing_logic(pBody->pArrayJson);
	}

	_log_tracing_free(pBody);
}

static void
_log_tracing_timer_out(void *ramkv, s8 *key)
{
	TracingBody *pBody = kv_inq_key_ptr(_log_tracing_kv, key);
	MsgInnerLoop loop;

	if(pBody == NULL)
	{
		LOGABNOR("can't find the key:%s", key);
		return;
	}

	if(dave_strcmp(pBody->chain_id, key) == dave_false)
	{
		LOGABNOR("Arithmetic error:%s/%s", pBody->chain_id, key);
		return;
	}

	if((-- pBody->life) <= 0)
	{
		loop.ptr = _log_tracing_del(pBody);

		if(pBody->pArrayJson != NULL)
		{
			id_msg(self(), MSGID_INNER_LOOP, &loop);
		}
	}
}

static RetCode
_log_tracing_recycle(void *ramkv, s8 *key)
{
	TracingBody *pBody = kv_del_key_ptr(ramkv, key);

	if(pBody == NULL)
	{
		return RetCode_empty_data;
	}

	_log_tracing_free(pBody);

	return RetCode_OK;
}

// =====================================================================

void
log_tracing_init(void)
{
	_log_tracing_kv = kv_malloc("logtracing", LOG_TRACING_TIME, _log_tracing_timer_out);

	t_lock_reset(&_log_tracing_pv);

	tracing_logic_init();

	reg_msg(MSGID_INNER_LOOP, _log_tracing_logic);
}

void
log_tracing_exit(void)
{
	unreg_msg(MSGID_INNER_LOOP);

	kv_free(_log_tracing_kv, _log_tracing_recycle);

	tracing_logic_exit();
}

dave_bool
log_tracing(ThreadChain *pChain, void *pJson)
{
	return _log_tracing_add(pChain->chain_id, pJson);
}

#endif

