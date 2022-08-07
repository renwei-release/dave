/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
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
#include "sync_server_tx.h"
#include "sync_server_rx.h"
#include "sync_server_data.h"
#include "sync_server_tools.h"
#include "sync_server_broadcadt.h"
#include "sync_server_config.h"
#include "sync_server_sync.h"
#include "sync_lock.h"
#include "sync_test.h"
#include "sync_log.h"

static dave_bool
_sync_server_sync_the_thread_on_client(SyncThread *pThread, SyncClient *pClient)
{
	ub client_index;

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		if(pThread->pClient[client_index] == pClient)
		{
			return dave_true;
		}
	}

	return dave_false;
}

static void
_sync_server_sync_client_ready_follow_up(SyncClient *pClient)
{
	SYNCTRACE("%s/%s ip:%s sync remote done! sync_thread_index:%d",
		pClient->globally_identifier, pClient->verno,
		ipv4str(pClient->NetInfo.addr.ip.ip_addr, pClient->NetInfo.port),
		pClient->sync_thread_index);

	sync_server_tx_blocks_state(pClient);

	pClient->sync_resynchronization_counter = 0;
	pClient->sync_thread_index = 0;
	
	sync_server_sync_auto_link(pClient);
	
	sync_server_config_tell_client(pClient);
}

static void
_sync_server_sync_add_remote_thread(SyncClient *pClient)
{
	SyncThread *pThread;
	dave_bool empty_flag;

	if(pClient == NULL)
	{
		SYNCLOG("pClient is NULL!");
		return;
	}

	if(pClient->sync_thread_index == 0)
		empty_flag = dave_true;
	else
		empty_flag = dave_false;

	while(pClient->sync_thread_index < SYNC_THREAD_MAX)
	{
		pThread = sync_server_thread(pClient->sync_thread_index);
		if((pThread == NULL)
			|| (pThread->thread_name[0] == '\0')
			|| (_sync_server_sync_the_thread_on_client(pThread, pClient) == dave_true))
		{
			pClient->sync_thread_index ++;
			continue;
		}

		empty_flag = dave_false;

		SYNCTRACE("%s/%s socket:%d thread:%s sync index:%d",
				pClient->globally_identifier, pClient->verno,
				pClient->client_socket, pThread->thread_name,
				pClient->sync_thread_index);

		// add the thread name to me!
		sync_server_tx_add_remote_thread_req(pClient, pThread->thread_name, pThread->thread_index);
		break;
	}

	if(pClient->sync_thread_index >= SYNC_THREAD_MAX)
	{
		if((pClient->ready_flag == dave_false) && (empty_flag == dave_true))
		{
			sync_server_tx_add_remote_thread_req(pClient, "", 0);
		}
		pClient->ready_flag = dave_true;

		_sync_server_sync_client_ready_follow_up(pClient);
	}
	else
	{
		SYNCDEBUG("socket:%d sync remote at here:%d!",
			pClient->client_socket,
			pClient->sync_thread_index);
	}
}

static void
_sync_server_sync_thread_booting(SyncClient *pClient)
{
	if(pClient == NULL)
	{
		return;
	}

	if(pClient->client_socket == INVALID_SOCKET_ID)
	{
		return;
	}

	if(pClient->sync_thread_flag != dave_true)
	{
		return;
	}

	pClient->sync_thread_flag = dave_false;

	if(pClient->sync_thread_index != 0)
	{
		SYNCLOG("the %s last sync<%d> was not completed!", pClient->verno, pClient->sync_thread_index);
		pClient->sync_thread_index = 0;
	}

	SYNCTRACE("verno:%s socket:%d", pClient->verno, pClient->client_socket);

	_sync_server_sync_add_remote_thread(pClient);
}

static dave_bool
_sync_server_sync_link_to(SyncClient *pDstClient, SyncClient *pSrcClient)
{
	dave_bool ret;
	ub dst_client_index = pDstClient->client_index;

	SYNCTRACE("verno:%s->%s(%s) link_port:%d link_flag:%d ready_flag:%d blocks_flag:%d client_flag:%d",
		pSrcClient->verno, pDstClient->verno, pSrcClient->send_down_and_up_flag[dst_client_index]==dave_true?"up":"down",
		pSrcClient->link_port,
		pSrcClient->link_flag,
		pSrcClient->ready_flag,
		pSrcClient->blocks_flag,
		pSrcClient->client_flag);

	if((pSrcClient->link_port != 0)
		&& (pSrcClient->link_flag == dave_true)
		&& (pSrcClient->ready_flag == dave_true)
		&& (pSrcClient->blocks_flag == dave_true)
		&& (pSrcClient->client_flag == dave_true))
	{
		if(pSrcClient->send_down_and_up_flag[dst_client_index] == dave_false)
		{
			ret = sync_server_tx_link_up_req(pDstClient, pSrcClient->verno, pSrcClient->globally_identifier, pSrcClient->link_ip, pSrcClient->link_port);
			if(ret == dave_true)
			{
				pSrcClient->send_down_and_up_flag[dst_client_index] = dave_true;
			}
		}
		else
		{
			ret = dave_true;
		}
	}
	else
	{
		if(pSrcClient->send_down_and_up_flag[dst_client_index] == dave_true)
		{
			ret = sync_server_tx_link_down_req(pDstClient, pSrcClient->verno, pSrcClient->link_ip, pSrcClient->link_port);
			if(ret == dave_true)
			{
				pSrcClient->send_down_and_up_flag[dst_client_index] = dave_false;
			}
		}
		else
		{
			ret = dave_true;
		}
	}

	if(ret == dave_false)
	{
		SYNCLOG("%s->%s failed!", pSrcClient->verno, pDstClient->verno);
	}

	return ret;
}

static void
_sync_server_sync_link_to_me(SyncClient *pClient)
{
	ub client_index;
	SyncClient *pOtherClient;

	SYNCTRACE("%s", pClient->verno);

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		pOtherClient = sync_server_client(client_index);

		if((pOtherClient != pClient) && (pOtherClient->client_socket != INVALID_SOCKET_ID))
		{
			_sync_server_sync_link_to(pClient, pOtherClient);
		}
	}
}

static void
_sync_server_sync_link_to_other(SyncClient *pClient)
{
	ub client_index;
	SyncClient *pOtherClient;

	SYNCTRACE("%s", pClient->verno);

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		pOtherClient = sync_server_client(client_index);

		if((pOtherClient != pClient) && (pOtherClient->client_socket != INVALID_SOCKET_ID))
		{
			_sync_server_sync_link_to(pOtherClient, pClient);
		}
	}
}

// =====================================================================

void
sync_server_sync_thread_next(SyncClient *pClient)
{
	pClient->sync_thread_index ++;

	_sync_server_sync_add_remote_thread(pClient);
}

void
sync_server_sync_thread_booting(SyncClient *pClient)
{
	_sync_server_sync_thread_booting(pClient);
}

void
__sync_server_sync_link__(SyncClient *pClient, dave_bool up_flag, s8 *fun, ub line)
{
	SYNCTRACE("verno:%s client:%s %s <%s:%d>",
		pClient->verno,
		pClient->client_flag==dave_true?"idle":"busy",
		up_flag==dave_true?"up":"down",
		fun, line);

	pClient->link_flag = up_flag;

	if(pClient->link_flag == dave_true)
	{
		_sync_server_sync_link_to_me(pClient);
	}

	_sync_server_sync_link_to_other(pClient);
}

void
sync_server_sync_auto_link(SyncClient *pClient)
{
	sync_server_sync_link(pClient, pClient->link_flag);
}

#endif

