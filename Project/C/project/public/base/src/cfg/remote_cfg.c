/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "dave_os.h"
#include "dave_3rdparty.h"
#include "dave_tools.h"
#include "cfg_param.h"
#include "cfg_log.h"

#define MIN_REMOTE_CFG_TTL 30

typedef struct {
	CFGRemoteUpdate update;
} RemoteReflash;

static void *_remote_cfg_kv = NULL;

static void
_base_remote_update(dave_bool put_flag, s8 *name, s8 *value, sb ttl)
{
	CFGRemoteUpdate *pUpdate = thread_msg(pUpdate);

	pUpdate->put_flag = put_flag;
	dave_strcpy(pUpdate->cfg_name, name, sizeof(pUpdate->cfg_name));
	dave_strcpy(pUpdate->cfg_value, value, sizeof(pUpdate->cfg_value));
	pUpdate->cfg_mbuf_name = NULL;
	pUpdate->cfg_mbuf_value = NULL;
	pUpdate->ttl = ttl;

	name_msg(SYNC_CLIENT_THREAD_NAME, MSGID_CFG_REMOTE_UPDATE, pUpdate);
}

static void
_base_remote_reflash(TIMERID timer_id, ub thread_index, void *param_ptr)
{
	RemoteReflash *pReflash = (RemoteReflash *)param_ptr;

	_base_remote_update(
		dave_true,
		pReflash->update.cfg_name, pReflash->update.cfg_value,
		pReflash->update.ttl);
}

// =====================================================================

void
base_remote_cfg_init(void)
{
	_remote_cfg_kv = kv_malloc("rcfgkv", KvAttrib_list, 0, NULL);
}

void
base_remote_cfg_exit(void)
{
	kv_free(_remote_cfg_kv, NULL);
}

dave_bool
base_remote_cfg_internal_add(s8 *name, s8 *value)
{
	return kv_add_key_value(_remote_cfg_kv, name, value);
}

dave_bool
base_remote_cfg_internal_del(s8 *name)
{
	return kv_del_key_value(_remote_cfg_kv, name);
}

RetCode
base_remote_cfg_set(s8 *name, s8 *value, sb ttl)
{
	RemoteReflash *pReflash;

	if(ttl <= 0)
	{
		ttl = 0;
	}

	CFGDEBUG("%s : %s ttl:%d", name, value, ttl);

	if(ttl > 0)
	{
		if(ttl < MIN_REMOTE_CFG_TTL)
		{
			ttl = MIN_REMOTE_CFG_TTL;
		}

		pReflash = dave_malloc(sizeof(RemoteReflash));

		pReflash->update.put_flag = dave_true;
		dave_strcpy(pReflash->update.cfg_name, name, sizeof(pReflash->update.cfg_name));
		dave_strcpy(pReflash->update.cfg_value, value, sizeof(pReflash->update.cfg_value));
		pReflash->update.ttl = ttl;

		base_timer_param_creat(name, _base_remote_reflash, pReflash, sizeof(pReflash), (ttl/2) * 1000);
	}

	_base_remote_update(dave_true, name, value, ttl);

	return RetCode_OK;
}

sb
base_remote_cfg_get(s8 *name, s8 *value_ptr, ub value_len)
{
	return kv_inq_key_value(_remote_cfg_kv, name, value_ptr, value_len);
}

void
base_remote_cfg_del(s8 *name)
{
	if(name == NULL)
	{
		return;
	}

	base_timer_kill(name);

	_base_remote_update(dave_false, name, NULL, -1);
}

sb
base_remote_cfg_index(ub index, s8 *key_ptr, ub key_len, s8 *value_ptr, ub value_len)
{
	return kv_index_key_value(_remote_cfg_kv, index, key_ptr, key_len, value_ptr, value_len);
}

#endif

