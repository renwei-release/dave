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
#include "base_rxtx.h"
#include "base_tools.h"
#include "thread_tools.h"
#include "sync_server_data.h"
#include "sync_server_tx.h"

static void
_sync_server_app_tx_client(SyncClient *pClient, ub msg_id, ub msg_len, void *msg_body)
{
	if(pClient == NULL)
	{
		return;
	}

	if((pClient->client_socket != INVALID_SOCKET_ID)
		&& (pClient->ready_flag == dave_true))
	{
		sync_server_tx_run_internal_msg_req(pClient, msg_id, msg_len, msg_body);
	}
}

// =====================================================================

void
sync_server_app_tx_all_client(ub msg_id, ub msg_len, void *msg_body)
{
	ub client_index;
	SyncClient *pClient;

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		pClient = sync_server_client(client_index);
		if(pClient == NULL)
		{
			break;
		}

		_sync_server_app_tx_client(pClient, msg_id, msg_len, msg_body);
	}
}

void
sync_server_app_tx_client(SyncClient *pClient, ub msg_id, ub msg_len, void *msg_body)
{
	_sync_server_app_tx_client(pClient, msg_id, msg_len, msg_body);
}

#endif

