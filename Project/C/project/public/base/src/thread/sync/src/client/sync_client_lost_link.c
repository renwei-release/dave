/*
 * Copyright (c) 2026 Renwei
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
#include "dave_os.h"
#include "globally_identifier.h"
#include "sync_param.h"
#include "sync_log.h"
#include "sync_client_lost_link.h"

static ub _sync_in_time = 0;
static ub _sync_out_quik_out_times = 0;

// =====================================================================


void
sync_client_lost_link_in(SyncServer *pServer)
{
	if(pServer->server_type == SyncServerType_sync_client)
	{
		_sync_in_time = dave_os_time_s();
	}
}

void
sync_client_lost_link_out(SyncServer *pServer)
{
	if(pServer->server_type == SyncServerType_sync_client)
	{
		if((dave_os_time_s() - _sync_in_time) < 16)
		{
			if((_sync_out_quik_out_times ++) > 6)
			{
				/*
				 * If the client frequently disconnects from sync in a short time,
				 * it is very likely that sync detected a GID conflict,
				 * so we proactively refresh the GID.
				 */
				SYNCABNOR("client_quik_out_times:%d, frequent quick disconnect from sync, "
					"likely GID conflict detected by sync, proactively refresh GID "
					"and reset_globally_identifier",
					_sync_out_quik_out_times);
				reset_globally_identifier();
			}
		}
		else
		{
			_sync_out_quik_out_times = 0;
		}
	}
}

#endif

