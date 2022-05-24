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
	return NULL;
}

static void
_ramkv_redis_disconnect(void *context)
{

}

static RetCode
_ramkv_redis_command_argv(KVRedis *pKV, void *return_ptr, ub *return_len, sb argc, void **argv_ptr, ub *argv_len)
{
	return RetCode_unsupport;
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
	void *argv_ptr[4];
	ub argv_len[4];
	sb ret;
	ub ret_len = sizeof(sb);

	argv_ptr[0] = "HSET";
	argv_len[0] = 4;

	argv_ptr[1] = pKV->table_name;
	argv_len[1] = pKV->table_name_len;

	argv_ptr[2] = key_ptr;
	argv_len[2] = key_len;

	argv_ptr[3] = value_ptr;
	argv_len[3] = value_len;

	if(_ramkv_redis_command_argv(pKV, &ret, &ret_len, 4, argv_ptr, argv_len) != RetCode_OK)
	{
		KVLOG("invalid command:%s %s %s", argv_ptr[0], pKV->table_name, key_ptr);
		return dave_false;
	}

	if(ret < 0)
	{
		KVLOG("invalid command ret:%d :%s %s %s", ret, argv_ptr[0], pKV->table_name, key_ptr);
		return dave_false;
	}

	return dave_true;
}

ub
ramkv_redis_bin_inq(KVRedis *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len)
{
	void *argv_ptr[3];
	ub argv_len[3];
	RetCode ret;

	argv_ptr[0] = "HGET";
	argv_len[0] = 4;

	argv_ptr[1] = pKV->table_name;
	argv_len[1] = pKV->table_name_len;

	argv_ptr[2] = key_ptr;
	argv_len[2] = key_len;

	ret = _ramkv_redis_command_argv(pKV, value_ptr, &value_len, 3, argv_ptr, argv_len);
	if(ret != RetCode_OK)
	{
		if(ret != RetCode_empty_data)
		{
			KVLOG("invalid command:%s %s %s error:%s", argv_ptr[0], pKV->table_name, key_ptr, retstr(ret));
		}
		return 0;
	}

	return value_len;
}

dave_bool
ramkv_redis_bin_del(KVRedis *pKV, u8 *key_ptr, ub key_len)
{
	void *argv_ptr[3];
	ub argv_len[3];
	sb ret;
	ub ret_len = sizeof(sb);

	argv_ptr[0] = "HDEL";
	argv_len[0] = 4;

	argv_ptr[1] = pKV->table_name;
	argv_len[1] = pKV->table_name_len;

	argv_ptr[2] = key_ptr;
	argv_len[2] = key_len;

	if(_ramkv_redis_command_argv(pKV, &ret, &ret_len, 3, argv_ptr, argv_len) != RetCode_OK)
	{
		KVLOG("invalid command:%s %s %s", argv_ptr[0], pKV->table_name, key_ptr);
		return dave_false;
	}

	if(ret < 0)
	{
		KVLOG("invalid command ret:%d :%s %s %s", ret, argv_ptr[0], pKV->table_name, key_ptr);
		return dave_false;
	}

	return dave_true;
}

#endif

