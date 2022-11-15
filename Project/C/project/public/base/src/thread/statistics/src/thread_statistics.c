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
#include "thread_id.h"
#include "thread_guardian.h"
#include "thread_log.h"

typedef struct {
	ThreadId msg_src;
	ThreadId msg_dst;
	ub msg_id;

	ub msg_counter;
} ThreadStatistics;

static TLock _statistics_pv;
static void *_statistics_kv = NULL;
static dave_bool _statistics_enable = dave_false;

static inline ThreadStatistics *
_thread_statistics_malloc(MSGBODY *pMsg)
{
	ThreadStatistics *pStatistics = dave_ralloc(sizeof(ThreadStatistics));

	pStatistics->msg_src = thread_get_local(pMsg->msg_src);
	pStatistics->msg_dst = thread_get_local(pMsg->msg_dst);
	pStatistics->msg_id = pMsg->msg_id;

	pStatistics->msg_counter = 0;

	return pStatistics;
}

static inline void
_thread_statistics_free(ThreadStatistics *pStatistics)
{
	dave_free(pStatistics);
}

static inline ub
_thread_statistics_msg_key(MSGBODY *pMsg)
{
	return (thread_get_local(pMsg->msg_src) << 48) | (thread_get_local(pMsg->msg_dst) << 32) | (pMsg->msg_id);
}

static RetCode
_thread_statistics_recycle(void *ramkv, s8 *key)
{
	ThreadStatistics *pStatistics = kv_del_key_ptr(ramkv, key);

	if(pStatistics == NULL)
		return RetCode_empty_data;

	_thread_statistics_free(pStatistics);

	return RetCode_OK;
}

static void
_thread_statistics_enable(void)
{
	SAFECODEv1(_statistics_pv, {
		if(_statistics_kv == NULL)
		{
			_statistics_kv = kv_malloc("ts", KvAttrib_list, 0, NULL);
		}
	} );

	_statistics_enable = dave_true;
}

static void
_thread_statistics_disable(void)
{
	_statistics_enable = dave_false;

	SAFECODEv1(_statistics_pv, {
		if(_statistics_kv != NULL)
		{
			kv_free(_statistics_kv, _thread_statistics_recycle);
			_statistics_kv = NULL;
		}
	} );
}

// =====================================================================

void
thread_statistics_init(void)
{
	t_lock_reset(&_statistics_pv);

	_statistics_kv = NULL;
	_statistics_enable = dave_false;
}

void
thread_statistics_exit(void)
{
	_thread_statistics_disable();
}

ub
thread_statistics_enable(s8 *msg_ptr, ub msg_len)
{
	_thread_statistics_enable();

	return dave_snprintf(msg_ptr, msg_len, "THREAD STATISTICS ENABLE!\n");
}

ub
thread_statistics_disable(s8 *msg_ptr, ub msg_len)
{
	_thread_statistics_disable();

	return dave_snprintf(msg_ptr, msg_len, "THREAD STATISTICS DISABLE!\n");
}

ub
thread_statistics_info(s8 *msg_ptr, ub msg_len)
{
	ub msg_index, index;
	ThreadStatistics *pStatistics;

	msg_index = 0;

	if(_statistics_kv == NULL)
	{
		msg_index += dave_snprintf(&msg_ptr[msg_index], msg_len-msg_index, "THREAD STATISTICS DISABLE!");
	}
	else
	{
		msg_index += dave_snprintf(&msg_ptr[msg_index], msg_len-msg_index, "THREAD STATISTICS INFO:\n");

		for(index=0; index<102400; index++)
		{
			pStatistics = kv_index_key_ptr(_statistics_kv, index);
			if(pStatistics == NULL)
				break;

			if(index > 0)
			{
				msg_index += dave_snprintf(&msg_ptr[msg_index], msg_len-msg_index, "\n");
			}

			msg_index += dave_snprintf(&msg_ptr[msg_index], msg_len-msg_index,
				" %s->%s:%s msg_counter:%d",
				thread_name(pStatistics->msg_src),
				thread_name(pStatistics->msg_dst),
				msgstr(pStatistics->msg_id),
				pStatistics->msg_counter);
		}
	}

	return msg_index;
}

ub
thread_statistics_start_msg(MSGBODY *pMsg)
{
	if(_statistics_enable == dave_false)
		return 0;

	ub msg_key;
	ThreadStatistics *pStatistics;

	msg_key = _thread_statistics_msg_key(pMsg);

	SAFECODEv1(_statistics_pv, {

		pStatistics = kv_inq_ub_ptr(_statistics_kv, msg_key);
		if(pStatistics == NULL)
		{
			pStatistics = _thread_statistics_malloc(pMsg);

			kv_add_ub_ptr(_statistics_kv, msg_key, pStatistics);
		}

		pStatistics->msg_counter ++;

	} );

	return dave_os_time_ns();
}

void
thread_statistics_end_msg(ub run_time, ThreadStruct *pThread, MSGBODY *msg)
{
	if(_statistics_enable == dave_false)
		return;
}

#endif

