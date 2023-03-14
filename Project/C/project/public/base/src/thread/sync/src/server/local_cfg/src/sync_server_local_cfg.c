/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_SERVER)
#include "dave_3rdparty.h"
#include "dave_tools.h"
#include "dave_verno.h"
#include "sync_server_remote_cfg.h"
#include "remote_etcd_cfg.h"
#include "sync_server_app_tx.h"
#include "sync_log.h"

static s8 * _local_update_list[] = {
	CFG_LOG_SERVER_DOMAIN,
	CFG_BASE_CHAIN_ENABLE,
	CFG_BASE_CHAIN_TYPE,
	CFG_IO_DOMAIN,
	NULL
};

static void
_sync_server_list_cfg_update_to_all_client(s8 *cfg_name, s8 *cfg_value)
{
	CFGUpdate update;

	dave_memset(&update, 0x00, sizeof(CFGUpdate));

	dave_strcpy(update.cfg_name, cfg_name, sizeof(update.cfg_name));
	update.cfg_length = dave_strlen(cfg_value);
	dave_strcpy(update.cfg_value, cfg_value, sizeof(update.cfg_value));

	sync_server_app_tx_all_client(MSGID_CFG_UPDATE, sizeof(CFGUpdate), &update);
}

static void
_sync_server_list_cfg_update_to_the_client(SyncClient *pClient)
{
	ub list_index, safe_counter;
	CFGUpdate update;

	list_index = safe_counter = 0;

	while(((++ safe_counter) < 999999) && (_local_update_list[list_index] != NULL))
	{
		dave_memset(&update, 0x00, sizeof(CFGUpdate));
	
		dave_strcpy(update.cfg_name, _local_update_list[list_index], sizeof(update.cfg_name));
		if(cfg_get(update.cfg_name, update.cfg_value, sizeof(update.cfg_value)) == dave_true)
		{
			update.cfg_length = dave_strlen(update.cfg_value);
			sync_server_app_tx_client(pClient, MSGID_CFG_UPDATE, sizeof(CFGUpdate), &update);
		}

		list_index ++;
	}
}

static void
_sync_server_local_cfg_update(MSGBODY *msg)
{
	CFGUpdate *pUpdate = (CFGUpdate *)(msg->msg_body);
	ub list_index, safe_counter;

	list_index = safe_counter = 0;

	while(((++ safe_counter) < 999999) && (_local_update_list[list_index] != NULL))
	{
		if(dave_strcmp(pUpdate->cfg_name, _local_update_list[list_index]) == dave_true)
		{
			_sync_server_list_cfg_update_to_all_client(pUpdate->cfg_name, (s8 *)(pUpdate->cfg_value));
		}

		list_index ++;
	}
}

// =====================================================================

void
sync_server_local_cfg_init(void)
{
	reg_msg(MSGID_CFG_UPDATE, _sync_server_local_cfg_update);
}

void
sync_server_local_cfg_exit(void)
{
	unreg_msg(MSGID_CFG_UPDATE);
}

void
sync_server_local_cfg_tell_client(SyncClient *pClient)
{
	_sync_server_list_cfg_update_to_the_client(pClient);
}

#endif

