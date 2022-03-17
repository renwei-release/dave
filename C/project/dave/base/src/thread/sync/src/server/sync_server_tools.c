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
#include "sync_lock.h"
#include "sync_test.h"
#include "sync_log.h"

static dave_bool
_sync_server_client_on_thread(SyncThread *pThread, SyncClient *pClient)
{
	ub client_index;

	if(pThread == NULL)
	{
		if(pClient != NULL)
		{
			SYNCLOG("pThread is NULL! verno:%s", pClient->verno);
		}
		return dave_false;
	}

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		if(pThread->pClient[client_index] == pClient)
		{
			return dave_true;
		}
	}

	return dave_false;
}

static dave_bool
_sync_server_are_they_brothers(SyncClient *pClientA, SyncClient *pClientB)
{
	s8 client_a_verno[128];
	s8 client_b_verno[128];

	if((pClientA == NULL) || (pClientB == NULL))
	{
		return dave_false;
	}

	if(pClientA->verno[0] == '\0')
	{
		SYNCLOG("%s has empty verno!",
			ipv4str(pClientA->NetInfo.addr.ip.ip_addr, pClientA->NetInfo.port));
		return dave_true;
	}

	if(pClientB->verno[0] == '\0')
	{
		SYNCLOG("%s has empty verno!",
			ipv4str(pClientB->NetInfo.addr.ip.ip_addr, pClientB->NetInfo.port));
		return dave_true;
	}

	dave_verno_product(pClientA->verno, client_a_verno, sizeof(client_a_verno));
	dave_verno_product(pClientB->verno, client_b_verno, sizeof(client_b_verno));

	return dave_strcmp(client_a_verno, client_b_verno);
}

static dave_bool
_sync_server_still_have_ready_brothers(SyncClient *pClient)
{
	ub client_index;

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		if((pClient != NULL)
			&& (pClient->client_socket != INVALID_SOCKET_ID)
			&& (pClient->ready_flag == dave_true)
			&& (sync_server_client(client_index) != pClient)
			&& (sync_server_client(client_index)->client_socket != INVALID_SOCKET_ID)
			&& (sync_server_client(client_index)->ready_flag == dave_true))
		{
			if(_sync_server_are_they_brothers(sync_server_client(client_index), pClient) == dave_true)
			{
				return dave_true;
			}
		}
	}

	return dave_false;
}

static dave_bool
_sync_server_still_have_blocks_brothers(SyncClient *pClient)
{
	ub client_index;

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		if((pClient != NULL)
			&& (pClient->client_socket != INVALID_SOCKET_ID)
			&& (sync_server_client(client_index) != pClient)
			&& (sync_server_client(client_index)->client_socket != INVALID_SOCKET_ID)
			&& (sync_server_client(client_index)->blocks_flag == dave_true))
		{
			if(_sync_server_are_they_brothers(sync_server_client(client_index), pClient) == dave_true)
			{
				return dave_true;
			}
		}
	}

	return dave_false;
}

// =====================================================================

dave_bool
sync_server_client_on_thread(SyncThread *pThread, SyncClient *pClient)
{
	return _sync_server_client_on_thread(pThread, pClient);
}

dave_bool
sync_server_are_they_brothers(SyncClient *pClientA, SyncClient *pClientB)
{
	return _sync_server_are_they_brothers(pClientA, pClientB);
}

dave_bool
sync_server_still_have_ready_brothers(SyncClient *pClient)
{
	return _sync_server_still_have_ready_brothers(pClient);
}

dave_bool
sync_server_still_have_blocks_brothers(SyncClient *pClient)
{
	return _sync_server_still_have_blocks_brothers(pClient);
}

dave_bool
sync_server_client_on_work(SyncClient *pClient)
{
	if(pClient != NULL)
	{
		if(pClient->ready_flag == dave_true)
		{
			if((pClient->blocks_flag == dave_true) && (pClient->client_flag == dave_true))
			{
				return dave_true;
			}

			if(pClient->release_quantity > 0)
			{
				return dave_true;
			}
		}
	}

	return dave_false;
}

#endif

