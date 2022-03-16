/*
 * ================================================================================
 * (c) Copyright 2020 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.08.10.
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
#include "sync_server_tx.h"
#include "sync_server_rx.h"
#include "sync_server_data.h"
#include "sync_lock.h"
#include "sync_test.h"
#include "sync_log.h"

#define CFG_SYNC_BLOCKS_DEFAULT_FLAG (s8 *)"SyncBlocksDefaultFlag"

// =====================================================================

dave_bool
sync_server_default_blocks_flag(SyncClient *pClient)
{
	u8 value[64];

	if(base_cfg_get(CFG_SYNC_BLOCKS_DEFAULT_FLAG, value, sizeof(value)) == dave_false)
	{
		return dave_true;
	}

	t_stdio_tolowers((s8 *)value);

	return dave_strcmp(value, "true");
}

#endif

