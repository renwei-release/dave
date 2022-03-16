/*
 * ================================================================================
 * (c) Copyright 2020 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.11.13.
 * The synchronization service in the future may try to connect to all within
 * the framework of the server.
 * ================================================================================
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_SERVER)
#include "dave_verno.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "sync_base_package.h"
#include "sync_param.h"
#include "sync_server_tools.h"
#include "sync_server_cfg.h"
#include "sync_test.h"
#include "sync_lock.h"
#include "sync_log.h"

static dave_bool
_sync_server_setup_blocks_flag_default(SyncClient *pClient)
{
	dave_bool blocks_flag;

	if(sync_server_still_have_blocks_brothers(pClient) == dave_false)
	{
		blocks_flag = dave_true;
	}
	else
	{
		blocks_flag = sync_server_default_blocks_flag(pClient);
	}

	return blocks_flag;
}

static dave_bool
_sync_server_setup_client_flag_default(SyncClient *pClient)
{
	return dave_true;
}

// =====================================================================

void
sync_server_setup_flag(SyncClient *pClient)
{
	pClient->blocks_flag = _sync_server_setup_blocks_flag_default(pClient);
	pClient->client_flag = _sync_server_setup_client_flag_default(pClient);
}

#endif

