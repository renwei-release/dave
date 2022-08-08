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

#define CFG_SYNC_BLOCKS_DEFAULT_FLAG (s8 *)"SyncBlocksDefaultFlag"

static dave_bool
_sync_server_default_blocks_flag(s8 *value)
{
	dave_snprintf(value, sizeof(value), "true");
	cfg_set(CFG_SYNC_BLOCKS_DEFAULT_FLAG, value, dave_strlen(value));
	return dave_true;
}

// =====================================================================

dave_bool
sync_server_default_blocks_flag(SyncClient *pClient)
{
	s8 value[64];

	if(cfg_get(CFG_SYNC_BLOCKS_DEFAULT_FLAG, (u8 *)value, sizeof(value)) == dave_false)
	{
		return _sync_server_default_blocks_flag(value);
	}

	if(t_is_all_show_char((u8 *)value, dave_strlen(value)) == dave_false)
	{
		return _sync_server_default_blocks_flag(value);
	}

	t_stdio_tolowers((s8 *)value);

	return dave_strcmp(value, "true");
}

#endif

