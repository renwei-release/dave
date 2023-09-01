/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_CLIENT)
#include "dave_verno.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "base_tools.h"
#include "sync_param.h"
#include "sync_client_tx.h"
#include "sync_client_data.h"
#include "sync_client_app_tx.h"
#include "sync_log.h"

static dave_bool
_sync_client_app_tx_server(SyncServer *pServer, ub msg_id, ub msg_len, void *msg_body)
{
	if(pServer == NULL)
	{
		return dave_false;
	}

	if(pServer->server_socket == INVALID_SOCKET_ID)
	{
		return dave_false;
	}
	if(pServer->server_type == SyncServerType_sync_client)
	{
		SYNCTRACE("There is no need to send a message to SYNC for the time being.");
		return dave_true;
	}

	return sync_client_tx_run_internal_msg_req(pServer, msg_id, msg_len, msg_body, dave_false);
}

static dave_bool
_sync_client_tx_busy(SyncServer *pServer)
{
	SystemBusy busy;

	dave_strcpy(busy.gid, globally_identifier(), sizeof(busy.gid));
	dave_strcpy(busy.verno, dave_verno(), sizeof(busy.verno));

	if(pServer == NULL)
		return sync_client_app_tx_all_server(MSGID_SYSTEM_BUSY, sizeof(SystemBusy), &busy);
	else
		return sync_client_app_tx_server(pServer, MSGID_SYSTEM_BUSY, sizeof(SystemBusy), &busy);
}

static dave_bool
_sync_client_tx_idle(SyncServer *pServer)
{
	SystemIdle idle; 

	dave_strcpy(idle.gid, globally_identifier(), sizeof(idle.gid));
	dave_strcpy(idle.verno, dave_verno(), sizeof(idle.verno));

	if(pServer == NULL)
		return sync_client_app_tx_all_server(MSGID_SYSTEM_IDLE, sizeof(SystemIdle), &idle);
	else
		return sync_client_app_tx_server(pServer, MSGID_SYSTEM_IDLE, sizeof(SystemIdle), &idle);
}

// =====================================================================

dave_bool
sync_client_app_tx_all_server(ub msg_id, ub msg_len, void *msg_body)
{
	ub server_index;
	SyncServer *pServer;

	for(server_index=0; server_index<SERVER_DATA_MAX; server_index++)
	{
		pServer = sync_client_server(server_index);
		if(pServer == NULL)
		{
			break;
		}

		_sync_client_app_tx_server(pServer, msg_id, msg_len, msg_body);
	}

	return dave_true;
}

dave_bool
sync_client_app_tx_server(SyncServer *pServer, ub msg_id, ub msg_len, void *msg_body)
{
	return _sync_client_app_tx_server(pServer, msg_id, msg_len, msg_body);
}

dave_bool
sync_client_tx_system_state(SyncServer *pServer)
{
	dave_bool busy_flag = sync_client_data_get_busy();

	if((pServer != NULL) && (pServer->server_type == SyncServerType_sync_client))
	{
		SYNCTRACE("There is no need to send a message to SYNC for the time being.");
		return dave_true;
	}

	SYNCTRACE("====== system on %s, remind %s ======",
		busy_flag == dave_true ? "busy" : "idle",
		pServer == NULL ? "all" : pServer->verno);

	if(busy_flag == dave_true)
	{
		return _sync_client_tx_busy(pServer);
	}
	else
	{
		return _sync_client_tx_idle(pServer);
	}
}

#endif

