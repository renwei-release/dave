/*
 * ================================================================================
 * (c) Copyright 2021 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2021.02.07.
 * The synchronization service in the future may try to connect to all within
 * the framework of the server.
 * ================================================================================
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_SERVER)
#include "dave_tools.h"
#include "dave_verno.h"
#include "dave_base.h"
#include "sync_param.h"
#include "sync_base_package.h"
#include "sync_server_param.h"
#include "sync_server_data.h"
#include "sync_server_tx.h"
#include "sync_server_broadcadt.h"
#include "sync_server_broadcadt_tx.h"
#include "sync_server_tools.h"
#include "sync_lock.h"
#include "sync_test.h"
#include "sync_log.h"

#define SERVER_BROADCADT_TABLE_NAME (s8 *)"ssbtn"
#define SERVER_BROADCADT_LIST_MAX 10240

typedef struct {
	ThreadId src_id;
	ThreadId dst_id;
	TaskAttribute src_attrib;
	TaskAttribute dst_attrib;
	s8 src_name[DAVE_THREAD_NAME_LEN];
	s8 dst_name[DAVE_THREAD_NAME_LEN];
	ub msg_id;
	BaseMsgType msg_type;
	MBUF *msg_body;

	void *next;
} SyncServerBroadcadtList;

typedef struct {
	s8 thread_name[DAVE_THREAD_NAME_LEN];
	SyncServerBroadcadtList *pListHead;
} SyncServerBroadcadt;

static TLock _broadcadt_pv;
static void *pSyncServerBroadcadt = NULL;

static SyncServerBroadcadtList *
_sync_server_broadcadt_list_malloc(
	ThreadId src_id, ThreadId dst_id,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	s8 *src_name, s8 *dst_name,
	ub msg_id,
	BaseMsgType msg_type,
	u8 *msg_body, ub msg_len)
{
	SyncServerBroadcadtList *pList;

	pList = dave_ralloc(sizeof(SyncServerBroadcadtList));

	pList->src_id = src_id;
	pList->dst_id = dst_id;
	pList->src_attrib = src_attrib;
	pList->dst_attrib = dst_attrib;
	dave_strcpy(pList->src_name, src_name, sizeof(pList->src_name));
	dave_strcpy(pList->dst_name, dst_name, sizeof(pList->dst_name));
	pList->msg_id = msg_id;
	pList->msg_type = msg_type;
	pList->msg_body = dave_mmalloc(msg_len);
	dave_memcpy(pList->msg_body->payload, msg_body, msg_len);

	pList->next = NULL;

	return pList;
}

static void
_sync_server_broadcadt_list_free(SyncServerBroadcadtList *pList)
{
	if(pList == NULL)
	{
		return;
	}

	if(pList->msg_body != NULL)
	{
		dave_mfree(pList->msg_body);
		pList->msg_body = NULL;
	}

	dave_free(pList);
}

static SyncServerBroadcadtList *
_sync_server_broadcadt_list_inq(
	SyncServerBroadcadtList *pListHead,
	s8 *src_name, s8 *dst_name,
	ub msg_id,
	SyncServerBroadcadtList **ppListUp)
{
	if(ppListUp != NULL)
	{
		*ppListUp = pListHead;
	}

	while(pListHead != NULL)
	{
		if((dave_strcmp(pListHead->src_name, src_name) == dave_true)
			&& (dave_strcmp(pListHead->dst_name, dst_name) == dave_true)
			&& (pListHead->msg_id == msg_id))
		{
			return pListHead;
		}

		if(ppListUp != NULL)
		{
			*ppListUp = pListHead;
		}

		pListHead = pListHead->next;
	}

	return NULL;
}

static SyncServerBroadcadtList *
_sync_server_broadcadt_list_add(
	SyncServerBroadcadtList *pListHead,
	ThreadId src_id, ThreadId dst_id,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	s8 *src_name, s8 *dst_name,
	ub msg_id,
	BaseMsgType msg_type,
	u8 *msg_body, ub msg_len)
{
	SyncServerBroadcadtList *pCurList, *pTailList;
	ub safe_counter;

	pCurList = _sync_server_broadcadt_list_inq(pListHead, src_name, dst_name, msg_id, NULL);
	if(pCurList == NULL)
	{
		pCurList = _sync_server_broadcadt_list_malloc(
			src_id, dst_id,
			src_attrib, dst_attrib,
			src_name, dst_name,
			msg_id,
			msg_type,
			msg_body, msg_len);

		if(pListHead == NULL)
		{
			pListHead = pCurList;
		}
		else
		{
			pTailList = pListHead;
			safe_counter = 0;
			while((pTailList->next != NULL) && ((safe_counter ++) < SERVER_BROADCADT_LIST_MAX)) { pTailList = pTailList->next; }
			if(safe_counter >= SERVER_BROADCADT_LIST_MAX)
			{
				SYNCABNOR("invalid safe_counter:%d", safe_counter);
			}

			pTailList->next = pCurList;
		}
	}
	else
	{
		pCurList->src_id = src_id;
		pCurList->dst_id = dst_id;
		pCurList->src_attrib = src_attrib;
		pCurList->dst_attrib = dst_attrib;
		dave_strcpy(pCurList->src_name, src_name, sizeof(pCurList->src_name));
		dave_strcpy(pCurList->dst_name, dst_name, sizeof(pCurList->dst_name));
		pCurList->msg_id = msg_id;
		pCurList->msg_type = msg_type;
		if(pCurList->msg_body->len != msg_len)
		{
			dave_mfree(pCurList->msg_body);
			pCurList->msg_body = dave_mmalloc(msg_len);
		}
		dave_memcpy(dave_mptr(pCurList->msg_body), msg_body, msg_len);		
	}

	return pListHead;
}

static void
_sync_server_broadcadt_list_del(SyncServerBroadcadtList *pListHead)
{
	SyncServerBroadcadtList *pNextList;

	while(pListHead != NULL)
	{
		pNextList = pListHead->next;

		_sync_server_broadcadt_list_free(pListHead);

		pListHead = pNextList;
	}
}

static SyncServerBroadcadt *
_sync_server_broadcadt_add(s8 *thread_name)
{
	SyncServerBroadcadt *pBroadcadt;

	pBroadcadt = base_kv_inq_key_ptr(pSyncServerBroadcadt, thread_name);
	if(pBroadcadt == NULL)
	{
		pBroadcadt = dave_ralloc(sizeof(SyncServerBroadcadt));

		dave_strcpy(pBroadcadt->thread_name, thread_name, sizeof(pBroadcadt->thread_name));
		pBroadcadt->pListHead = NULL;

		base_kv_add_key_ptr(pSyncServerBroadcadt, thread_name, pBroadcadt);
	}

	return pBroadcadt;
}

static SyncServerBroadcadt *
_sync_server_broadcadt_inq(s8 *thread_name)
{
	return (SyncServerBroadcadt *)base_kv_inq_key_ptr(pSyncServerBroadcadt, thread_name);
}

static ErrCode
_sync_server_broadcadt_del(void *kv, s8 *thread_name)
{
	SyncServerBroadcadt *pBroadcadt;

	pBroadcadt = base_kv_del_key_ptr(pSyncServerBroadcadt, thread_name);
	if(pBroadcadt == NULL)
	{
		return ERRCODE_empty_data;
	}

	_sync_server_broadcadt_list_del(pBroadcadt->pListHead);

	dave_free(pBroadcadt);

	return ERRCODE_OK;
}

static ErrCode
_sync_server_broadcadt_list_add_one(
	SyncClient *pSrcClient,
	ThreadId src_id, ThreadId dst_id,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	s8 *src_name, s8 *dst_name,
	ub msg_id,
	BaseMsgType msg_type,
	u8 *msg_body, ub msg_len)
{
	SyncThread *pSrcThread, *pDstThread;
	SyncServerBroadcadt *pBroadcadt;

	pSrcThread = sync_server_find_thread(src_name);
	if(pSrcThread == NULL)
	{
		SYNCLOG("can't find src! %s->%s:%d", src_name, dst_name, msg_id);
		return ERRCODE_can_not_find_thread;
	}
	pDstThread = sync_server_find_thread(dst_name);
	if(pDstThread == NULL)
	{
		SYNCLOG("can't find dst! %s->%s:%d", src_name, dst_name, msg_id);
		return ERRCODE_can_not_find_thread;
	}

	pBroadcadt = _sync_server_broadcadt_add(dst_name);

	pBroadcadt->pListHead = _sync_server_broadcadt_list_add(
		pBroadcadt->pListHead,
		src_id, dst_id,
		src_attrib, dst_attrib,
		src_name, dst_name,
		msg_id,
		msg_type,
		msg_body, msg_len);

	sync_server_broadcadt_tx_the_thread_all_client(
		pSrcClient,
		pSrcThread, pDstThread,
		src_id, dst_id,
		src_attrib, dst_attrib,
		src_name, dst_name,
		msg_id,
		msg_type,
		msg_body, msg_len);

	SYNCTRACE("%s->%s:%d", src_name, dst_name, msg_id);

	return ERRCODE_OK;
}

static void
_sync_server_broadcadt_list_del_one(s8 *src_name, s8 *dst_name, ub msg_id)
{
	SyncServerBroadcadt *pBroadcadt;
	SyncServerBroadcadtList *pDelList, *pUpList;

	pBroadcadt = _sync_server_broadcadt_inq(dst_name);

	pDelList = _sync_server_broadcadt_list_inq(pBroadcadt->pListHead, src_name, dst_name, msg_id, &pUpList);
	if(pDelList == NULL)
	{
		return;
	}

	if(pDelList == pBroadcadt->pListHead)
	{
		pBroadcadt->pListHead = pBroadcadt->pListHead->next;
	}
	else
	{
		pUpList->next = pDelList->next;
	}

	pDelList->next = NULL;

	_sync_server_broadcadt_list_del(pDelList);

	if(pBroadcadt->pListHead == NULL)
	{
		_sync_server_broadcadt_del(pSyncServerBroadcadt, dst_name);
	}

	SYNCTRACE("%s->%s:%d", src_name, dst_name, msg_id);
}

static ErrCode
_sync_server_broadcadt(
	SyncClient *pSrcClient,
	ThreadId route_src, s8 *src,
	ThreadId route_dst, s8 *dst,
	ub msg_id,
	BaseMsgType msg_type,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	u8 *msg_body, ub msg_len)
{
	ErrCode ret = ERRCODE_OK;

	switch(msg_type)
	{
		case BaseMsgType_Broadcast_thread:
				ret = _sync_server_broadcadt_list_add_one(
					pSrcClient,
					route_src, route_dst,
					src_attrib, dst_attrib,
					src, dst,
					msg_id, msg_type,
					msg_body, msg_len);
			break;
		case BaseMsgType_Broadcast_dismiss:
				_sync_server_broadcadt_list_del_one(src, dst, msg_id);
			break;
		case BaseMsgType_Broadcast_remote:
		case BaseMsgType_Broadcast_total:
				ret = sync_server_broadcadt_tx_the_msg_to_all_client(
					pSrcClient,
					route_src, src_attrib, src,
					msg_id,
					msg_type,
					msg_body, msg_len);
			break;
		default:
				SYNCLOG("invalid msg_type:%d", msg_type);
			break;
	}

	return ret;
}

static void
_sync_server_broadcadt_the_thread_has_msg(SyncClient *pDstClient, s8 *thread_name)
{
	SyncClient *pSrcClient;
	SyncThread *pSrcThread, *pDstThread;
	SyncServerBroadcadt *pBroadcadt;
	SyncServerBroadcadtList *pList;

	pBroadcadt = _sync_server_broadcadt_inq(thread_name);
	if(pBroadcadt == NULL)
	{
		SYNCTRACE("verno:%s thread:%s no broadcadt msg!", pDstClient->verno, thread_name);
		return;
	}

	if(pBroadcadt->pListHead ==  NULL)
	{
		_sync_server_broadcadt_del(pSyncServerBroadcadt, thread_name);
		return;
	}

	pDstThread = sync_server_find_thread(thread_name);
	if(pDstThread == NULL)
	{
		SYNCLOG("thread:%s can't find thread!", thread_name);
		return;
	}

	pList = pBroadcadt->pListHead;

	while(pList != NULL)
	{
		pSrcClient = sync_server_find_effective_client(pList->src_name);
		pSrcThread = sync_server_find_thread(pList->src_name);

		if((pSrcClient != NULL) && (pSrcThread != NULL))
		{
			sync_server_broadcadt_tx_the_msg_to_client(
				pSrcThread, pDstThread,
				pSrcClient, pDstClient,
				pList->src_id, pList->dst_id,
				pList->src_attrib, pList->dst_attrib,
				pList->src_name, pList->dst_name,
				pList->msg_id,
				pList->msg_type,
				(u8 *)(pList->msg_body->payload), pList->msg_body->len);
		}
		else
		{
			SYNCLOG("thread:%s can't find client:%x or thread:%x!",
				pList->src_name, pSrcClient, pSrcThread);
		}

		pList = pList->next;
	}
}

// =====================================================================

void
sync_server_broadcadt_init(void)
{
	t_lock_reset(&_broadcadt_pv);

	pSyncServerBroadcadt = base_kv_malloc(SERVER_BROADCADT_TABLE_NAME, KVAttrib_ram, 0, NULL);
}

void
sync_server_broadcadt_exit(void)
{
	base_kv_free(pSyncServerBroadcadt, _sync_server_broadcadt_del);
}

ErrCode
sync_server_broadcadt(
	SyncClient *pSrcClient,
	ThreadId route_src, s8 *src,
	ThreadId route_dst, s8 *dst,
	ub msg_id,
	BaseMsgType msg_type,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	u8 *msg_body, ub msg_len)
{
	ErrCode ret = ERRCODE_Resource_conflicts;

	SYNCTRACE("%s->%s:%d", src, dst, msg_id);

	SAFEZONEv5W(_broadcadt_pv, ret = _sync_server_broadcadt(
			pSrcClient,
			route_src, src,
			route_dst, dst,
			msg_id,
			msg_type,
			src_attrib, dst_attrib,
			msg_body, msg_len			
		);
	);

	return ret;
}

void
sync_server_broadcadt_the_thread_has_msg(SyncClient *pDstClient, s8 *thread_name)
{
	SAFEZONEv5R(_broadcadt_pv, _sync_server_broadcadt_the_thread_has_msg(pDstClient, thread_name); );
}

ub
sync_server_broadcadt_info(s8 *msg_ptr, ub msg_len)
{
	ub msg_index, index;
	SyncServerBroadcadt *pBroadcadt;
	SyncServerBroadcadtList *pList;

	msg_index = 0;

	msg_index += dave_snprintf(&msg_ptr[msg_index], msg_len-msg_index, "BROADCADT INFO:\n");

	index = 0;

	while(index < 10240000)
	{
		pBroadcadt = base_kv_inq_index_ptr(pSyncServerBroadcadt, index);
		if(pBroadcadt == NULL)
		{
			break;
		}

		if(index > 0)
		{
			msg_index += dave_snprintf(&msg_ptr[msg_index], msg_len-msg_index, "\n");
		}

		msg_index += dave_snprintf(&msg_ptr[msg_index], msg_len-msg_index, " %s ", pBroadcadt->thread_name);

		pList = pBroadcadt->pListHead;

		while(pList != NULL)
		{
			msg_index += dave_snprintf(&msg_ptr[msg_index], msg_len-msg_index, "%s->%s:%d ",
				pList->src_name, pList->dst_name, pList->msg_id);

			pList = pList->next;
		}

		index ++;
	}

	if((msg_index > 0) && (msg_ptr[msg_index - 1] != '\n'))
	{
		msg_index += dave_snprintf(&msg_ptr[msg_index], msg_len-msg_index, "\n");
	}

	return msg_index;
}

#endif

