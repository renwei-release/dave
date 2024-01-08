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
#include "remote_cfg_kv.h"
#include "remote_cfg_file.h"
#include "cfg_log.h"

#define MIN_REMOTE_CFG_TTL 60
#define REFRESH_LEAD_TIME 10

typedef struct {
	CFGRemoteSyncUpdate update;
} RemoteReflash;

static dave_bool
_base_remote_update(dave_bool put_flag, s8 *name, s8 *value, sb ttl)
{
	CFGRemoteSyncUpdate *pUpdate = thread_msg(pUpdate);

	pUpdate->put_flag = put_flag;
	pUpdate->cfg_mbuf_name = t_a2b_str_to_mbuf(name, 0);
	pUpdate->cfg_mbuf_value = t_a2b_str_to_mbuf(value, 0);
	pUpdate->ttl = ttl;

	return name_msg(SYNC_CLIENT_THREAD_NAME, MSGID_CFG_REMOTE_SYNC_UPDATE, pUpdate);
}

static void
_base_remote_reflash(TIMERID timer_id, ub thread_index, void *param_ptr)
{
	RemoteReflash *pReflash = (RemoteReflash *)param_ptr;

	_base_remote_update(
		dave_true,
		ms8(pReflash->update.cfg_mbuf_name), ms8(pReflash->update.cfg_mbuf_value),
		pReflash->update.ttl);
}

static void
_base_remote_reflash_creat(s8 *name, s8 *value, sb ttl)
{
	RemoteReflash *pReflash;

	if(ttl < MIN_REMOTE_CFG_TTL)
	{
		ttl = MIN_REMOTE_CFG_TTL;
	}

	pReflash = dave_malloc(sizeof(RemoteReflash));
	
	pReflash->update.put_flag = dave_true;
	pReflash->update.cfg_mbuf_name = t_a2b_str_to_mbuf(name, 0);
	pReflash->update.cfg_mbuf_value = t_a2b_str_to_mbuf(value, 0);
	pReflash->update.ttl = ttl;

	base_timer_param_creat(name, _base_remote_reflash, pReflash, sizeof(void *), (ttl - REFRESH_LEAD_TIME) * 1000);
}

static void
_base_remote_reflash_del(s8 *name)
{
	RemoteReflash *pReflash;

	pReflash = base_timer_kill(name);
	if(pReflash != NULL)
	{
		dave_mfree(pReflash->update.cfg_mbuf_name);
		dave_mfree(pReflash->update.cfg_mbuf_value);

		dave_free(pReflash);
	}
}

// =====================================================================

void
base_remote_cfg_init(void)
{
	remote_cfg_file_init();
	remote_cfg_kv_init();
}

void
base_remote_cfg_exit(void)
{
	remote_cfg_file_exit();
	remote_cfg_kv_exit();
}

dave_bool
base_remote_cfg_internal_add(s8 *name, s8 *value)
{
	CFGDEBUG("%s : %s", name, value);
	return remote_cfg_file_set(name, value);
}

sb
base_remote_cfg_internal_inq(s8 *name, s8 *value_ptr, ub value_len)
{
	return remote_cfg_file_get(name, value_ptr, value_len);
}

dave_bool
base_remote_cfg_internal_del(s8 *name)
{
	CFGDEBUG("%s", name);
	remote_cfg_file_del(name);
	remote_cfg_kv_del(name);
	return dave_true;
}

RetCode
base_remote_cfg_set(s8 *name, s8 *value, sb ttl)
{
	CFGDEBUG("%s : %s ttl:%d", name, value, ttl);

	if(ttl <= 0)
	{
		ttl = 0;
	}

	if(ttl > 0)
	{
		_base_remote_reflash_del(name);

		_base_remote_reflash_creat(name, value, ttl);
	}

	_base_remote_update(dave_true, name, value, ttl);

	return RetCode_OK;
}

sb
base_remote_cfg_get(s8 *name, s8 *value_ptr, ub value_len)
{
	sb get_len;

	get_len = remote_cfg_kv_get(name, value_ptr, value_len);
	if(get_len < 0)
	{
		CFGDEBUG("%s:%d/%s", name, get_len, value_ptr);

		get_len = remote_cfg_file_get(name, value_ptr, value_len);
		if(get_len >= 0)
		{
			CFGDEBUG("%s:%d/%s", name, get_len, value_ptr);

			remote_cfg_kv_set(name, value_ptr);
		}
	}

	return get_len;
}

dave_bool
base_remote_cfg_del(s8 *name)
{
	if(name == NULL)
	{
		CFGLOG("name is NULL!");
		return dave_false;
	}

	_base_remote_reflash_del(name);

	return _base_remote_update(dave_false, name, NULL, -1);
}

sb
base_remote_cfg_index(ub index, s8 *key_ptr, ub key_len, s8 *value_ptr, ub value_len)
{
	return remote_cfg_file_index(index, key_ptr, key_len, value_ptr, value_len);
}

#endif

