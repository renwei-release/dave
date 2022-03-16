/*
 * ================================================================================
 * (c) Copyright 2020 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.09.30.
 * The synchronization service in the future may try to connect to all within
 * the framework of the server.
 * ================================================================================
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_CLIENT)
#include "dave_verno.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "thread_tools.h"
#include "sync_base_package.h"
#include "sync_param.h"
#include "sync_client_tx.h"
#include "sync_client_param.h"
#include "sync_client_link.h"
#include "sync_client_data.h"
#include "sync_client_tools.h"
#include "sync_lock.h"
#include "sync_log.h"

static inline SyncServer *
_sync_client_load_balancer_(LinkThread *pThread)
{
	SyncServer *pServer = NULL;
	ub server_index;
	ub safe_counter;

	server_index = pThread->chose_server_index % SERVER_DATA_MAX;

	for(safe_counter=0; safe_counter<SERVER_DATA_MAX; safe_counter++)
	{
		if(server_index >= SERVER_DATA_MAX)
		{
			server_index = 0;
		}

		if((pThread->pServer[server_index] != NULL)
			&& (pThread->pServer[server_index]->server_type == SyncServerType_client)
			&& (pThread->pServer[server_index]->server_socket != INVALID_SOCKET_ID)
			&& (pThread->pServer[server_index]->server_ready == dave_true))
		{
			pServer = pThread->pServer[server_index];

			pThread->chose_server_index = ++ server_index;
			break;
		}

		server_index ++;
	}

	return pServer;
}

static inline SyncServer *
_sync_client_safe_load_balancer(LinkThread *pThread)
{
	SyncServer *pServer;

	t_lock_spin(&(pThread->chose_server_pv));
	pServer = _sync_client_load_balancer_(pThread);
	t_unlock_spin(&(pThread->chose_server_pv));

	return pServer;
}

static inline SyncServer *
_sync_client_load_balancer(MSGBODY *pMsg)
{
	s8 *thread_dst;
	ub thread_dst_index;
	LinkThread *pThread;
	SyncServer *pServer;

	thread_dst = thread_name(pMsg->msg_dst);
	if((thread_dst == NULL) || (dave_strcmp(thread_dst, "NULL") == dave_true))
	{
		SYNCLTRACE(60,1,"invalid dst >> %lx/%s->%lx:%d",
			pMsg->msg_src, thread_name(pMsg->msg_src), pMsg->msg_dst, pMsg->msg_id);
		return NULL;
	}
	thread_dst_index = sync_client_data_thread_name_to_index(thread_dst);

	pThread = sync_client_data_thread_on_name(thread_dst, thread_dst_index);
	if(pThread == NULL)
	{
		SYNCLTRACE(60,5,"You send a message(%lx/%s->%lx/%s:%d) to the remote, \
but the source(%s) of the message was not registered as a remote thread. \
This message will be processed by SYNC routing",
			pMsg->msg_src, thread_name(pMsg->msg_src),
			pMsg->msg_dst, thread_name(pMsg->msg_dst),
			pMsg->msg_id,
			thread_dst);
		return NULL;
	}

	pServer = _sync_client_safe_load_balancer(pThread);

	if(pServer == NULL)
	{
		SYNCDEBUG("thread:%s can't find pServer! %s->%s:%d",
			pThread->thread_name,
			thread_name(pMsg->msg_src), thread_name(pMsg->msg_dst),
			pMsg->msg_id);
	}

	return pServer;
}

static inline SyncServer *
_sync_client_load_link(MSGBODY *pMsg)
{
	ub server_index;

	thread_get_remote(pMsg->msg_dst, NULL, NULL, &server_index);

	return sync_client_data_server_inq_on_index(server_index);
}

static inline SyncServer *
_sync_client_chose_server(MSGBODY *pMsg)
{
	SyncServer *pServer = NULL;

	if(pMsg->msg_type != BaseMsgType_Unicast)
	{
		/*
		 * 广播消息，全部交给SYNC服务器转发。
		 */
		pServer = sync_client_data_sync_server();
	}
	else if((thread_is_remote(pMsg->msg_dst) == dave_false)
		|| (thread_get_net(pMsg->msg_dst) == SYNC_NET_INDEX_MAX))
	{
		/*
		 * 本地始发消息。
		 */
		pServer = _sync_client_load_balancer(pMsg);
	}
	else if(thread_is_remote(pMsg->msg_dst) == dave_true)
	{
		if(thread_is_sync(pMsg->msg_dst) == dave_true)
		{
			/*
			 * 来自SYNC服务器的消息，现在仍然需要回到SYNC服务器。
			 */
			pServer = sync_client_data_sync_server();
		}
		else
		{
			/*
			 * 来自LINK的消息。
			 */
			pServer = _sync_client_load_link(pMsg);
		}
	}

	if(pServer == NULL)
	{
		SYNCDEBUG("Is there no choose? %s/%lx->%s/%lx:%d",
			thread_name(pMsg->msg_src), pMsg->msg_src,
			thread_name(pMsg->msg_dst), pMsg->msg_dst,
			pMsg->msg_id);

		pServer = sync_client_data_sync_server();
	}

	return pServer;
}

// =====================================================================

SyncServer *
sync_client_chose_server(MSGBODY *pMsg)
{
	return _sync_client_chose_server(pMsg);
}

#endif

