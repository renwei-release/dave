/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_CLIENT)
#include "dave_base.h"
#include "dave_tools.h"
#include "base_tools.h"
#include "thread_tools.h"
#include "sync_client_data.h"
#include "sync_client_run.h"
#include "sync_client_route.h"
#include "sync_param.h"
#include "sync_log.h"

static ThreadId _queue_server_thread = INVALID_THREAD_ID;

static inline dave_bool
_sync_client_queue_can_be_upload(SyncServer *pServer, s8 *src, s8 *dst, ub msg_id)
{
	if(pServer->server_type == SyncServerType_sync_client)
	{
		return dave_false;
	}
	if(pServer->service_statement.support_queue_server == dave_false)
	{
		return dave_false;
	}
	if(_queue_server_thread == INVALID_THREAD_ID)
	{
		_queue_server_thread = thread_id(QUEUE_SERVER_THREAD_NAME);
	}
	if(_queue_server_thread == INVALID_THREAD_ID)
	{
		SYNCLTRACE(60, 1, "%s->%s:%s send it over a link channel!", src, dst, msgstr(msg_id));
		return dave_false;
	}
	if(base_thread_has_initialization(_queue_server_thread) == dave_false)
	{
		return dave_false;
	}

	return dave_true;
}

static inline void
_sync_client_queue_send_msg(
	ThreadId route_src, ThreadId route_dst,
	QueueUploadMsgReq *pReq)
{
	ThreadMsg *pMsg = thread_build_msg(
		_queue_server_thread, REMOTE_TASK_ATTRIB,
		NULL, NULL,
		pReq->src_gid, pReq->src_name,
		thread_get_local(route_src), _queue_server_thread,
		MSGID_QUEUE_UPLOAD_MESSAGE_REQ, sizeof(*pReq), (u8 *)pReq,
		BaseMsgType_Unicast,
		(s8 *)__func__, __LINE__);

	sync_client_message_route(&(pMsg->msg_body));

	thread_clean_msg(pMsg);
}

static inline dave_bool
_sync_client_queue_upload(
	SyncServer *pServer,
	ThreadId route_src, ThreadId route_dst,
	s8 *src, s8 *dst,
	ub msg_id,
	MBUF *data)
{
	QueueUploadMsgReq *pReq = thread_msg(pReq);

	dave_strcpy(pReq->src_name, src, sizeof(pReq->src_name));
	dave_strcpy(pReq->dst_name, dst, sizeof(pReq->dst_name));
	dave_strcpy(pReq->src_gid, globally_identifier(), sizeof(pReq->src_gid));
	if(thread_get_net(route_dst) != SYNC_NET_INDEX_MAX)
		dave_strcpy(pReq->dst_gid, pServer->globally_identifier, sizeof(pReq->dst_gid));
	else
		pReq->dst_gid[0] = '\0';
	pReq->msg_id = msg_id;
	pReq->msg = data;
	pReq->ptr = NULL;

	SYNCDEBUG("%s->%s:%s", src, dst, msgstr(msg_id));

	_sync_client_queue_send_msg(route_src, route_dst, pReq);

	return dave_true;
}

// =====================================================================

BaseMsgType
sync_client_queue_enable(SyncServer *pServer, s8 *src, s8 *dst, ub msg_id, BaseMsgType msg_type)
{
	if((msg_type != BaseMsgType_Unicast) && (msg_type != BaseMsgType_Unicast_queue))
	{
		return msg_type;
	}

	if((msg_type == BaseMsgType_Unicast) && (pServer->server_busy == dave_true))
	{
		msg_type = BaseMsgType_Unicast_queue;
	}

	if(msg_type == BaseMsgType_Unicast_queue)
	{
		if(_sync_client_queue_can_be_upload(pServer, src, dst, msg_id) == dave_false)
		{
			msg_type = BaseMsgType_Unicast;
		}
	}

	return msg_type;
}

dave_bool
sync_client_queue_upload(
	SyncServer *pServer,
	ThreadId route_src, ThreadId route_dst,
	s8 *src, s8 *dst,
	ub msg_id,
	MBUF *data)
{
	if(_sync_client_queue_can_be_upload(pServer, src, dst, msg_id) == dave_false)
		return dave_false;

	return _sync_client_queue_upload(
		pServer,
		route_src, route_dst,
		src, dst,
		msg_id,
		data);
}

void
sync_client_queue_run(QueueRunMsgReq *pReq)
{
	SyncServer *pServer = sync_client_gid_to_server(pReq->src_gid);

	if(pServer == NULL)
	{
		SYNCABNOR("%s:%s->%s:%s %s local not ready, discard message!",
			pReq->src_name, pReq->src_gid, pReq->dst_name, pReq->dst_gid);
	}
	else
	{
		sync_client_run_thread(pServer, mlen(pReq->msg), (u8 *)ms8(pReq->msg));
	}

	dave_mfree(pReq->msg);
}

#endif

