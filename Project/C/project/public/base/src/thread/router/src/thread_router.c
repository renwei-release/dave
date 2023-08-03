/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "base_define.h"
#include "base_tools.h"
#include "thread_router.h"
#include "thread_tools.h"
#include "thread_gid_table.h"
#include "thread_orchestration.h"
#include "thread_log.h"

static inline dave_bool
_thread_router_valid(ThreadRouter *pRouter)
{
	if((pRouter == NULL) || (pRouter->uid[0] == '\0'))
		return dave_false;
	else
		return dave_true;
}

static inline s8 *
_thread_router_show(s8 *msg, ThreadRouter *pRouter)
{
#ifdef LEVEL_PRODUCT_alpha
	ub info_index, info_len = 8192;
	ub router_index;

	if(pRouter->router_info != NULL)
	{
		dave_free(pRouter->router_info);
		pRouter->router_info = NULL;
	}
	pRouter->router_info = dave_malloc(info_len);

	info_index = 0;

	info_index += dave_snprintf(
		&pRouter->router_info[info_index], info_len-info_index,
		"%s uid:%s router:%d/%d\n",
		msg,
		pRouter->uid, pRouter->current_router_index, pRouter->router_number);

	for(router_index=0; (router_index<DAVE_ROUTER_SUB_MAX)&&(router_index<pRouter->router_number); router_index++)
	{
		if(router_index > 0)
		{
			info_index += dave_snprintf(
				&pRouter->router_info[info_index], info_len-info_index,
				"\n");
		}

		info_index += dave_snprintf(
			&pRouter->router_info[info_index], info_len-info_index,
			"\tthread:%s gid:%s",
			pRouter->sub_router[router_index].thread, pRouter->sub_router[router_index].gid);
	}

	return pRouter->router_info;
#else
	return NULL;
#endif
}

static inline void
_thread_router_reset(ThreadRouter *pRouter)
{
	if(pRouter != NULL)
	{
		pRouter->uid[0] = '\0';
		pRouter->router_number = 0;
		pRouter->current_router_index = 0;

		pRouter->router_info = NULL;
	}
}

static inline ThreadRouter *
_thread_router_malloc(void)
{
	ThreadRouter *pRouter = dave_malloc(sizeof(ThreadRouter));

	_thread_router_reset(pRouter);

	return pRouter;
}

static inline void
_thread_router_free(ThreadRouter *pRouter)
{
	if(pRouter != NULL)
	{
		if(pRouter->router_info != NULL)
		{
			dave_free(pRouter->router_info);
			pRouter->router_info = NULL;
		}
	
		dave_free(pRouter);
	}
}

static inline void
_thread_router_copy(ThreadRouter *pDst, ThreadRouter *pSrc)
{
	ub sub_index;

	dave_strcpy(pDst->uid, pSrc->uid, sizeof(pDst->uid));

	if(pSrc->router_number > DAVE_ROUTER_SUB_MAX)
	{
		THREADABNOR("invalid uid:%s router_number:%d",
			pSrc->uid, pSrc->router_number);
		pSrc->router_number = DAVE_ROUTER_SUB_MAX;
	}
	pDst->router_number = pSrc->router_number;
	pDst->current_router_index = pSrc->current_router_index;

	for(sub_index=0; sub_index<pSrc->router_number; sub_index++)
	{
		dave_strcpy(pDst->sub_router[sub_index].gid, pSrc->sub_router[sub_index].gid, sizeof(pDst->sub_router[sub_index].gid));
		dave_strcpy(pDst->sub_router[sub_index].thread, pSrc->sub_router[sub_index].thread, sizeof(pDst->sub_router[sub_index].thread));
	}
}

static inline ThreadId
_thread_router_current_thread_id(ThreadRouter *pRouter, s8 *fun, ub line)
{
	ub current_router_index;
	s8 *gid, *thread;

	if((pRouter->router_number == 0)
		|| (pRouter->router_number > DAVE_ROUTER_SUB_MAX))
	{
		THREADABNOR("%s <%s:%d>", _thread_router_show("invalid router_number", pRouter), fun, line);
		return INVALID_THREAD_ID;
	}

	if((pRouter->current_router_index >= pRouter->router_number)
		|| (pRouter->current_router_index >= DAVE_ROUTER_SUB_MAX))
	{
		THREADLOG("Has the end of the router:%s, the last route is now given! <%s:%d>",
			_thread_router_show("", pRouter),
			fun, line);
		current_router_index = pRouter->router_number - 1;
	}
	else
	{
		current_router_index = pRouter->current_router_index;
	}

	gid = pRouter->sub_router[current_router_index].gid;
	thread = pRouter->sub_router[current_router_index].thread;

	if((gid[0] != '\0') && (thread[0] != '\0'))
	{
		return thread_gid_table_inq(gid, thread);
	}
	else if(thread[0] != '\0')
	{
		return thread_id(thread);
	}

	return INVALID_THREAD_ID;
}

static inline void
_thread_router_next_route(ThreadRouter *pRouter)
{
	if(pRouter->current_router_index >= pRouter->router_number)
	{
		return;
	}

	pRouter->current_router_index ++;
}

static inline void *
_thread_sub_router_to_bson(ub router_number, ThreadSubRouter *pSubRouter)
{
	void *pArray, *pSubRouterBson;
	ub router_index;

	pArray = t_bson_malloc_array();

	for(router_index=0; router_index<router_number; router_index++)
	{
		pSubRouterBson = t_bson_malloc_object();

		t_bson_add_string(pSubRouterBson, "1", pSubRouter[router_index].thread);
		if(pSubRouter[router_index].gid[0] != '\0')
		{
			t_bson_add_string(pSubRouterBson, "2", pSubRouter[router_index].gid);
		}

		t_bson_array_add_object(pArray, pSubRouterBson);
	}

	return pArray;
}

static inline ub
_thread_bson_to_sub_router(ThreadSubRouter *pSubRouter, void *pArray)
{
	ub array_len = (ub)t_bson_array_number(pArray);
	ub array_index;
	void *pSubRouterBson;

	for(array_index=0; array_index<array_len; array_index++)
	{
		pSubRouterBson = t_bson_array_inq_object(pArray, array_index);
		if(pSubRouterBson == NULL)
			break;

		t_bson_cpy_string(pSubRouterBson, "1", pSubRouter[array_index].thread, sizeof(pSubRouter[array_index].thread));
		t_bson_cpy_string(pSubRouterBson, "2", pSubRouter[array_index].gid, sizeof(pSubRouter[array_index].gid));
	}

	return array_index;
}

// =====================================================================

void
thread_router_reset(ThreadRouter *pRouter)
{
	_thread_router_reset(pRouter);
}

void
thread_router_free(ThreadRouter *pRouter)
{
	_thread_router_free(pRouter);
}

ThreadId
__thread_router_build_router__(ThreadRouter **ppRouter, s8 *uid, s8 *fun, ub line)
{
	ThreadRouter *pThreadRouter, *pRouter;
	ThreadId thread_id;
	dave_bool load_thread_router;

	*ppRouter = NULL;

	pThreadRouter = thread_current_router();
	if(pThreadRouter == NULL)
	{
		THREADABNOR("pThreadRouter is NULL! uid:%s", uid);
		return INVALID_THREAD_ID;
	}

	pRouter = _thread_router_malloc();

	load_thread_router = _thread_router_valid(pThreadRouter);
	if(load_thread_router == dave_true)
	{
		if(dave_strcmp(uid, pThreadRouter->uid) == dave_false)
		{
			THREADLOG("uid:%s->%s is changed, load new router!", pThreadRouter->uid, uid);
			load_thread_router = dave_false;
		}
		else
		{
			_thread_router_copy(pRouter, pThreadRouter);
		}
	}

	if(load_thread_router == dave_false)
	{
		if(thread_orchestration_router(pRouter, uid) == dave_false)
		{
			_thread_router_free(pRouter);
			pRouter = NULL;
		}
	}

	if(pRouter == NULL)
	{
		return INVALID_THREAD_ID;
	}

	THREADDEBUG("%s pThreadRouter:%lx uid:%s \npThreadRouter:%s \npRouter:%s <%s:%d>",
		load_thread_router==dave_true?"from thread":"from or",
		pThreadRouter,
		uid,
		_thread_router_show("", pThreadRouter),
		_thread_router_show("", pRouter),
		fun, line);

	thread_id = _thread_router_current_thread_id(pRouter, fun, line);

	if(thread_id == INVALID_THREAD_ID)
	{
		_thread_router_free(pRouter);
	}
	else
	{
		_thread_router_next_route(pRouter);

		if(load_thread_router == dave_true)
		{
			_thread_router_next_route(pThreadRouter);
		}
		else
		{
			_thread_router_copy(pThreadRouter, pRouter);
		}

		*ppRouter = pRouter;
	}

	return thread_id;
}

ThreadRouter *
thread_router_build_msg(void *msg_router, ub msg_id)
{
	ThreadRouter *pRouter;
	ThreadRouter *pThreadRouter;

	if(msg_router != NULL)
	{
		return (ThreadRouter *)msg_router;
	}

	if(thread_internal_msg(msg_id) == dave_true)
	{
		return NULL;
	}

	pThreadRouter = thread_current_router();
	if(pThreadRouter == NULL)
	{
		THREADDEBUG("pThreadRouter is NULL!");
		return NULL;
	}

	if(_thread_router_valid(pThreadRouter) == dave_false)
	{
		return NULL;
	}

	pRouter = _thread_router_malloc();

	_thread_router_copy(pRouter, pThreadRouter);

	return pRouter;
}

void
thread_router_clean_msg(MSGBODY *msg)
{
	if(msg->msg_router != NULL)
	{
		_thread_router_free((ThreadRouter *)(msg->msg_router));
		msg->msg_router = NULL;
	}
}

ThreadId
__thread_router_pop_msg__(ThreadRouter **ppRouter, s8 *uid, s8 *fun, ub line)
{
	ThreadRouter *pRouter;
	ThreadId thread_id = INVALID_THREAD_ID;

	*ppRouter = NULL;

	pRouter = _thread_router_malloc();

	if(thread_orchestration_router(pRouter, uid) == dave_true)
	{
		thread_id = _thread_router_current_thread_id(pRouter, fun, line);
	}

	if(thread_id == INVALID_THREAD_ID)
	{
		_thread_router_free(pRouter);
	}
	else
	{
		*ppRouter = pRouter;
	}

	return thread_id;
}

ThreadRouter *
thread_router_run_msg(MSGBODY *msg)
{
	ThreadRouter *pMsgRouter = (ThreadRouter *)(msg->msg_router);
	ThreadRouter *pThreadRouter;

	if(thread_internal_msg(msg->msg_id) == dave_true)
	{
		return NULL;
	}

	if(_thread_router_valid(pMsgRouter) == dave_false)
	{
		return NULL;
	}

	pThreadRouter = thread_current_router();
	if(pThreadRouter == NULL)
	{
		return NULL;
	}

	THREADDEBUG("pThreadRouter:%lx uid:%s %s->%s:%s \npThreadRouter:%s \npMsgRouter:%s",
		pThreadRouter, pThreadRouter->uid,
		thread_name(msg->msg_src), thread_name(msg->msg_dst), msgstr(msg->msg_id),
		_thread_router_show("", pThreadRouter),
		_thread_router_show("", pMsgRouter));

	_thread_router_copy(pThreadRouter, pMsgRouter);

	return pThreadRouter;
}

void
thread_router_run_clean(ThreadRouter *pThreadRouter)
{
	_thread_router_reset(pThreadRouter);
}

ThreadId
__thread_router_check_uid__(s8 *uid, s8 *fun, ub line)
{
	ThreadRouter *pRouter;
	ThreadId thread_id = INVALID_THREAD_ID;

	pRouter = _thread_router_malloc();

	if(thread_orchestration_router(pRouter, uid) == dave_true)
	{
		thread_id = _thread_router_current_thread_id(pRouter, fun, line);
	}

	_thread_router_free(pRouter);

	return thread_id;
}

void *
thread_router_to_bson(ThreadRouter *pRouter)
{
	void *pBson;

	if(pRouter == NULL)
		return NULL;

	if(pRouter->router_number > DAVE_ROUTER_SUB_MAX)
	{
		THREADABNOR("invalid router_number:%d", pRouter->router_number);
		pRouter->router_number = DAVE_ROUTER_SUB_MAX;
	}
	if(pRouter->current_router_index > pRouter->router_number)
	{
		THREADABNOR("invalid current_router_index:%d/%d", pRouter->current_router_index, pRouter->router_number);
		pRouter->router_number = pRouter->router_number;
	}

	pBson = t_bson_malloc_object();

	t_bson_add_string(pBson, "uid", pRouter->uid);
	t_bson_add_int64(pBson, "router_number", pRouter->router_number);
	t_bson_add_int64(pBson, "current_router_index", pRouter->current_router_index);
	t_bson_add_object(pBson, "sub_router", _thread_sub_router_to_bson(pRouter->router_number, pRouter->sub_router));

	return pBson;
}

ThreadRouter *
thread_bson_to_router(void *pBson)
{
	ThreadRouter *pRouter;
	ub router_number;

	if(pBson == NULL)
		return NULL;

	pRouter = _thread_router_malloc();

	t_bson_cpy_string(pBson, "uid", pRouter->uid, sizeof(pRouter->uid));
	t_bson_inq_int64(pBson, "router_number", &(pRouter->router_number));
	t_bson_inq_int64(pBson, "current_router_index", &(pRouter->current_router_index));
	router_number = _thread_bson_to_sub_router(pRouter->sub_router, t_bson_inq_object(pBson, "sub_router"));

	if(pRouter->router_number > DAVE_ROUTER_SUB_MAX)
	{
		THREADABNOR("invalid router_number:%d", pRouter->router_number);
		pRouter->router_number = DAVE_ROUTER_SUB_MAX;
	}
	if(pRouter->current_router_index > pRouter->router_number)
	{
		THREADABNOR("invalid current_router_index:%d/%d", pRouter->current_router_index, pRouter->router_number);
		pRouter->current_router_index = pRouter->router_number;
	}
	if(router_number != pRouter->router_number)
	{
		THREADABNOR("router_number:%d %d/%d mismatch!",
			router_number,
			pRouter->current_router_index, pRouter->router_number);
		pRouter->router_number = router_number;
		if(pRouter->current_router_index > pRouter->router_number)
		{
			pRouter->current_router_index = pRouter->router_number;
		}
	}

	return pRouter;
}

#endif

