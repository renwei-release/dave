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

#define OR_THREAD_JSON_KEY "thread"
#define OR_GID_JSON_KEY "gid"

static dave_bool
_or_cfg_check_valid_config_data(s8 *config_data)
{
	return t_is_all_show_char((u8 *)config_data, dave_strlen(config_data));
}

static dave_bool
_or_cfg_check_valid_gid_data(s8 *gid)
{
	if(_or_cfg_check_valid_config_data(gid) == dave_true)
	{
		if(dave_strlen(gid) == (DAVE_GLOBALLY_IDENTIFIER_LEN - 1))
		{
			return dave_true;
		}
	}

	return dave_false;
}

static ORUIDGIDTable *
_or_cfg_malloc_gid(void)
{
	ORUIDGIDTable *pGIDTable = dave_ralloc(sizeof(ORUIDGIDTable));

	pGIDTable->load_balancer = 0;
	pGIDTable->gid_number = 0;

	return pGIDTable;
}

static void
_or_cfg_free_gid(ORUIDGIDTable *pGIDTable)
{
	if(pGIDTable != NULL)
	{
		dave_memset(pGIDTable, 0x00, sizeof(ORUIDGIDTable));

		dave_free(pGIDTable);
	}
}

static ORUIDConfig *
_or_cfg_malloc_config(s8 *uid)
{
	ORUIDConfig *pConfig = dave_ralloc(sizeof(ORUIDConfig));

	dave_strcpy(pConfig->uid, uid, sizeof(pConfig->uid));

	pConfig->router_number = 0;

	return pConfig;
}

static void
_or_cfg_free_config(ORUIDConfig *pConfig)
{
	ub route_index;

	if(pConfig != NULL)
	{
		for(route_index=0; route_index<DAVE_OR_ROUTER_TABLE_MAX; route_index++)		
		{
			_or_cfg_free_gid(pConfig->router_table[route_index].pGIDTable);
		}

		dave_memset(pConfig, 0x00, sizeof(ORUIDConfig));

		dave_free(pConfig);
	}
}

static dave_bool
_or_cfg_analyze_json_gid(ORUIDRouter *pRouter, void *pArrayGID)
{
	sb array_len = dave_json_get_array_length(pArrayGID);
	sb array_index;
	s8 *gid_ptr;

	if(array_len == 0)
	{
		THREADLOG("the array:%s length is zero!", dave_json_to_string(pArrayGID, NULL));
		return dave_false;
	}

	if(array_len > DAVE_OR_GID_TABLE_MAX)
	{
		THREADLOG("the array_len:%d overflow! DAVE_OR_GID_TABLE_MAX:%d",
			array_len, DAVE_OR_GID_TABLE_MAX);
		array_len = DAVE_OR_GID_TABLE_MAX;
	}

	pRouter->pGIDTable = _or_cfg_malloc_gid();

	for(array_index=0; array_index<array_len; array_index++)
	{
		gid_ptr = dave_json_array_get_str(pArrayGID, array_index, NULL);
		if(gid_ptr != NULL)
		{
			if(_or_cfg_check_valid_gid_data(gid_ptr) == dave_true)
			{
				dave_strcpy(pRouter->pGIDTable->gid_table[pRouter->pGIDTable->gid_number ++], gid_ptr, DAVE_GLOBALLY_IDENTIFIER_LEN);
			}
			else
			{
				THREADLOG("invalid gid_ptr:%s", gid_ptr);
				pRouter->pGIDTable->gid_number = 0;
				break;
			}
		}
	}

	if(pRouter->pGIDTable->gid_number == 0)
	{
		_or_cfg_free_gid(pRouter->pGIDTable);
		pRouter->pGIDTable = NULL;
		return dave_false;
	}

	return dave_true;
}

static dave_bool
_or_cfg_analyze_json_router(s8 *uid, ORUIDRouter *pUpRouter, ORUIDRouter *pCurRouter, void *pRouterJson)
{
	void *pArrayGID;

	if(dave_json_get_str_v2(pRouterJson, OR_THREAD_JSON_KEY, pCurRouter->thread, sizeof(pCurRouter->thread)) == 0)
	{
		THREADLOG("get invalid router json:%s", dave_json_to_string(pRouterJson, NULL));
		return dave_false;
	}

	if(pUpRouter != NULL)
	{
		if(dave_strcmp(pUpRouter->thread, pCurRouter->thread) == dave_true)
		{
			THREADLOG("Duplicate calls:%s found on configuration:%s", pCurRouter->thread, uid);
			return dave_false;
		}
	}

	if(_or_cfg_check_valid_config_data(pCurRouter->thread) == dave_false)
	{
		THREADLOG("invalid thread:%s", pCurRouter->thread);
		return dave_false;
	}

	pArrayGID = dave_json_get_array(pRouterJson, OR_GID_JSON_KEY);
	if(pArrayGID != NULL)
	{
		return _or_cfg_analyze_json_gid(pCurRouter, pArrayGID);
	}

	return dave_true;
}

static ORUIDConfig *
_or_cfg_analyze_json_config(s8 *uid, void *pArrayConfig)
{
	sb array_len = dave_json_get_array_length(pArrayConfig);
	sb array_index;
	ORUIDConfig *pConfig;
	void *pRouterJson;
	ORUIDRouter *pUpRouter;
	ORUIDRouter *pCurRouter;

	if(array_len == 0)
	{
		THREADLOG("the array:%s length is zero!", dave_json_to_string(pArrayConfig, NULL));
		return NULL;
	}

	if(array_len > DAVE_OR_ROUTER_TABLE_MAX)
	{
		THREADLOG("the array_len:%d overflow! DAVE_OR_ROUTER_TABLE_MAX:%d",
			array_len, DAVE_OR_ROUTER_TABLE_MAX);
		array_len = DAVE_OR_ROUTER_TABLE_MAX;
	}

	pConfig = _or_cfg_malloc_config(uid);

	for(array_index=0; array_index<array_len; array_index++)
	{
		pRouterJson = dave_json_get_array_idx(pArrayConfig, array_index);
		if(pRouterJson != NULL)
		{
			if(pConfig->router_number == 0)
				pUpRouter = NULL;
			else
				pUpRouter = &(pConfig->router_table[pConfig->router_number - 1]);
			pCurRouter = &(pConfig->router_table[pConfig->router_number]);

			if(_or_cfg_analyze_json_router(uid, pUpRouter, pCurRouter, pRouterJson) == dave_true)
			{
				pConfig->router_number ++;
			}
			else
			{
				pConfig->router_number = 0;
				break;
			}
		}
	}

	if(pConfig->router_number == 0)
	{
		THREADLOG("the array:%s has invalid router!", dave_json_to_string(pArrayConfig, NULL));
		_or_cfg_free_config(pConfig);
		pConfig = NULL;
	}

	return pConfig;
}

// =====================================================================

ORUIDConfig *
or_json_malloc_config(s8 *uid, void *pArrayConfig)
{
	if(_or_cfg_check_valid_config_data(uid) == dave_false)
	{
		THREADLOG("invalid uid:%s", uid);
		return NULL;
	}

	return _or_cfg_analyze_json_config(uid, pArrayConfig);
}

void
or_json_free_config(ORUIDConfig *pConfig)
{
	_or_cfg_free_config(pConfig);
}

#endif

