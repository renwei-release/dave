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
#include "thread_tools.h"
#include "thread_parameter.h"
#include "thread_mem.h"
#include "thread_log.h"

#define MAX_MSG_CALL_FUN (512)

typedef struct {
	ThreadId thread_id;
	TLock pv_opt;
	MsgCallFun call_fun[MAX_MSG_CALL_FUN];
} MsgCall;

static MsgCall _msg_call[THREAD_MAX];

static void
_thread_msg_fun_reset(MsgCallFun *pFun)
{
	dave_memset(pFun, 0x00, sizeof(MsgCallFun));

	pFun->msg_id = MSGID_RESERVED;
	pFun->msg_fun = NULL;
}

static void
_thread_msg_fun_all_reset(MsgCall *pCall)
{
	ub fun_index;

	for(fun_index=0; fun_index<MAX_MSG_CALL_FUN; fun_index++)
	{
		_thread_msg_fun_reset(&(pCall->call_fun[fun_index]));
	}
}

static void
_thread_msg_call_reset(MsgCall *pCall)
{
	pCall->thread_id = INVALID_THREAD_ID;

	_thread_msg_fun_all_reset(pCall);
}

static void
_thread_msg_call_reset_all(void)
{
	ub call_index;

	for(call_index=0; call_index<THREAD_MAX; call_index++)
	{
		dave_memset(&_msg_call[call_index], 0x00, sizeof(MsgCall));

		_thread_msg_call_reset(&_msg_call[call_index]);

		t_lock_reset(&(_msg_call[call_index].pv_opt));
	}
}

static ub
_thread_msg_call_id_to_index(ThreadId thread_id)
{
	return (ub)(thread_id % THREAD_MAX);
}

static MsgCallFun *
_thread_msg_id_to_fun(MsgCall *pCall, ub msg_id, dave_bool find_new)
{
	ub fun_index, safe_counter;
	MsgCallFun *pFun;

	fun_index = msg_id % MAX_MSG_CALL_FUN;

	pFun = NULL;

	for(safe_counter=0; safe_counter<MAX_MSG_CALL_FUN; safe_counter++)
	{
		if(fun_index >= MAX_MSG_CALL_FUN)
		{
			fun_index = 0;
		}

		if(((find_new == dave_true) && (pCall->call_fun[fun_index].msg_id == MSGID_RESERVED))
			|| (pCall->call_fun[fun_index].msg_id == msg_id))
		{
			pFun = &(pCall->call_fun[fun_index]);
			break;
		}

		fun_index ++;
	}

	return pFun;
}

static dave_bool
_thread_msg_call_register_new(MsgCall *pCall, ub msg_id, thread_msg_fun msg_fun, void *user_ptr)
{
	MsgCallFun *pFun;

	pFun = _thread_msg_id_to_fun(pCall, msg_id, dave_true);
	if(pFun == NULL)
	{
		THREADABNOR("Limited resources! %s:%d", thread_name(pCall->thread_id), msg_id);
		return dave_false;
	}

	pFun->msg_id = msg_id;
	pFun->msg_fun = msg_fun;
	pFun->user_ptr = user_ptr;

	return dave_true;
}

static dave_bool
_thread_msg_call_register_old(MsgCallFun *pFun, MsgCall *pCall, ub msg_id, thread_msg_fun msg_fun, void *user_ptr)
{
	pFun->msg_id = msg_id;
	if(msg_fun != pFun->msg_fun)
	{
		THREADLOG("Please pay attention, it will cause the message to be received. "\
			"messages can only be processed by the last passed function."\
			"thread:%s msg:%d fun change!",
			thread_name(pCall->thread_id), msg_id);
	}
	pFun->msg_fun = msg_fun;
	pFun->user_ptr = user_ptr;

	return dave_true;
}

static void
_thread_msg_call_unregister(MsgCall *pCall, ub msg_id)
{
	MsgCallFun *pFun;

	pFun = _thread_msg_id_to_fun(pCall, msg_id, dave_false);
	if(pFun == NULL)
	{
		return;
	}

	_thread_msg_fun_reset(pFun);
}

static MsgCallFun *
_thread_msg_call(MsgCall *pCall, ub msg_id)
{
	MsgCallFun *pFun;

	pFun = _thread_msg_id_to_fun(pCall, msg_id, dave_false);
	if(pFun == NULL)
	{
		return NULL;
	}

	return pFun;
}

// =====================================================================

void
thread_msg_call_init(void)
{
	_thread_msg_call_reset_all();
}

void
thread_msg_call_exit(void)
{

}

dave_bool
thread_msg_call_register(ThreadId thread_id, ub msg_id, thread_msg_fun msg_fun, void *user_ptr)
{
	ub call_index;
	MsgCallFun *pFun;
	dave_bool ret = dave_false;

	if((thread_id == INVALID_THREAD_ID) || (msg_fun == NULL))
	{
		THREADABNOR("invalid param:%d,%x", thread_id, msg_fun);
		return dave_false;
	}

	thread_id = thread_get_local(thread_id);

	call_index = _thread_msg_call_id_to_index(thread_id);

	if(call_index >= THREAD_MAX)
	{
		THREADABNOR("invalid call_index:%d thread_id:%d", call_index, thread_id);
		return dave_false;
	}

	if((_msg_call[call_index].thread_id != INVALID_THREAD_ID) && (_msg_call[call_index].thread_id != thread_id))
	{
		THREADABNOR("thread_id is change:%d->%d", _msg_call[call_index].thread_id, thread_id);
		return dave_false;
	}

	SAFEZONEv3(_msg_call[call_index].pv_opt, {
		_msg_call[call_index].thread_id = thread_id;

		pFun = _thread_msg_call(&_msg_call[call_index], msg_id);
		if(pFun == NULL)
		{
			ret = _thread_msg_call_register_new(&_msg_call[call_index], msg_id, msg_fun, user_ptr);
		}
		else
		{
			ret = _thread_msg_call_register_old(pFun, &_msg_call[call_index], msg_id, msg_fun, user_ptr);
		}
	} );

	if(ret == dave_false)
	{
		THREADABNOR("%s register id:%d fun:%x failed!", thread_name(thread_id), msg_id, msg_fun);
	}

	return ret;
}

void
thread_msg_call_unregister(ThreadId thread_id, ub msg_id)
{
	ub call_index;

	thread_id = thread_get_local(thread_id);

	call_index = _thread_msg_call_id_to_index(thread_id);

	if(call_index >= THREAD_MAX)
	{
		THREADABNOR("invalid call_index:%d thread_id:%d", call_index, thread_id);
		return;
	}

	if((_msg_call[call_index].thread_id != INVALID_THREAD_ID) && (_msg_call[call_index].thread_id != thread_id))
	{
		THREADABNOR("thread_id is change:%d->%d", _msg_call[call_index].thread_id, thread_id);
		return;
	}

	SAFEZONEv3(_msg_call[call_index].pv_opt, {
		_msg_call[call_index].thread_id = thread_id;
		
		_thread_msg_call_unregister(&_msg_call[call_index], msg_id);
	} );
}

MsgCallFun *
thread_msg_call(ThreadId thread_id, ub msg_id)
{
	ub call_index;

	thread_id = thread_get_local(thread_id);

	call_index = _thread_msg_call_id_to_index(thread_id);

	if(call_index >= THREAD_MAX)
	{
		THREADABNOR("invalid call_index:%d thread_id:%d msg_id:%d", call_index, thread_id, msg_id);
		return NULL;
	}

	if((_msg_call[call_index].thread_id != INVALID_THREAD_ID) && (_msg_call[call_index].thread_id != thread_id))
	{
		THREADABNOR("thread_id is change:%d<%s>->%d msg_id:%d",
			_msg_call[call_index].thread_id, get_thread_name(_msg_call[call_index].thread_id),
			thread_id, msg_id);

		return NULL;
	}

	_msg_call[call_index].thread_id = thread_id;	

	return _thread_msg_call(&_msg_call[call_index], msg_id);
}

#endif

