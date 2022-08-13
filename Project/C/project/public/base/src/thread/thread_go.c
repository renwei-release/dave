/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "thread_parameter.h"
#include "thread_struct.h"
#include "thread_lock.h"
#include "thread_mem.h"
#include "thread_tools.h"
#include "thread_coroutine.h"
#include "thread_gid_table.h"
#include "thread_msg_buffer.h"
#include "thread_log.h"

static inline dave_bool
_thread_go_is_ready(ThreadId *dst_id, s8 *gid, s8 *dst_thread)
{
	if((gid != NULL) && (dst_thread != NULL))
	{
		*dst_id = thread_gid_table_inq(gid, dst_thread);
	}
	else
	{
		*dst_id = thread_id(dst_thread);
	}

	if(*dst_id == INVALID_THREAD_ID)
	{
		return dave_false;
	}

	return dave_true;
}

static inline void *
_thread_ready_go_msg(
	ThreadStruct *pSrcThread,
	ThreadId dst_id,
	ub req_id, ub req_len, u8 *req_body,
	ub rsp_id,
	s8 *fun, ub line)
{
	ThreadId src_id = pSrcThread->thread_id;
	void *coroutine_site;
	void *rsp_body = NULL;

	coroutine_site = thread_coroutine_running_step_setup(
		pSrcThread,
		&src_id,
		req_id, req_body,
		rsp_id, NULL, 0);

	if(base_thread_id_msg(
		NULL,
		src_id, dst_id,
		BaseMsgType_Unicast,
		req_id, req_len, req_body,
		0,
		fun, line) == dave_true)
	{
		rsp_body = thread_coroutine_running_step_yield(coroutine_site);
	}

	if(rsp_body == NULL)
	{
		THREADLTRACE(60,1,"thread:%s msg_id:%s go failed! <%s:%d>",
			pSrcThread->thread_name, msgstr(req_id),
			fun, line);
	}

	return rsp_body;	
}

static inline void *
_thread_no_ready_go_msg(
	ThreadStruct *pSrcThread,
	s8 *gid, s8 *dst_thread,
	ub req_id, ub req_len, u8 *req_body,
	ub rsp_id,
	s8 *fun, ub line)
{
	ThreadId src_id;
	void *coroutine_site;
	void *rsp_body = NULL;

	coroutine_site = thread_coroutine_running_step_setup(
		pSrcThread,
		&src_id,
		req_id, req_body,
		rsp_id, NULL, 0);

	if((gid != NULL) && (dst_thread != NULL))
	{
		if(thread_msg_buffer_gid_push(
			src_id, gid, dst_thread,
			BaseMsgType_Unicast,
			req_id, req_len, req_body,
			fun, line) == dave_true)
		{
			rsp_body = thread_coroutine_running_step_yield(coroutine_site);
		}
	}
	else if(dst_thread != NULL)
	{
		if(thread_msg_buffer_thread_push(
			src_id, dst_thread,
			BaseMsgType_Unicast,
			req_id, req_len, req_body,
			fun, line) == dave_true)
		{
			rsp_body = thread_coroutine_running_step_yield(coroutine_site);
		}
	}

	if(rsp_body == NULL)
	{
		THREADLTRACE(60,1,"thread:%s msg_id:%s sync failed! <%s:%d>",
			pSrcThread->thread_name, msgstr(req_id),
			fun, line);
	}

	return rsp_body;	
}

// =====================================================================

void *
thread_go_msg(
	ThreadStruct *pSrcThread,
	s8 *gid, s8 *dst_thread,
	ub req_id, ub req_len, u8 *req_body,
	ub rsp_id,
	s8 *fun, ub line)
{
	ThreadId dst_id;

	if(thread_enable_coroutine(pSrcThread) == dave_false)
	{
		THREADABNOR("%s disable coroutine! gid:%s dst_thread:%s req_id:%s req_len:%d rsp_id:%s <%s:%d>",
			pSrcThread->thread_name, gid, dst_thread,
			msgstr(req_id), req_len, msgstr(rsp_id),
			fun, line);
		return NULL;
	}

	if(_thread_go_is_ready(&dst_id, gid, dst_thread) == dave_true)
	{
		return _thread_ready_go_msg(
			pSrcThread,
			dst_id,
			req_id, req_len, req_body,
			rsp_id,
			fun, line);
	}
	else
	{
		return _thread_no_ready_go_msg(
			pSrcThread,
			gid, dst_thread,
			req_id, req_len, req_body,
			rsp_id,
			fun, line);
	}
}

void *
thread_go_id(
	ThreadStruct *pSrcThread,
	ThreadId dst_id,
	ub req_id, ub req_len, u8 *req_body,
	ub rsp_id,
	s8 *fun, ub line)
{
	s8 *dst_thread;

	if(thread_enable_coroutine(pSrcThread) == dave_false)
	{
		THREADABNOR("%s disable coroutine! dst_id:%lx req_id:%s req_len:%d rsp_id:%s <%s:%d>",
			pSrcThread->thread_name, dst_id,
			msgstr(req_id), req_len, msgstr(rsp_id),
			fun, line);
		return NULL;
	}

	dst_thread = thread_name(dst_id);
	if(dave_strcmp(dst_thread, "NULL") == dave_true)
	{
		THREADABNOR("dst_id:%lx not ready! src:%s req_id:%s req_len:%d rsp_id:%s <%s:%d>",
			dst_id, pSrcThread->thread_name,
			msgstr(req_id), req_len, msgstr(rsp_id),
			fun, line);		
		return NULL;
	}

	return _thread_ready_go_msg(
		pSrcThread,
		dst_id,
		req_id, req_len, req_body,
		rsp_id,
		fun, line);
}

#endif

