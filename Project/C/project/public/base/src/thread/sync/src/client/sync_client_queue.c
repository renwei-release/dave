/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_CLIENT)
#include "dave_tools.h"
#include "base_tools.h"
#include "thread_tools.h"
#include "sync_client_data.h"
#include "sync_client_run.h"
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

	if(id_msg(_queue_server_thread, MSGID_QUEUE_UPLOAD_MESSAGE_REQ, pReq) == dave_false)
	{
		SYNCLTRACE(60, 1, "%s->%s:%s send it over a link channel!", src, dst, msgstr(msg_id));
		return dave_false;
	}

	return dave_true;
}

// =====================================================================

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

