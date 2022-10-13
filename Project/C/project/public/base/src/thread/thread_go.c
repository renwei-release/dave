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

static inline ThreadId
_thread_go_name_is_ready(s8 *dst_thread)
{
	return thread_id(dst_thread);
}

static inline ThreadId
_thread_go_gid_is_ready(s8 *gid, s8 *dst_thread)
{
	return thread_gid_table_inq(gid, dst_thread);
}

static inline void *
_thread_ready_go_msg(
	void *msg_router,
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
		NULL, msg_router,
		NULL, NULL,
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
	s8 *gid, s8 *dst_thread, s8 *uid,
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
	else if(uid != NULL)
	{
		if(thread_msg_buffer_uid_push(
			src_id, uid,
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
thread_go_id(
	ThreadStruct *pSrcThread,
	ThreadId dst_id,
	ub req_id, ub req_len, u8 *req_body,
	ub rsp_id,
	s8 *fun, ub line)
{
	if(pSrcThread == NULL)
	{
		THREADLOG("invalid param, pSrcThread:%lx",
			pSrcThread);
		thread_clean_user_input_data(req_body, req_id);
		return NULL;
	}

	if(pSrcThread->has_initialization == dave_false)
	{
		THREADLOG("This interface(%s->%s) is not allowed before initialization is not completed. <%s:%d>",
			msgstr(req_id), msgstr(rsp_id), 
			fun, line);
		return NULL;
	}

	if(thread_enable_coroutine(pSrcThread) == dave_false)
	{
		THREADABNOR("%s disable coroutine! dst_id:%lx req_id:%s req_len:%d rsp_id:%s <%s:%d>",
			pSrcThread->thread_name, dst_id,
			msgstr(req_id), req_len, msgstr(rsp_id),
			fun, line);
		thread_clean_user_input_data(req_body, req_id);
		return NULL;
	}

	if(dave_strcmp(thread_name(dst_id), "NULL") == dave_true)
	{
		THREADABNOR("dst_id:%lx not ready! src:%s req_id:%s req_len:%d rsp_id:%s <%s:%d>",
			dst_id, pSrcThread->thread_name,
			msgstr(req_id), req_len, msgstr(rsp_id),
			fun, line);
		return NULL;
	}

	return _thread_ready_go_msg(
		NULL,
		pSrcThread,
		dst_id,
		req_id, req_len, req_body,
		rsp_id,
		fun, line);
}

void *
thread_go_name(
	ThreadStruct *pSrcThread,
	s8 *dst_thread,
	ub req_id, ub req_len, u8 *req_body,
	ub rsp_id,
	s8 *fun, ub line)
{
	ThreadId dst_id;

	if((pSrcThread == NULL) || (dst_thread == NULL))
	{
		THREADLOG("invalid param, pSrcThread:%lx dst_thread:%lx <%s:%d>",
			pSrcThread, dst_thread,
			fun, line);
		thread_clean_user_input_data(req_body, req_id);
		return NULL;
	}

	if(pSrcThread->has_initialization == dave_false)
	{
		THREADLOG("This interface(%s->%s) is not allowed before initialization is not completed. <%s:%d>",
			msgstr(req_id), msgstr(rsp_id), 
			fun, line);
		return NULL;
	}

	if(thread_enable_coroutine(pSrcThread) == dave_false)
	{
		THREADABNOR("%s disable coroutine! dst_thread:%s req_id:%s req_len:%d rsp_id:%s <%s:%d>",
			pSrcThread->thread_name, dst_thread,
			msgstr(req_id), req_len, msgstr(rsp_id),
			fun, line);
		thread_clean_user_input_data(req_body, req_id);
		return NULL;
	}

	dst_id = _thread_go_name_is_ready(dst_thread);
	if(dst_id != INVALID_THREAD_ID)
	{
		return _thread_ready_go_msg(
			NULL,
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
			NULL, dst_thread, NULL,
			req_id, req_len, req_body,
			rsp_id,
			fun, line);
	}
}

void *
thread_go_gid(
	ThreadStruct *pSrcThread,
	s8 *gid, s8 *dst_thread,
	ub req_id, ub req_len, u8 *req_body,
	ub rsp_id,
	s8 *fun, ub line)
{
	ThreadId dst_id;

	if((pSrcThread == NULL) || (gid == NULL) || (dst_thread == NULL))
	{
		THREADLOG("invalid param, pSrcThread:%lx gid:%lx dst_thread:%lx <%s:%d>",
			pSrcThread, gid, dst_thread,
			fun, line);
		thread_clean_user_input_data(req_body, req_id);
		return NULL;
	}

	if(pSrcThread->has_initialization == dave_false)
	{
		THREADLOG("This interface(%s->%s) is not allowed before initialization is not completed. <%s:%d>",
			msgstr(req_id), msgstr(rsp_id), 
			fun, line);
		return NULL;
	}

	if(thread_enable_coroutine(pSrcThread) == dave_false)
	{
		THREADABNOR("%s disable coroutine! gid:%s dst_thread:%s req_id:%s req_len:%d rsp_id:%s <%s:%d>",
			pSrcThread->thread_name, gid, dst_thread,
			msgstr(req_id), req_len, msgstr(rsp_id),
			fun, line);
		thread_clean_user_input_data(req_body, req_id);
		return NULL;
	}

	dst_id = _thread_go_gid_is_ready(gid, dst_thread);
	if(dst_id != INVALID_THREAD_ID)
	{
		return _thread_ready_go_msg(
			NULL,
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
			gid, dst_thread, NULL,
			req_id, req_len, req_body,
			rsp_id,
			fun, line);
	}
}

void *
thread_go_uid(
	ThreadStruct *pSrcThread,
	s8 *uid,
	ub req_id, ub req_len, u8 *req_body,
	ub rsp_id,
	s8 *fun, ub line)
{
	ThreadRouter *pRouter;
	ThreadId dst_id;

	if((pSrcThread == NULL) || (uid == NULL))
	{
		THREADLOG("invalid param, pSrcThread:%lx uid:%lx <%s:%d>",
			pSrcThread, uid,
			fun, line);
		thread_clean_user_input_data(req_body, req_id);
		return NULL;
	}

	if(pSrcThread->has_initialization == dave_false)
	{
		THREADLOG("This interface(%s->%s) is not allowed before initialization is not completed. <%s:%d>",
			msgstr(req_id), msgstr(rsp_id), 
			fun, line);
		return NULL;
	}

	if(thread_enable_coroutine(pSrcThread) == dave_false)
	{
		THREADABNOR("%s disable coroutine! uid:%s req_id:%s req_len:%d rsp_id:%s <%s:%d>",
			pSrcThread->thread_name, uid,
			msgstr(req_id), req_len, msgstr(rsp_id),
			fun, line);
		thread_clean_user_input_data(req_body, req_id);
		return NULL;
	}

	dst_id = __thread_router_build_router__(&pRouter, uid, fun, line);

	if(dst_id != INVALID_THREAD_ID)
	{
		return _thread_ready_go_msg(
			pRouter,
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
			NULL, NULL, uid,
			req_id, req_len, req_body,
			rsp_id,
			fun, line);
	}
}

#endif

