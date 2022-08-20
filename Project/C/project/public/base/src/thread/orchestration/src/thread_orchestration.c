/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "dave_3rdparty.h"
#include "thread_parameter.h"
#include "thread_struct.h"
#include "thread_lock.h"
#include "thread_mem.h"
#include "thread_tools.h"
#include "orchestration_config.h"
#include "thread_log.h"

static inline void
_thread_orchestration_sub_router(ThreadSubRouter *pSubRouter, ORUIDRouter *pCfgRouter)
{
	dave_strcpy(pSubRouter->thread, pCfgRouter->thread, sizeof(pSubRouter->thread));
	if(pCfgRouter->pGIDTable == NULL)
	{
		pSubRouter->gid[0] = '\0';
	}
	else
	{
		if(pCfgRouter->pGIDTable->gid_number > DAVE_OR_GID_TABLE_MAX)
		{
			pCfgRouter->pGIDTable->gid_number = DAVE_OR_GID_TABLE_MAX;
		}

		dave_strcpy(pSubRouter->gid,
			pCfgRouter->pGIDTable->gid_table[(pCfgRouter->pGIDTable->load_balancer ++) % DAVE_OR_GID_TABLE_MAX],
			sizeof(pSubRouter->gid));
	}
}

static inline void
_thread_orchestration_router(ThreadRouter *pRouter, ORUIDConfig *pConfig)
{
	ub router_index;

	dave_strcpy(pRouter->uid, pConfig->uid, sizeof(pRouter->uid));

	if(pConfig->router_number > DAVE_OR_ROUTER_TABLE_MAX)
	{
		THREADABNOR("invalid router_number:%d", pConfig->router_number);
		pConfig->router_number = DAVE_OR_ROUTER_TABLE_MAX;
	}

	pRouter->router_number = pConfig->router_number;
	pRouter->current_router_index = 0;

	for(router_index=0; router_index<pRouter->router_number; router_index++)
	{
		_thread_orchestration_sub_router(
			&(pRouter->sub_router[router_index]),
			&(pConfig->router_table[router_index]));
	}
}

// =====================================================================

void
thread_orchestration_init(void)
{
	orchestration_config_init();
}

void
thread_orchestration_exit(void)
{
	orchestration_config_exit();
}

dave_bool
thread_orchestration_router(ThreadRouter *pRouter, s8 *uid)
{
	ORUIDConfig *pConfig = orchestration_config(uid);

	if(pConfig == NULL)
	{
		THREADLTRACE(60,1,"uid:%s can't find router!", uid);
		return dave_false;
	}

	_thread_orchestration_router(pRouter, pConfig);

	return dave_true;
}

#endif

