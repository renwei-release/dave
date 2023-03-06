/*
 * Copyright (c) 2022 Renwei
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
#include "thread_tools.h"
#include "sync_base_package.h"
#include "sync_param.h"
#include "sync_client_tx.h"
#include "sync_client_param.h"
#include "sync_client_link.h"
#include "sync_client_load_balancer.h"
#include "sync_lock.h"
#include "sync_log.h"

#define NTP_UPDATE_MIN_INTERVAL 3

static void
_sync_client_ntp(DateStruct *remote_date, DateStruct *local_date)
{
	dave_bool update_flag = dave_false;
	ub remote_date_ub, local_date_ub;

	SYNCDEBUG("ntp local:%s remote:%s", datestr(local_date), datestr2(remote_date));

	remote_date_ub = t_time_struct_second(remote_date);
	local_date_ub = t_time_struct_second(local_date);

	if(remote_date_ub >= local_date_ub)
	{
		if((remote_date_ub - local_date_ub) >= NTP_UPDATE_MIN_INTERVAL)
			update_flag = dave_true;
	}
	else
	{
		if((local_date_ub - remote_date_ub) >= NTP_UPDATE_MIN_INTERVAL)
			update_flag = dave_true;
	}

	if(update_flag == dave_true)
	{
		SYNCLOG("date update %ld/%s->%ld/%s",
			local_date_ub, datestr(local_date),
			remote_date_ub, datestr2(remote_date));
		t_time_set_date(remote_date);
	}
}

// =====================================================================

void
sync_client_ntp(SyncServer *pServer, DateStruct remote_date)
{
	DateStruct local_date;

	if(pServer->server_type == SyncServerType_sync_client)
	{
		local_date = t_time_get_date(NULL);

		_sync_client_ntp(&remote_date, &local_date);
	}
}

#endif

