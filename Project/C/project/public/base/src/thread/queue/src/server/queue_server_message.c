/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_queue.h"
#if defined(QUEUE_STACK_SERVER)
#include "dave_base.h"
#include "dave_tools.h"
#include "base_tools.h"
#include "thread_struct.h"
#include "thread_tools.h"
#include "queue_server_map.h"
#include "queue_log.h"

typedef struct {
	s8 name[DAVE_THREAD_NAME_LEN];
	s8 gid[DAVE_GLOBALLY_IDENTIFIER_LEN];

	ThreadQueue *pQueue;
} QueueMessage;

static void *_message_kv = NULL;
static TLock _message_pv;
static ub _out_fo_order_download;

static inline s8 *
_queue_server_message_key(s8 *key_ptr, ub key_len, s8 *dst_name, s8 *dst_gid)
{
	dave_snprintf(key_ptr, key_len, "%s:%s", dst_name, dst_gid);
	return key_ptr;
}

static inline QueueMessage *
_queue_server_message_malloc(s8 *name, s8 *gid)
{
	QueueMessage *pMessage = dave_ralloc(sizeof(QueueMessage));

	dave_strcpy(pMessage->name, name, sizeof(pMessage->name));
	dave_strcpy(pMessage->gid, gid, sizeof(pMessage->gid));
	pMessage->pQueue = dave_ralloc(sizeof(ThreadQueue));

	thread_queue_booting(pMessage->pQueue, 1);

	thread_queue_malloc(pMessage->pQueue, 1);

	return pMessage;
}

static inline void
_queue_server_message_free(QueueMessage *pMessage)
{
	thread_queue_free(pMessage->pQueue, 1);

	dave_free(pMessage->pQueue);

	dave_free(pMessage);
}

static inline QueueMessage *
_queue_server_message_inq(s8 *name, s8 *gid)
{
	s8 key[256];
	QueueMessage *pMessage = NULL;

	_queue_server_message_key(
		key, sizeof(key),
		name, gid);

	pMessage = kv_inq_key_ptr(_message_kv, key);
	if(pMessage == NULL)
	{
		SAFECODEv1(_message_pv, {

			pMessage = _queue_server_message_malloc(name, gid);
			kv_add_key_ptr(_message_kv, key, pMessage);

		});
	}

	return pMessage;
}

static inline ThreadMsg *
_queue_server_message_build_msg(QueueUploadMsgReq *pReq)
{
	ThreadMsg *pMsg;

	pMsg = (ThreadMsg *)dave_malloc(sizeof(ThreadMsg));

	pMsg->msg_body.msg_id = pReq->msg_id;
	pMsg->msg_body.msg_len = 0;
	pMsg->msg_body.msg_body = NULL;

	pMsg->msg_body.user_ptr = NULL;
	pMsg->msg_body.queue_ptr = pReq->msg;

	pMsg->msg_body.msg_chain = pMsg->msg_body.msg_router = NULL;

	dave_strcpy(pMsg->msg_body.src_gid, pReq->src_gid, sizeof(pMsg->msg_body.src_gid));
	dave_strcpy(pMsg->msg_body.dst_gid, pReq->dst_gid, sizeof(pMsg->msg_body.dst_gid));
	dave_strcpy(pMsg->msg_body.src_name, pReq->src_name, sizeof(pMsg->msg_body.src_name));
	dave_strcpy(pMsg->msg_body.dst_name, pReq->dst_name, sizeof(pMsg->msg_body.dst_name));

	pMsg->pQueue = NULL;
	pMsg->next = NULL;

	return pMsg;
}

static inline ThreadMsg *
_queue_server_message_read_msg(ThreadQueue *pQueue)
{
	return thread_queue_read(pQueue);
}

static inline void
_queue_server_message_clean_msg(ThreadMsg *pMsg)
{
	if(pMsg->msg_body.queue_ptr != NULL)
	{
		dave_mfree(pMsg->msg_body.queue_ptr);
		pMsg->msg_body.queue_ptr = NULL;
	}

	dave_free(pMsg);
}

static inline ub
_queue_server_message_number_msg(QueueMessage *pMessage)
{
	return pMessage->pQueue->list_number;
}

static inline void
_queue_server_message_update_client_state(s8 *gid, ThreadMsg *pMsg, QueueMessage *pMessage)
{
	QueueUpdateStateReq *pReq = thread_msg(pReq);

	dave_strcpy(pReq->src_name, pMsg->msg_body.src_name, sizeof(pReq->src_name));
	dave_strcpy(pReq->dst_name, pMsg->msg_body.dst_name, sizeof(pReq->dst_name));
	dave_strcpy(pReq->src_gid, pMsg->msg_body.src_gid, sizeof(pReq->src_gid));
	dave_strcpy(pReq->dst_gid, pMsg->msg_body.dst_gid, sizeof(pReq->dst_gid));
	pReq->msg_id = pMsg->msg_body.msg_id;
	pReq->queue_number = _queue_server_message_number_msg(pMessage);
	dave_strcpy(pReq->queue_gid, globally_identifier(), sizeof(pReq->queue_gid));
	pReq->ptr = NULL;

	gid_msg(gid, QUEUE_CLIENT_THREAD_NAME, MSGID_QUEUE_UPDATE_STATE_REQ, pReq);	
}

static inline void
_queue_server_message_update_map_state(QueueServerMap *pMap, ThreadMsg *pMsg, QueueMessage *pMessage)
{
	ub index;

	for(index=0; index<QUEUE_SERVER_MAP_MAX; index++)
	{
		if(pMap->client_gid[index][0] == '\0')
			break;

		_queue_server_message_update_client_state(pMap->client_gid[index], pMsg, pMessage);
	}
}

static inline void
_queue_server_message_update_state(QueueMessage *pMessage)
{
	ThreadQueue *pQueue = pMessage->pQueue;
	ThreadMsg *pMsg;

	if(pQueue != NULL)
	{
		pMsg = thread_queue_clone(pQueue);
		if(pMsg != NULL)
		{		
			QueueServerMap *pMap = queue_server_map_inq(pMsg->msg_body.dst_name);

			if(pMsg->msg_body.dst_gid[0] != '\0')
				_queue_server_message_update_client_state(pMsg->msg_body.dst_gid, pMsg, pMessage);
			else
				_queue_server_message_update_map_state(pMap, pMsg, pMessage);

			dave_free(pMsg);
		}
	}
}

static inline dave_bool
_queue_server_message_upload(ThreadMsg *pMsg)
{
	QueueMessage *pMessage;
	ub list_number;

	pMessage = _queue_server_message_inq(pMsg->msg_body.dst_name, pMsg->msg_body.dst_gid);

	list_number = thread_queue_write(pMessage->pQueue, pMsg);
	if(list_number > 0)
	{
		if((list_number == 1) || ((list_number % 256) == 0))
		{
			_queue_server_message_update_state(pMessage);
		}
	}

	return dave_true;
}

static inline ThreadMsg *
_queue_server_message_download_(s8 *name, s8 *gid)
{
	QueueMessage *pMessage;

	pMessage = _queue_server_message_inq(name, gid);
	if(pMessage == NULL)
		return NULL;

	return _queue_server_message_read_msg(pMessage->pQueue);
}

static inline ThreadMsg *
_queue_server_message_download(QueueDownloadMsgReq *pReq)
{
	s8 gid[DAVE_GLOBALLY_IDENTIFIER_LEN] = { '\0', '\0' };
	ThreadMsg *pMsg;

	if((_out_fo_order_download ++) % 2 == 0)
	{
		pMsg = _queue_server_message_download_(pReq->name, gid);
		if(pMsg != NULL)
			return pMsg;
		pMsg = _queue_server_message_download_(pReq->name, pReq->gid);
		if(pMsg != NULL)
			return pMsg;		
	}
	else
	{
		pMsg = _queue_server_message_download_(pReq->name, pReq->gid);
		if(pMsg != NULL)
			return pMsg;
		pMsg = _queue_server_message_download_(pReq->name, gid);
		if(pMsg != NULL)
			return pMsg;
	}

	return NULL;
}

static inline void
_queue_server_message_timerout(void *ramkv, s8 *key)
{
	QueueMessage *pMessage = kv_inq_key_ptr(_message_kv, key);

	if(pMessage == NULL)
		return;

	_queue_server_message_update_state(pMessage);
}

static inline RetCode
_queue_server_message_recycle(void *ramkv, s8 *key)
{
	QueueMessage *pMessage = kv_del_key_ptr(_message_kv, key);

	if(pMessage == NULL)
	{
		return RetCode_empty_data;
	}

	_queue_server_message_free(pMessage);

	return RetCode_OK;
}

// =====================================================================

void
queue_server_message_init(void)
{
	_message_kv = kv_malloc("queue-server-message", 5, _queue_server_message_timerout);
	t_lock_reset(&_message_pv);
	_out_fo_order_download = t_rand();
}

void
queue_server_message_exit(void)
{
	kv_free(_message_kv, _queue_server_message_recycle);
}

void
queue_server_message_upload(QueueUploadMsgReq *pReq)
{
	ThreadMsg *pMsg = _queue_server_message_build_msg(pReq);

	QUEUEDEBUG("%s:%s->%s:%s msg_id:%s",
		pReq->src_name, pReq->src_gid,
		pReq->dst_name, pReq->dst_gid,
		msgstr(pReq->msg_id));

	if(_queue_server_message_upload(pMsg) == dave_false)
	{
		QUEUEABNOR("can't add %s:%s->%s:%s %s",
			pReq->src_name, pReq->src_gid,
			pReq->dst_name, pReq->dst_gid,
			msgstr(pReq->msg_id));
		_queue_server_message_clean_msg(pMsg);
	}
}

void
queue_server_message_download(ThreadId src, QueueDownloadMsgReq *pReq)
{
	ThreadMsg *pMsg;
	QueueDownloadMsgRsp *pRsp = thread_msg(pRsp);

	pMsg = _queue_server_message_download(pReq);

	if(pMsg == NULL)
	{
		pRsp->ret = RetCode_empty_data;
		pRsp->src_name[0] = '\0';
		dave_strcpy(pRsp->dst_name, pReq->name, sizeof(pRsp->dst_name));
		pRsp->src_gid[0] = '\0';
		dave_strcpy(pRsp->dst_gid, pReq->gid, sizeof(pRsp->dst_gid));
		pRsp->msg = NULL;
	}
	else
	{
		pRsp->ret = RetCode_OK;
		dave_strcpy(pRsp->src_name, pMsg->msg_body.src_name, sizeof(pRsp->src_name));
		dave_strcpy(pRsp->dst_name, pMsg->msg_body.dst_name, sizeof(pRsp->dst_name));
		dave_strcpy(pRsp->src_gid, pMsg->msg_body.src_gid, sizeof(pRsp->src_gid));
		dave_strcpy(pRsp->dst_gid, pMsg->msg_body.dst_gid, sizeof(pRsp->dst_gid));
		pRsp->msg = pMsg->msg_body.queue_ptr;

		pMsg->msg_body.queue_ptr = NULL;

		_queue_server_message_clean_msg(pMsg);
	}
	pRsp->ptr = pReq->ptr;

	QUEUEDEBUG("%s:%s->%s:%s ret:%s",
		pRsp->src_name, pRsp->src_gid,
		pRsp->dst_name, pRsp->dst_gid,
		retstr(pRsp->ret));

	id_msg(src, MSGID_QUEUE_DOWNLOAD_MESSAGE_RSP, pRsp);
}

ub
queue_server_message_info(s8 *info_ptr, ub info_len)
{
	ub index, info_index;
	QueueMessage *pMessage;
	ub unprocessed_counter, received_counter, processed_counter;

	info_index = 0;

	info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "QUEUE MESSAGE INFO:\n");

	for(index=0; index<1024000; index++)
	{
		pMessage = kv_index_key_ptr(_message_kv, index);
		if(pMessage == NULL)
			break;

		thread_queue_total(&unprocessed_counter, &received_counter, &processed_counter, pMessage->pQueue, 1);

		info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
			" %s%s%s %lu/%lu:%lu\n",
			pMessage->name,
			pMessage->gid[0] == '\0' ? "":"->",
			pMessage->gid[0] == '\0' ? "":pMessage->gid,
			received_counter, processed_counter, unprocessed_counter);
	}

	return info_index;
}

#endif

