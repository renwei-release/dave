/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_parameter.h"
#if defined(__DAVE_BASE__) && defined(ENABLE_THREAD_COROUTINE)
#include "dave_base.h"
#include "dave_os.h"
#include "thread_struct.h"
#include "thread_mem.h"
#include "thread_thread.h"
#include "thread_tools.h"
#include "thread_coroutine.h"
#include "coroutine_core.h"
#include "thread_log.h"

#define COROUTINE_WAIT_TIMER 180
#define COROUTINE_DELAY_RELEASE_TIMER 3

typedef enum {
	wakeupevent_get_msg,
	wakeupevent_timer_out,
} wakeupevent;

typedef struct {
	ub msg_id;
	void *msg_body;
	ub msg_len;

	void *next;
} CoMsgList;

typedef struct {
	ThreadStruct *pThread;
	coroutine_thread_fun coroutine_fun;
	base_thread_fun thread_fun;
	MSGBODY msg;
	void *co;

	ThreadId src_thread;
	ub msg_id;
	ub msg_site;
	ub wakeup_index;

	ub thread_index;

	void *rsp_msg_body;
	CoMsgList *rsp_msg_head;

	u8 *user_msg_body;
	ub user_msg_len;
	void *user_msg_ptr;
} CoroutineSite;

static void *_coroutine_kv = NULL;
static void *_delayed_destruction_site_kv = NULL;

static inline ThreadId
_thread_coroutine_set_index(ThreadId thread_id, ub *wakeup_index)
{
	if(thread_thread_self(wakeup_index) == thread_id)
	{
		return thread_set_wakeup(thread_id, *wakeup_index);
	}
	else
	{
		*wakeup_index = 0;

		return thread_set_wakeup(thread_id, *wakeup_index);
	}
}

static inline s8 *
_thread_coroutine_key_kv(
	ub msg_id,
	ub wakeup_index,
	ub msg_site,
	s8 *key_ptr, ub key_len)
{
	dave_snprintf(key_ptr, key_len,
		"%lx-%lx-%lx",
		msg_id,
		wakeup_index,
		msg_site);

	return key_ptr;
}

static inline void
_thread_coroutine_add_kv_(s8 *key, CoroutineSite *pSite)
{
	kv_add_key_ptr(_coroutine_kv, key, pSite);
}

static inline CoroutineSite *
_thread_coroutine_del_kv_(s8 *key)
{
	return (CoroutineSite *)kv_del_key_ptr(_coroutine_kv, key);
}

static inline void
_thread_coroutine_add_kv(CoroutineSite *pSite)
{
	s8 key[256];

	if(_coroutine_kv == NULL)
		return;

	_thread_coroutine_key_kv(
		pSite->msg_id,
		pSite->wakeup_index,
		pSite->msg_site,
		key, sizeof(key));

	THREADDEBUG("msg_id:%s wakeup_index:%d msg_site:%lx key:%s",
		msgstr(pSite->msg_id), pSite->wakeup_index, pSite->msg_site, key);

	_thread_coroutine_add_kv_(key, pSite);
}

static inline CoroutineSite *
_thread_coroutine_del_kv(
	ub msg_id,
	ub wakeup_index,
	ub msg_site)
{
	s8 key[256];

	if(_coroutine_kv == NULL)
		return NULL;

	_thread_coroutine_key_kv(
		msg_id,
		wakeup_index,
		msg_site,
		key, sizeof(key));

	return _thread_coroutine_del_kv_(key);
}

/*
 * 考虑到竞争问题，
 * 我们把与这个线程有关的消息都在该线程自己的消息队列去排队执行。
 */
static inline void
_thread_coroutine_wakeup_me(
	void *msg_chain, void *msg_router,
	CoroutineSite *pSite, wakeupevent event)
{
	CoroutineWakeup *pWakeup;

	pWakeup = thread_msg(pWakeup);

	pWakeup->wakeup_id = event;

	pWakeup->thread_index = pSite->thread_index;
	pWakeup->wakeup_index = pSite->wakeup_index;
	pWakeup->ptr = pSite;

	thread_thread_write(msg_chain, msg_router, pSite->thread_index, pSite->wakeup_index, MSGID_COROUTINE_WAKEUP, pWakeup);
}

static inline void *
_thread_coroutine_push_msg_list(CoroutineSite *pSite, ub msg_id, void *msg_body, ub msg_len)
{
	CoMsgList *pList;

	if((msg_body == NULL) || (msg_len == 0))
		return NULL;

	pList = dave_malloc(sizeof(CoMsgList));

	pList->msg_id = msg_id;
	pList->msg_body = msg_body;
	pList->msg_len = msg_len;

	pList->next = pSite->rsp_msg_head;

	pSite->rsp_msg_head = pList;

	return msg_body;
}

static inline dave_bool
_thread_coroutine_pop_msg_list(CoroutineSite *pSite)
{
	CoMsgList *pList;

	if(pSite->rsp_msg_head == NULL)
		return dave_false;

	pList = pSite->rsp_msg_head;
	pSite->rsp_msg_head = pSite->rsp_msg_head->next;

	thread_clean_user_input_data(pList->msg_body, pList->msg_id);

	dave_free(pList);

	return dave_true;
}

static inline CoroutineSite *
_thread_coroutine_info_malloc(ThreadStruct *pThread, coroutine_thread_fun coroutine_fun, base_thread_fun thread_fun, MSGBODY *msg)
{
	CoroutineSite *pSite = dave_malloc(sizeof(CoroutineSite));

	pSite->pThread = pThread;
	pSite->coroutine_fun = coroutine_fun;
	pSite->thread_fun = thread_fun;
	pSite->msg = *msg;
	pSite->co = NULL;

	pSite->src_thread = INVALID_THREAD_ID;
	pSite->msg_id = MSGID_RESERVED;
	pSite->msg_site = 0;
	pSite->wakeup_index = msg->thread_wakeup_index;

	pSite->thread_index = pThread->thread_index;

	pSite->rsp_msg_head = NULL;

	pSite->user_msg_body = NULL;
	pSite->user_msg_len = 0;
	pSite->user_msg_ptr = NULL;

	msg->mem_state = MsgMemState_captured;

	return pSite;
}

static inline void
_thread_coroutine_info_free(CoroutineSite *pSite)
{
	if(pSite->msg.msg_body != NULL)
	{
		thread_clean_user_input_data(pSite->msg.msg_body, pSite->msg.msg_id);
	}

	if(pSite->co != NULL)
	{
		coroutine_release(pSite->co);
	}

	while(pSite->rsp_msg_head != NULL)
	{
		_thread_coroutine_pop_msg_list(pSite);
	}

	dave_free(pSite);
}

static inline void
_thread_coroutine_running_step_8(void *ramkv, s8 *key)
{
	CoroutineSite *pSite;

	pSite = kv_del_key_ptr(_delayed_destruction_site_kv, key);

	if(pSite != NULL)
	{
		_thread_coroutine_info_free(pSite);
	}
}

static inline void
_thread_coroutine_running_step_7(CoroutineSite *pSite)
{
	kv_add_ub_ptr(_delayed_destruction_site_kv, (ub)pSite, pSite);
}

static inline void
_thread_coroutine_running_step_6(CoroutineWakeup *pWakeup, ub wakeup_index)
{
	CoroutineSite *pSite = (CoroutineSite *)(pWakeup->ptr);

	if(pWakeup->wakeup_index != wakeup_index)
	{
		THREADABNOR("Algorithm error, wakeup_index mismatch:%d/%d",
			pWakeup->wakeup_index, wakeup_index);
	}

	if(coroutine_resume(pSite->co) == dave_false)
	{
		THREADABNOR("resume failed! %s->%s:%s",
			thread_name(pSite->msg.msg_src), thread_name(pSite->msg.msg_dst), msgstr(pSite->msg.msg_id));
	}
}

static inline dave_bool
_thread_coroutine_running_step_5(
	void *msg_chain, void *msg_router,
	ThreadId src_id,
	ThreadStruct *pDstThread, ThreadId dst_id,
	ub msg_id, void *msg_body, ub msg_len)
{
	ub wakeup_index, msg_site;
	CoroutineSite *pSite;

	wakeup_index = thread_get_wakeup(dst_id);
	msg_site = (ub)(t_rpc_ptr(msg_id, msg_body, NULL));

	pSite = _thread_coroutine_del_kv(msg_id, wakeup_index, msg_site);
	if(pSite == NULL)
	{
		return dave_false;
	}

	if((pSite->thread_index != pDstThread->thread_index)
		|| (pSite->msg_id != msg_id)
		|| (pSite->msg_site != msg_site)
		|| (pSite->wakeup_index != wakeup_index))
	{
		THREADABNOR("wait thread mismatch, thread_index:%d/%d msg_id:%s/%s msg_site:%lx/%lx wakeup_index:%ld/%ld",
			pSite->thread_index, pDstThread->thread_index,
			msgstr(pSite->msg_id), msgstr(msg_id),
			pSite->msg_site, msg_site,
			pSite->wakeup_index, wakeup_index);
		return dave_false;
	}

	pSite->rsp_msg_body = _thread_coroutine_push_msg_list(pSite, msg_id, msg_body, msg_len);

	if(pSite->user_msg_body != NULL)
	{
		if(msg_len > pSite->user_msg_len)
		{
			msg_len = pSite->user_msg_len;
		}

		pSite->user_msg_len = dave_memcpy(pSite->user_msg_body, msg_body, msg_len);
	}
	t_rpc_ptr(msg_id, msg_body, pSite->user_msg_ptr);

	_thread_coroutine_wakeup_me(
		msg_chain, msg_router,
		pSite, wakeupevent_get_msg);

	return dave_true;
}

static inline void *
_thread_coroutine_running_step_4(void *param)
{
	CoroutineSite *pSite = (CoroutineSite *)param;

	THREADDEBUG("thread_index:%d wakeup_index:%d wait_msg:%s",
		pSite->thread_index, pSite->wakeup_index, msgstr(pSite->msg_id));

	thread_thread_clean_coroutine_site(pSite->thread_index, pSite->wakeup_index);

	if(coroutine_yield(pSite->co) == dave_false)
	{
		THREADABNOR("yield failed! %s->%s:%s",
			thread_name(pSite->msg.msg_src), thread_name(pSite->msg.msg_dst), msgstr(pSite->msg.msg_id));
	}

	thread_thread_set_coroutine_site(pSite->thread_index, pSite->wakeup_index, pSite);

	THREADDEBUG("wakeup me !!!!!!!!!!!!!!!!");

	return (pSite->user_msg_body != NULL ? pSite->user_msg_body : pSite->rsp_msg_body);
}

static inline void *
_thread_coroutine_running_step_3(
	ThreadStruct *pSrcThread,
	ThreadId *src_id,
	ub req_msg_id, u8 *req_msg_body,
	ub rsp_msg_id, u8 *rsp_msg_body, ub rsp_msg_len)
{
	ub wakeup_index;
	CoroutineSite *pSite;

	*src_id = _thread_coroutine_set_index(pSrcThread->thread_id, &wakeup_index);

	pSite = (CoroutineSite *)thread_thread_get_coroutine_site(pSrcThread->thread_index, wakeup_index);
	if(pSite == NULL)
	{
		THREADABNOR("%s can't find site on msg_id:%s!",
			pSrcThread->thread_name, msgstr(rsp_msg_id));
		return NULL;
	}

	pSite->src_thread = *src_id;
	pSite->msg_id = rsp_msg_id;
	pSite->msg_site = (ub)(pSite);
	pSite->wakeup_index = wakeup_index;

	pSite->thread_index = pSrcThread->thread_index;

	pSite->rsp_msg_body = NULL;

	pSite->user_msg_body = rsp_msg_body;
	pSite->user_msg_len = (pSite->user_msg_body == NULL) ? 0 : rsp_msg_len;
	pSite->user_msg_ptr = t_rpc_ptr(req_msg_id, req_msg_body, (void *)(pSite->msg_site));

	_thread_coroutine_add_kv(pSite);

	return pSite;
}

static inline void *
_thread_coroutine_running_step_2(void *param)
{
	CoroutineSite *pSite = param;

	thread_thread_set_coroutine_site(pSite->thread_index, pSite->wakeup_index, pSite);

	pSite->coroutine_fun(pSite->thread_fun, &(pSite->msg));

	thread_thread_clean_coroutine_site(pSite->thread_index, pSite->wakeup_index);

	_thread_coroutine_running_step_7(pSite);

	return NULL;
}

static inline void
_thread_coroutine_running_step_1(ThreadStruct *pThread, coroutine_thread_fun coroutine_fun, base_thread_fun thread_fun, MSGBODY *msg)
{
	CoroutineSite *pSite = _thread_coroutine_info_malloc(pThread, coroutine_fun, thread_fun, msg);

	pSite->co = coroutine_create(_thread_coroutine_running_step_2, pSite, msg);

	if(coroutine_resume(pSite->co) == dave_false)
	{
		THREADABNOR("resume failed! %s->%s:%s",
			thread_name(pSite->msg.msg_src), thread_name(pSite->msg.msg_dst), msgstr(pSite->msg.msg_id));
	}
}

static inline void
_thread_coroutine_wakeup(MSGBODY *thread_msg)
{
	CoroutineWakeup *pWakeup = (CoroutineWakeup *)(thread_msg->msg_body);

	THREADDEBUG("wakeup_id:%d", pWakeup->wakeup_id);

	switch(pWakeup->wakeup_id)
	{
		case wakeupevent_get_msg:
		case wakeupevent_timer_out:
				_thread_coroutine_running_step_6(pWakeup, thread_msg->thread_wakeup_index);
			break;
		default:
			break;
	}
}

static inline void
_thread_coroutine_kv_timer_out(void *ramkv, s8 *key)
{
	CoroutineSite *pSite;

	pSite = _thread_coroutine_del_kv_(key);
	if(pSite != NULL)
	{
		THREADLOG("%s %lx:%lx pSite:%lx co:%lx",
			key,
			pSite->src_thread, pSite->msg_id,
			pSite, pSite->co);
	
		_thread_coroutine_wakeup_me(
			NULL, NULL,
			pSite, wakeupevent_timer_out);
	}
}

static inline dave_bool
_thread_coroutine_thread_can_be_go(ThreadStruct *pThread)
{
	if((pThread->thread_flag & THREAD_THREAD_FLAG) == 0x00)
		return dave_false;

	if(pThread->attrib == REMOTE_TASK_ATTRIB)
		return dave_false;

	return dave_true;
}

static inline dave_bool
_thread_coroutine_msg_can_be_go(MSGBODY *msg)
{
	switch(msg->msg_id)
	{
		case MSGID_WAKEUP:
		case MSGID_RESTART_REQ:
		case MSGID_RESTART_RSP:
		case MSGID_POWER_OFF:
		case MSGID_COROUTINE_WAKEUP:
				return dave_false;
			break;
		default:
				return dave_true;
			break;
	}

	return dave_true;
}

static inline dave_bool
_thread_coroutine_can_be_go(ThreadStruct *pThread, MSGBODY *msg)
{
	if(_thread_coroutine_thread_can_be_go(pThread) == dave_false)
		return dave_false;

	if(_thread_coroutine_msg_can_be_go(msg) == dave_false)
		return dave_false;

	return dave_true;
}

static inline void
_thread_coroutine_booting(void)
{
	if(_coroutine_kv == NULL)
	{
		thread_other_lock();
		if(_coroutine_kv == NULL)
		{
			_coroutine_kv = kv_malloc("ckv", KvAttrib_list, COROUTINE_WAIT_TIMER, _thread_coroutine_kv_timer_out);
		}
		if(_delayed_destruction_site_kv == NULL)
		{
			_delayed_destruction_site_kv = kv_malloc("ddskv", KvAttrib_list, COROUTINE_DELAY_RELEASE_TIMER, _thread_coroutine_running_step_8);
		}
		thread_other_unlock();
	}
}

// =====================================================================

void
thread_coroutine_init(void)
{
	coroutine_core_init();
}

void
thread_coroutine_exit(void)
{
	coroutine_core_exit();
}

void
thread_coroutine_creat(ThreadStruct *pThread)
{
	coroutine_core_creat();

	base_thread_msg_register(pThread->thread_id, MSGID_COROUTINE_WAKEUP, _thread_coroutine_wakeup, NULL);
}

void
thread_coroutine_die(ThreadStruct *pThread)
{
	base_thread_msg_unregister(pThread->thread_id, MSGID_COROUTINE_WAKEUP);

	coroutine_core_die();
}

dave_bool
thread_coroutine_running_step_go(
	ThreadStruct *pThread,
	coroutine_thread_fun coroutine_fun,
	base_thread_fun thread_fun,
	MSGBODY *msg)
{
	if(_thread_coroutine_can_be_go(pThread, msg) == dave_false)
		return dave_false;

	_thread_coroutine_booting();

	_thread_coroutine_running_step_1(pThread, coroutine_fun, thread_fun, msg);

	return dave_true;
}

void *
thread_coroutine_running_step_setup(
	ThreadStruct *pSrcThread,
	ThreadId *src_id,
	ub req_msg_id, u8 *req_msg_body,
	ub rsp_msg_id, u8 *rsp_msg_body, ub rsp_msg_len)
{
	return _thread_coroutine_running_step_3(
		pSrcThread,
		src_id,
		req_msg_id, req_msg_body,
		rsp_msg_id, rsp_msg_body, rsp_msg_len);
}

void *
thread_coroutine_running_step_yield(void *param)
{
	if(param == NULL)
	{
		THREADLOG("empty param!");
		return NULL;
	}

	return _thread_coroutine_running_step_4(param);
}

dave_bool
thread_coroutine_running_step_resume(
	void *msg_chain, void *msg_router,
	ThreadId src_id,
	ThreadStruct *pDstThread, ThreadId dst_id,
	ub msg_id, void *msg_body, ub msg_len)
{
	return _thread_coroutine_running_step_5(
		msg_chain, msg_router,
		src_id,
		pDstThread, dst_id,
		msg_id, msg_body, msg_len);
}

#endif

