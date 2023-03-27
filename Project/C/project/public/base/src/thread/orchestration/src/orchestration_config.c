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
#include "orchestration_json.h"
#include "thread_log.h"

#define ORCHESTRATION_LOCAL_CFG_FILE "config/ORCHESTRATION.json"

static void *_or_kv = NULL;

static ub
_or_cfg_show(s8 *info_ptr, ub info_len, ORUIDConfig *pConfig)
{
	ub router_index, gid_index, info_index = 0;

	info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
		"uid:%s router_number:%d\n",
		pConfig->uid, pConfig->router_number);

	for(router_index=0; router_index<pConfig->router_number; router_index++)
	{
		info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
			"\tthread:%s",
			pConfig->router_table[router_index].thread);

		if(pConfig->router_table[router_index].pGIDTable != NULL)
		{
			for(gid_index=0; gid_index<pConfig->router_table[router_index].pGIDTable->gid_number; gid_index++)
			{
				info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
					"\t\tgid:%s",
					pConfig->router_table[router_index].pGIDTable->gid_table[gid_index]);
			}
		}

		info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "\n");
	}

	return info_index;
}

static RetCode
_or_cfg_del(void *ramkv, s8 *uid)
{
	ORUIDConfig *pConfig = (ORUIDConfig *)kv_del_key_ptr(ramkv, uid);

	if(pConfig == NULL)
		return RetCode_empty_data;

	or_json_free_config(pConfig);

	return RetCode_OK;
}

static void
_or_cfg_add(void *ramkv, s8 *uid, void *pArrayConfig)
{
	ORUIDConfig *pConfig;

	pConfig = or_json_malloc_config((s8 *)uid, pArrayConfig);
	
	if(pConfig != NULL)
	{
		kv_add_key_ptr(ramkv, uid, pConfig);
	}
}

static void
_or_cfg_load_json_struct(void *pJson, dave_bool put_flag)
{
	struct lh_entry *entry = NULL;
	char *uid = NULL;
	struct json_object *pArrayConfig = NULL;

	entry = json_object_get_object(pJson)->head;

	while(entry)
	{
		uid = (char *)(entry->k);
		pArrayConfig = (struct json_object *)(entry->v);

		if(put_flag == dave_true)
			_or_cfg_add(_or_kv, uid, pArrayConfig);
		else
			_or_cfg_del(_or_kv, uid);

		entry = entry->next;
	}
}

static s8 *
_or_cfg_remote_config_name(s8 *cfg_name)
{
	s8 *dir_path = dave_strfindfrist(cfg_name, '/');

	if(dave_memcmp(dir_path,
		"sync/ORCHESTRATION",
		dave_strlen("sync/ORCHESTRATION")) == dave_false)
	{
		return NULL;
	}

	return dave_strfindlast(cfg_name, '/');
}

static void
_or_cfg_remote_update(MSGBODY *msg)
{
	CFGRemoteUpdate *pUpdate = (CFGRemoteUpdate *)(msg->msg_body);
	s8 *remote_config_name;

	remote_config_name = _or_cfg_remote_config_name(pUpdate->cfg_name);

	if(remote_config_name != NULL)
	{
		void *pJson = dave_string_to_json(pUpdate->cfg_value, pUpdate->cfg_value);

		if(pJson != NULL)
		{
			_or_cfg_load_json_struct(pJson, pUpdate->put_flag);

			dave_json_free(pJson);
		}
	}
}

static void
_or_cfg_local_update(void)
{
	void *pJson;

	pJson = dave_json_read(ORCHESTRATION_LOCAL_CFG_FILE, dave_false);
	if(pJson == NULL)
		return;

	_or_cfg_load_json_struct(pJson, dave_true);

	dave_json_free(pJson);
}

// =====================================================================

void
orchestration_config_init(void)
{
	_or_kv = kv_malloc("orkv", 0, NULL);
	_or_cfg_local_update();
	reg_msg(MSGID_CFG_REMOTE_UPDATE, _or_cfg_remote_update);
}

void
orchestration_config_exit(void)
{
	unreg_msg(MSGID_CFG_REMOTE_UPDATE);
	if(_or_kv != NULL)
	{
		kv_free(_or_kv, _or_cfg_del);
		_or_kv = NULL;
	}
}

ORUIDConfig *
orchestration_config(s8 *uid)
{
	return kv_inq_key_ptr(_or_kv, uid);
}

ub
orchestration_config_info(s8 *info_ptr, ub info_len)
{
	ub index;
	ORUIDConfig *pConfig;
	ub info_index = 0;

	for(index=0; index<1024000; index++)
	{
		pConfig = base_ramkv_inq_index_ptr(_or_kv, index);
		if(pConfig == NULL)
			break;

		info_index += _or_cfg_show(&info_ptr[info_index], info_len-info_index, pConfig);
	}

	return info_index;
}

#endif

