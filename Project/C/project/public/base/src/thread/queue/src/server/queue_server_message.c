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
#include "thread_struct.h"
#include "thread_tools.h"
#include "queue_log.h"

typedef struct {
	s8 name[DAVE_THREAD_NAME_LEN];
	s8 gid[DAVE_GLOBALLY_IDENTIFIER_LEN];

	ThreadQueue *pQueue;
} QueueMessage;

static void *_message_kv = NULL;
static TLock _message_pv;

static s8 *
_queue_server_message_key(s8 *key_ptr, ub key_len, s8 *dst_name, s8 *dst_gid)
{
	dave_snprintf(key_ptr, key_len, "%s:%s", dst_name, dst_gid);
	return key_ptr;
}

static QueueMessage *
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

static void
_queue_server_message_free(QueueMessage *pMessage)
{
	thread_queue_free(pMessage->pQueue, 1);

	dave_free(pMessage->pQueue);

	dave_free(pMessage);
}

static QueueMessage *
_queue_server_message_inq(s8 *key, s8 *name, s8 *gid)
{
	QueueMessage *pMessage = NULL;

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

static ThreadMsg *
_queue_server_message_build_msg(QueueUploadMsgReq *pReq)
{
	ThreadMsg *pMsg;

	pMsg = (ThreadMsg *)dave_malloc(sizeof(ThreadMsg));

	dave_strcpy(pMsg->msg_body.src_name, pReq->src_name, sizeof(pMsg->msg_body.src_name));
	dave_strcpy(pMsg->msg_body.dst_name, pReq->dst_name, sizeof(pMsg->msg_body.dst_name));
	dave_strcpy(pMsg->msg_body.src_gid, pReq->src_gid, sizeof(pMsg->msg_body.src_gid));
	dave_strcpy(pMsg->msg_body.dst_gid, pReq->dst_gid, sizeof(pMsg->msg_body.dst_gid));
	pMsg->msg_body.msg_id = pReq->msg_id;
	pMsg->msg_body.msg_len = pReq->msg->tot_len;
	pMsg->msg_body.msg_body = pReq->msg;
	pMsg->msg_body.user_ptr = pReq->ptr;

	pMsg->pQueue = NULL;
	pMsg->next = NULL;

	return pMsg;
}

static ThreadMsg *
_queue_server_message_read_msg(ThreadQueue *pQueue)
{
	return thread_queue_read(pQueue);
}

static void
_queue_server_message_clean_msg(ThreadMsg *pMsg)
{
	if(pMsg->msg_body.msg_body != NULL)
	{
		dave_mfree(pMsg->msg_body.msg_body);
		pMsg->msg_body.msg_body = NULL;
	}

	dave_free(pMsg);
}

static void
_queue_server_message_update_state(ThreadQueue *pQueue)
{
	ThreadMsg *pMsg;

	if(pQueue != NULL)
	{
		pMsg = thread_queue_clone(pQueue);
		if(pMsg != NULL)
		{
			QueueUpdateStateReq *pReq = thread_msg(pReq);

			dave_strcpy(pReq->src_name, pMsg->msg_body.src_name, sizeof(pReq->src_name));
			dave_strcpy(pReq->dst_name, pMsg->msg_body.dst_name, sizeof(pReq->dst_name));
			dave_strcpy(pReq->src_gid, pMsg->msg_body.src_gid, sizeof(pReq->src_gid));
			dave_strcpy(pReq->dst_gid, pMsg->msg_body.dst_gid, sizeof(pReq->dst_gid));

			if(pReq->dst_gid[0] == '\0')
				name_msg(pReq->dst_name, MSGID_QUEUE_UPDATE_STATE_REQ, pReq);
			else
				gid_msg(pReq->dst_gid, pReq->dst_name, MSGID_QUEUE_UPDATE_STATE_REQ, pReq);

			dave_free(pMsg);
		}
	}
}

static dave_bool
_queue_server_message_upload(ThreadMsg *pMsg)
{
	s8 key[256];
	QueueMessage *pMessage;
	RetCode ret;

	_queue_server_message_key(
		key, sizeof(key),
		pMsg->msg_body.dst_name, pMsg->msg_body.dst_gid);

	pMessage = _queue_server_message_inq(key, pMsg->msg_body.dst_name, pMsg->msg_body.dst_gid);
	
	ret = thread_queue_write(pMessage->pQueue, pMsg);
	if(ret == RetCode_OK)
	{
		if((pMessage->pQueue->list_number > 0) && (pMessage->pQueue->list_number < 3))
		{
			_queue_server_message_update_state(pMessage->pQueue);
		}
		return dave_true;
	}

	QUEUEABNOR("can't write %s:%s->%s:%s %s ret:%s",
		pMsg->msg_body.src_name, pMsg->msg_body.src_gid,
		pMsg->msg_body.dst_name, pMsg->msg_body.dst_gid,
		msgstr(pMsg->msg_body.msg_id),
		retstr(ret));	
	return dave_false;
}

static ThreadQueue *
_queue_server_message_download(QueueDownloadMsgReq *pReq)
{
	s8 key[256];
	QueueMessage *pMessage;

	_queue_server_message_key(
		key, sizeof(key),
		pReq->name, pReq->gid);

	pMessage = _queue_server_message_inq(key, pReq->name, pReq->gid);
	if(pMessage == NULL)
		return NULL;

	return pMessage->pQueue;
}

static void
_queue_server_message_timerout(void *ramkv, s8 *key)
{
	QueueMessage *pMessage = kv_inq_key_ptr(_message_kv, key);

	if(pMessage == NULL)
		return;

	_queue_server_message_update_state(pMessage->pQueue);
}

static RetCode
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
	_message_kv = kv_malloc("queue-server-message", 60, _queue_server_message_timerout);
	t_lock_reset(&_message_pv);
}

void
queue_server_message_exit(void)
{
	kv_free(_message_kv, _queue_server_message_recycle);
}

void
queue_server_message_upload(ThreadId src, QueueUploadMsgReq *pReq)
{
	ThreadMsg *pMsg = _queue_server_message_build_msg(pReq);

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
	ThreadQueue *pQueue = _queue_server_message_download(pReq);
	ThreadMsg *pMsg = NULL;
	QueueDownloadMsgRsp *pRsp = thread_msg(pRsp);

	if(pQueue != NULL)
	{
		pMsg = _queue_server_message_read_msg(pQueue);
	}

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
		pRsp->msg = pMsg->msg_body.msg_body;

		pMsg->msg_body.msg_body = NULL;

		_queue_server_message_clean_msg(pMsg);
	}
	pRsp->ptr = pReq->ptr;

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

