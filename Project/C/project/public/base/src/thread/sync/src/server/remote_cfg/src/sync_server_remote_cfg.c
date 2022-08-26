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

static void
_sync_server_the_config_tell_all_client(dave_bool put_flag, s8 *key, s8 *value)
{
	CFGRemoteUpdate update;

	update.put_flag = put_flag;
	dave_strcpy(update.cfg_name, key, sizeof(update.cfg_name));
	dave_strcpy(update.cfg_value, value, sizeof(update.cfg_value));

	sync_server_app_tx_all_client(MSGID_CFG_REMOTE_UPDATE, sizeof(CFGRemoteUpdate), &update);
}

static void
_sync_server_the_client_tell_all_config(SyncClient *pClient)
{
	CFGRemoteUpdate update;
	ub index;

	for(index=0; index<102400; index++)
	{
		update.put_flag = dave_true;
		if(rcfg_index(
			index,
			update.cfg_name, sizeof(update.cfg_name),
			update.cfg_value, sizeof(update.cfg_value)) < 0)
		{
			break;
		}

		sync_server_app_tx_client(pClient, MSGID_CFG_REMOTE_UPDATE, sizeof(CFGRemoteUpdate), &update);
	}
}

// =====================================================================

void
sync_server_remote_cfg_init(void)
{
	remote_etcd_cfg_init();
}

void
sync_server_remote_cfg_exit(void)
{
	remote_etcd_cfg_exit();
}

dave_bool
sync_server_remote_cfg_set(SyncClient *pClient, CFGRemoteUpdate *pUpdate)
{
	if(pUpdate->put_flag == dave_true)
	{
		return remote_etcd_cfg_set(pClient->verno, pClient->globally_identifier, pUpdate->cfg_name, pUpdate->cfg_value);
	}

	return dave_false;
}

void
sync_server_remote_cfg_tell_client(SyncClient *pClient)
{
	_sync_server_the_client_tell_all_config(pClient);
}

void
sync_server_remote_cfg_tell_config(dave_bool put_flag, s8 *key, s8 *value)
{
	_sync_server_the_config_tell_all_client(put_flag, key, value);
}

#endif

