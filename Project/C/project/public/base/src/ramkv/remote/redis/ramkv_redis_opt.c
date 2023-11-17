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
#include "dave_3rdparty.h"
#include "dave_store.h"
#include "ramkv_param.h"
#include "ramkv_redis_struct.h"
#include "ramkv_redis_opt.h"
#include "ramkv_redis_api.h"
#include "ramkv_redis_cfg.h"
#include "ramkv_log.h"

static void *
_ramkv_redis_remote_command(MBUF *command)
{
	StoreRedisReq *pReq = thread_msg(pReq);
	StoreRedisRsp *pRsp;
	void *pJson;
	
	pReq->command = command;
	pReq->ptr = NULL;
	
	pRsp = name_co(STORE_THREAD_NAME, STORE_REDIS_REQ, pReq, STORE_REDIS_RSP);
	if(pRsp == NULL)
	{
		KVLOG("timer out!");
		return NULL;
	}

	if(pRsp->ret != RetCode_OK)
	{
		KVLOG("ret:%s", retstr(pRsp->ret));
		dave_mfree(pRsp->reply);
		return NULL;
	}

	if(pRsp->reply == NULL)
	{
		return NULL;
	}

	pJson = t_a2b_mbuf_to_json(pRsp->reply);

	dave_mfree(pRsp->reply);

	return pJson;
}

static void *
_ramkv_redis_local_command(KVRedis *pKV, MBUF *command)
{
	void *pJson;

	if(pKV->redis_context == NULL)
	{
		KVLOG("redis_context is NULL! %s %s", pKV->table_name_ptr, ms8(command));
		pJson = NULL;
	}
	else
	{
		pJson = dave_redis_command(pKV->redis_context, ms8(command));
	}

	dave_mfree(command);

	return pJson;
}

static void *
_ramkv_redis_connect(s8 *ip, ub port)
{
#ifdef REDIS_3RDPARTY
	return dave_redis_connect(ip, port);
#else
	return NULL;
#endif
}

static void
_ramkv_redis_disconnect(void *context)
{
#ifdef REDIS_3RDPARTY
	dave_redis_disconnect(context);
#endif
}

static void *
_ramkv_redis_command(KVRedis *pKV, MBUF *command)
{
	void *pJson;

	if(pKV->local_redis_flag == dave_true)
	{
		pJson = _ramkv_redis_local_command(pKV, command);
	}
	else
	{
		pJson = _ramkv_redis_remote_command(command);
	}

	return pJson;
}

static RetCode
_ramkv_redis_bin_add(KVRedis *pKV, MBUF *command)
{
	RetCode ret;
	void *pJson;
	sb command_ret;

	pJson = _ramkv_redis_command(pKV, command);

	if(dave_json_get_sb(pJson, "INTEGER", &command_ret) == dave_false)
		command_ret = -1;
	
	dave_json_free(pJson);
	
	if((command_ret == 0) || (command_ret == 1))
		ret = RetCode_OK;
	else
		ret = RetCode_invalid_option;

	if(ret != RetCode_OK)
	{
		KVLOG("ret:%s", retstr(ret));
	}

	return ret;
}

static sb
_ramkv_redis_bin_inq(KVRedis *pKV, MBUF *command, s8 *bin_ptr, ub bin_len)
{
	void *pJson;

	pJson = _ramkv_redis_command(pKV, command);

	bin_len = dave_json_get_str_v2(pJson, "STRING", bin_ptr, bin_len);
	
	dave_json_free(pJson);

	return bin_len;
}

static RetCode
_ramkv_redis_bin_del(KVRedis *pKV, MBUF *command)
{
	RetCode ret;
	void *pJson;
	sb command_ret;

	pJson = _ramkv_redis_command(pKV, command);

	if(dave_json_get_sb(pJson, "INTEGER", &command_ret) == dave_false)
		command_ret = -1;
	
	dave_json_free(pJson);
	
	if((command_ret == 0) || (command_ret == 1))
		ret = RetCode_OK;
	else
		ret = RetCode_invalid_option;

	if(ret != RetCode_OK)
	{
		KVLOG("ret:%s", retstr(ret));
	}

	return ret;
}

// ====================================================================

void *
ramkv_redis_connect(KVRedis *pKV)
{
	if(pKV->local_redis_flag == dave_true)
	{
		ramkv_redis_cfg(pKV);

		return _ramkv_redis_connect(pKV->redis_address, pKV->redis_port);
	}
	else
	{
		return NULL;
	}
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
	MBUF *command;
	ub set_command_len = 128 + pKV->table_name_len + key_len + value_len * 5;
	ub set_command_index;
	s8 *set_command_ptr;
	RetCode ret;

	command = dave_mmalloc(set_command_len);
	set_command_ptr = ms8(command);

	set_command_index = dave_snprintf(set_command_ptr, set_command_len, "HSET %s %s ", pKV->table_name_ptr, key_ptr);
	t_crypto_base64_encode((const u8 *)value_ptr, value_len, &(set_command_ptr[set_command_index]), set_command_len-set_command_index);

	ret = _ramkv_redis_bin_add(pKV, command);

	if(ret != RetCode_OK)
	{
		KVLOG("ret:%s", retstr(ret));
		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

sb
ramkv_redis_bin_inq(KVRedis *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len)
{
	MBUF *command;
	ub get_command_len = 128 + pKV->table_name_len + key_len;
	s8 *get_command_ptr;
	ub bin_len = value_len * 10;
	s8 *bin_ptr;

	command = dave_mmalloc(get_command_len);
	bin_ptr = dave_malloc(bin_len);
	get_command_ptr = ms8(command);

	dave_snprintf(get_command_ptr, get_command_len, "HGET %s %s", pKV->table_name_ptr, key_ptr);

	bin_len = _ramkv_redis_bin_inq(pKV, command, bin_ptr, bin_len);
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

	dave_free(bin_ptr);

	return (sb)value_len;
}

dave_bool
ramkv_redis_bin_del(KVRedis *pKV, u8 *key_ptr, ub key_len)
{
	MBUF *command;
	ub del_command_len = 128 + pKV->table_name_len + key_len;
	s8 *del_command_ptr;

	command = dave_mmalloc(del_command_len);
	del_command_ptr = ms8(command);

	dave_snprintf(del_command_ptr, del_command_len, "HDEL %s %s", pKV->table_name_ptr, key_ptr);

	_ramkv_redis_bin_del(pKV, command);

	return dave_true;
}

#endif

