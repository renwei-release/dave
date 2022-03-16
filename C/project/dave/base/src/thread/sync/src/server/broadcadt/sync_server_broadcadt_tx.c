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
#include "base_rxtx.h"
#include "thread_tools.h"
#include "sync_param.h"
#include "sync_base_package.h"
#include "sync_server_param.h"
#include "sync_server_data.h"
#include "sync_server_tx.h"
#include "sync_server_broadcadt.h"
#include "sync_server_tools.h"
#include "sync_lock.h"
#include "sync_test.h"
#include "sync_log.h"

static ErrCode
_sync_server_broadcadt_the_msg_to_all_client(
	SyncClient *pSrcClient,
	ThreadId src_id, TaskAttribute src_attrib, s8 *src_name,
	ub msg_id,
	BaseMsgType msg_type,
	u8 *msg_body, ub msg_len)
{
	ub thread_index, client_index;
	SyncThread *pSrcThread, *pDstThread;
	SyncClient *pDstClient;
	ThreadId dst_id;
	TaskAttribute dst_attrib;
	s8 *dst_name;
	ErrCode ret;

	pSrcThread = sync_server_find_thread(src_name);
	if(pSrcThread == NULL)
	{
		SYNCLOG("can't find src! %s:%d", src_name, msg_id);
		return ERRCODE_can_not_find_thread;
	}

	for(thread_index=0; thread_index<SYNC_THREAD_MAX; thread_index++)
	{
		pDstThread = sync_server_thread(thread_index);

		if((pDstThread->thread_name[0] != '\0')
			&& (pSrcThread != pDstThread)
			&& (sync_server_client_on_thread(pDstThread, pSrcClient) == dave_false)
			&& (dave_strcmp(pDstThread->thread_name, src_name) == dave_false))
		{
			SYNCTRACE("%s->%s:%d", pSrcThread->thread_name, pDstThread->thread_name, msg_id);

			for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
			{
				pDstClient = pDstThread->pClient[client_index];

				if((pDstClient != NULL)
					&& (pSrcClient != pDstClient)
					&& (pDstClient->ready_flag == dave_true))
				{
					dst_name = pDstThread->thread_name;
					dst_id = thread_set_remote(0, pDstThread->thread_index, pDstThread->thread_index, pDstClient->client_index);
					dst_attrib = REMOTE_TASK_ATTRIB;

					ret = sync_server_tx_run_thread_msg_req(
						pSrcThread, pDstThread,
						pSrcClient, pDstClient,
						src_id, dst_id,
						src_name, dst_name,
						src_attrib, dst_attrib,
						msg_id,
						msg_type,
						msg_body, msg_len);

					SYNCTRACE("broadcadt (%d %x/%x %x/%x) ret:%s thread:%s->%s client:%s->%s ready:%d/%d %s->%s msg_id:%d msg_type:%d msg_len:%d",
						t_a2b_errorstr(ret),
						client_index,
						pSrcThread, pDstThread,
						pSrcClient, pDstClient,
						pSrcThread->thread_name, pDstThread->thread_name,
						pSrcClient->verno, pDstClient->verno,
						pSrcClient->ready_flag, pDstClient->ready_flag,
						src_name, dst_name,
						msg_id,
						msg_type,
						msg_len);
				}
			}
		}
	}

	return ERRCODE_OK;
}

static void
_sync_server_broadcadt_tx_the_thread_all_client(
	SyncClient *pSrcClient,
	SyncThread *pSrcThread, SyncThread *pDstThread,
	ThreadId src_id, ThreadId dst_id,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	s8 *src_name, s8 *dst_name,
	ub msg_id,
	BaseMsgType msg_type,
	u8 *msg_body, ub msg_len)
{
	ub client_index;
	SyncClient *pDstClient;
	ErrCode ret;

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		pDstClient = pDstThread->pClient[client_index];
	
		if((pDstClient != NULL) && (pSrcClient != pDstClient))
		{
			dst_name = pDstThread->thread_name;
			dst_id = thread_set_remote(0, pDstThread->thread_index, pDstThread->thread_index, SYNC_NET_INDEX_MAX);
			dst_attrib = REMOTE_TASK_ATTRIB;

			ret = sync_server_tx_run_thread_msg_req(
				pSrcThread, pDstThread,
				pSrcClient, pDstClient,
				src_id, dst_id,
				src_name, dst_name,
				src_attrib, dst_attrib,
				msg_id,
				msg_type,
				msg_body, msg_len);

			SYNCTRACE("broadcadt ret:%s client_index:%d %s->%s %x/%s->%x/%s %s->%s msg_id:%d msg_type:%d msg_len:%d",
				t_a2b_errorstr(ret),
				client_index,
				pSrcThread->thread_name, pDstThread->thread_name,
				pSrcClient, pSrcClient->verno, pDstClient, pDstClient->verno,
				src_name, dst_name,
				msg_id,
				msg_type,
				msg_len);
		}
	}
}

// =====================================================================

ErrCode
sync_server_broadcadt_tx_the_msg_to_all_client(
	SyncClient *pSrcClient,
	ThreadId src_id, TaskAttribute src_attrib, s8 *src_name,
	ub msg_id,
	BaseMsgType msg_type,
	u8 *msg_body, ub msg_len)
{
	return _sync_server_broadcadt_the_msg_to_all_client(
		pSrcClient,
		src_id, src_attrib, src_name,
		msg_id,
		msg_type,
		msg_body, msg_len);
}

void
sync_server_broadcadt_tx_the_thread_all_client(
	SyncClient *pSrcClient,
	SyncThread *pSrcThread, SyncThread *pDstThread,
	ThreadId src_id, ThreadId dst_id,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	s8 *src_name, s8 *dst_name,
	ub msg_id,
	BaseMsgType msg_type,
	u8 *msg_body, ub msg_len)
{
	_sync_server_broadcadt_tx_the_thread_all_client(
		pSrcClient,
		pSrcThread, pDstThread,
		src_id, dst_id,
		src_attrib, dst_attrib,
		src_name, dst_name,
		msg_id,
		msg_type,
		msg_body, msg_len);
}

void
sync_server_broadcadt_tx_the_msg_to_client(
	SyncThread *pSrcThread, SyncThread *pDstThread,
	SyncClient *pSrcClient, SyncClient *pDstClient,
	ThreadId src_id, ThreadId dst_id,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	s8 *src_name, s8 *dst_name,
	ub msg_id,
	BaseMsgType msg_type,
	u8 *msg_body, ub msg_len)
{
	ErrCode ret;

	src_name = pSrcThread->thread_name;
	src_id = thread_set_remote(0, thread_get_local(src_id), pSrcThread->thread_index, pSrcClient->client_index);
	src_attrib = REMOTE_TASK_ATTRIB;

	dst_name = pDstThread->thread_name;
	dst_id = thread_set_remote(0, thread_get_local(dst_id), pDstThread->thread_index, pDstClient->client_index);
	dst_attrib = REMOTE_TASK_ATTRIB;

	ret = sync_server_tx_run_thread_msg_req(
		pSrcThread, pDstThread,
		pSrcClient, pDstClient,
		src_id, dst_id,
		src_name, dst_name,
		src_attrib, dst_attrib,
		msg_id,
		msg_type,
		msg_body, msg_len);
	
	SYNCTRACE("broadcadt ret:%s thread:%s->%s client:%s->%s ready:%d/%d %s->%s msg_id:%d msg_type:%d msg_len:%d",
		t_a2b_errorstr(ret),
		pSrcThread->thread_name, pDstThread->thread_name,
		pSrcClient->verno, pDstClient->verno,
		pSrcClient->ready_flag, pDstClient->ready_flag,
		src_name, dst_name,
		msg_id,
		msg_type,
		msg_len);
}

#endif

