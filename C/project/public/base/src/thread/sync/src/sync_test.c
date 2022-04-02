/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_tools.h"
#include "dave_verno.h"
#include "base_timer.h"
#include "thread_sync.h"
#include "sync_param.h"
#include "sync_test.h"
#include "sync_log.h"

static void
_sync_test_echo(ThreadId dst, void *ptr, s8 *msg_ptr, ub msg_len)
{
	DateStruct date;

	t_time_get_date(&date);

	dave_snprintf(msg_ptr, msg_len, "%s echo : hello (%s)",
		dave_verno_my_product(), t_a2b_date_str(&date));
}

static ub
_sync_test_info(ThreadId dst, void *ptr, sync_info_fun info_fun, s8 *msg_ptr, ub msg_len)
{
	ub msg_index = 0;

	if(info_fun != NULL)
	{
		msg_index += info_fun(&msg_ptr[msg_index], msg_len-msg_index);
	}

	return msg_index;
}

// =====================================================================

void
sync_test_req(ThreadId src, DebugReq *pReq, sync_info_fun info_fun)
{
	DebugRsp *pRsp = thread_reset_msg(pRsp);

	SYNCDEBUG("%s", pReq->msg);

    if(pReq->msg[0] == 'i')
	{
		_sync_test_info(src, pReq->ptr, info_fun, pRsp->msg, sizeof(pRsp->msg));
	}
	else
	{
		_sync_test_echo(src, pReq->ptr, pRsp->msg, sizeof(pRsp->msg));
	}

	if(pRsp->msg[0] == '\0')
	{
		dave_snprintf(pRsp->msg, sizeof(pRsp->msg), "info is empty!");
	}

	pRsp->ptr = pReq->ptr;

	write_msg(src, MSGID_DEBUG_RSP, pRsp);
}

void
sync_test_rsp(MSGBODY *pMsg)
{
	/*
	 * 此消息无法被路由，因为只是从本地线程发起的传送，
	 * 该线程并未注册未远端线程。
	 */
	SYNCLOG("You send a message(%lx/%s->%lx/%s) to the remote, \
but the %s of the message was not registered as a remote thread.",
		pMsg->msg_src, thread_name(pMsg->msg_src),
		pMsg->msg_dst, thread_name(pMsg->msg_dst),
		thread_name(pMsg->msg_dst));
}

#endif

