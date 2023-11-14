/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_os.h"
#include "dave_tools.h"
#include "ramkv_param.h"
#include "ramkv_redis_struct.h"
#include "ramkv_redis_opt.h"
#include "ramkv_redis_api.h"
#include "ramkv_redis_cfg.h"
#include "ramkv_log.h"

static void *
_ramkv_redis_connect(s8 *ip, ub port)
{
	return dave_redis_connect(ip, port);
}

static void
_ramkv_redis_disconnect(void *context)
{
	dave_redis_disconnect(context);
}

// ====================================================================

void *
ramkv_redis_connect(KVRedis *pKV)
{
	ramkv_redis_cfg(pKV);

	return _ramkv_redis_connect(pKV->redis_address, pKV->redis_port);
}

void
ramkv_redis_disconnect(KVRedis *pKV)
{
	if(pKV->redis_context != NULL)
	{
		_ramkv_redis_disconnect(pKV->redis_context);

		pKV->redis_context = NULL;
	}
}

dave_bool
ramkv_redis_bin_add(KVRedis *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len)
{
	ub set_command_len = 128 + pKV->table_name_len + key_len + value_len * 5;
	ub set_command_index;
	s8 *set_command_ptr;
	RetCode ret;

	set_command_ptr = dave_malloc(set_command_len);

	set_command_index = dave_snprintf(set_command_ptr, set_command_len, "HSET %s %s ", pKV->table_name_ptr, key_ptr);
	t_crypto_base64_encode((const u8 *)value_ptr, value_len, &(set_command_ptr[set_command_index]), set_command_len-set_command_index);

	ret = dave_redis_set(pKV->redis_context, set_command_ptr);

	if(ret != RetCode_OK)
	{
		KVLOG("ret:%s command:(%s)", retstr(ret), set_command_ptr);
		dave_free(set_command_ptr);
		return dave_false;
	}
	else
	{
		KVDEBUG("command:(%s) success!", set_command_ptr);
		dave_free(set_command_ptr);
		return dave_true;
	}
}

sb
ramkv_redis_bin_inq(KVRedis *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len)
{
	ub get_command_len = 128 + pKV->table_name_len + key_len;
	s8 *get_command_ptr;
	ub bin_len = value_len * 10;
	u8 *bin_ptr;

	get_command_ptr = dave_malloc(get_command_len);
	bin_ptr = dave_malloc(bin_len);

	dave_snprintf(get_command_ptr, get_command_len, "HGET %s %s", pKV->table_name_ptr, key_ptr);

	bin_len = dave_redis_get(pKV->redis_context, get_command_ptr, bin_ptr, bin_len);
	if(bin_len > 0)
	{
		if(t_crypto_base64_decode((const s8 *)bin_ptr, bin_len, (u8 *)value_ptr, &value_len) == dave_false)
		{
			value_len = 0;
			KVLOG("command:(%s) get invalid data:%s", get_command_ptr, bin_ptr);
		}
	}
	else
	{
		KVDEBUG("command:(%s) get empty data!", get_command_ptr);
		value_len = 0;
	}

	dave_free(get_command_ptr);
	dave_free(bin_ptr);

	return (sb)value_len;
}

dave_bool
ramkv_redis_bin_del(KVRedis *pKV, u8 *key_ptr, ub key_len)
{
	ub del_command_len = 128 + pKV->table_name_len + key_len;
	s8 *del_command_ptr;
	RetCode ret;

	del_command_ptr = dave_malloc(del_command_len);

	dave_snprintf(del_command_ptr, del_command_len, "HDEL %s %s", pKV->table_name_ptr, key_ptr);

	ret = dave_redis_set(pKV->redis_context, del_command_ptr);
	if(ret != RetCode_OK)
	{
		KVLOG("ret:%s command:(%s)", retstr(ret), del_command_ptr);
	}

	dave_free(del_command_ptr);

	return dave_true;
}

#endif

