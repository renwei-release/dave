/*
 * ================================================================================
 * (c) Copyright 2020 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.10.22.
 * ================================================================================
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_os.h"
#include "dave_tools.h"
#include "kv_param.h"
#include "kv_redis_struct.h"
#include "kv_redis_opt.h"
#include "kv_redis_api.h"
#include "kv_redis_cfg.h"
#include "kv_log.h"

#ifndef REDIS_3RDPARTY

void *
dave_redis_connect(s8 *ip, ub port)
{
	return NULL;
}

void
dave_redis_disconnect(void *context)
{

}

ErrCode
dave_redis_command_argv(void *context, void *values, ub *values_len, sb argc, void **argv, ub *argv_len)
{
	return ERRCODE_unsupport;
}

#endif

static ErrCode
_kv_redis_command_argv(KVRedis *pKV, void *return_ptr, ub *return_len, sb argc, void **argv_ptr, ub *argv_len)
{
	return dave_redis_command_argv(pKV->redis_context, return_ptr, return_len, argc, argv_ptr, argv_len);
}

// ====================================================================

void *
kv_redis_connect(KVRedis *pKV)
{
	kv_redis_cfg(pKV);

	return dave_redis_connect(pKV->redis_address, pKV->redis_port);
}

void
kv_redis_disconnect(KVRedis *pKV)
{
	if(pKV->redis_context != NULL)
	{
		dave_redis_disconnect(pKV->redis_context);

		pKV->redis_context = NULL;
	}
}

dave_bool
kv_redis_bin_add(KVRedis *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len)
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

	if(_kv_redis_command_argv(pKV, &ret, &ret_len, 4, argv_ptr, argv_len) != ERRCODE_OK)
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
kv_redis_bin_inq(KVRedis *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len)
{
	void *argv_ptr[3];
	ub argv_len[3];
	ErrCode ret;

	argv_ptr[0] = "HGET";
	argv_len[0] = 4;

	argv_ptr[1] = pKV->table_name;
	argv_len[1] = pKV->table_name_len;

	argv_ptr[2] = key_ptr;
	argv_len[2] = key_len;

	ret = _kv_redis_command_argv(pKV, value_ptr, &value_len, 3, argv_ptr, argv_len);
	if(ret != ERRCODE_OK)
	{
		if(ret != ERRCODE_empty_data)
		{
			KVLOG("invalid command:%s %s %s error:%s", argv_ptr[0], pKV->table_name, key_ptr, errorstr(ret));
		}
		return 0;
	}

	return value_len;
}

dave_bool
kv_redis_bin_del(KVRedis *pKV, u8 *key_ptr, ub key_len)
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

	if(_kv_redis_command_argv(pKV, &ret, &ret_len, 3, argv_ptr, argv_len) != ERRCODE_OK)
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

