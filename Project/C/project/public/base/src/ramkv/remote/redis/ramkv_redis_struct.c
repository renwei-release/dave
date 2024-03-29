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
#include "dave_base.h"
#include "dave_store.h"
#include "ramkv_param.h"
#include "ramkv_redis_struct.h"
#include "ramkv_redis_api.h"
#include "ramkv_redis_opt.h"
#include "ramkv_redis_cfg.h"
#include "ramkv_log.h"

#define CFG_KV_ON_LOCAL_REDIS "KVOnLocalRedis"

// ====================================================================

dave_bool
ramkv_malloc_redis(KVRedis *pKV, s8 *table_name)
{
	dave_memset(pKV, 0x00, sizeof(KVRedis));

	pKV->local_redis_flag = cfg_get_bool(CFG_KV_ON_LOCAL_REDIS, dave_false);

	dave_strcpy(pKV->table_name_ptr, table_name, sizeof(pKV->table_name_ptr));
	pKV->table_name_len = dave_strlen(pKV->table_name_ptr);

	pKV->store_thread = INVALID_THREAD_ID;
	if(get_thread_flag() & THREAD_COROUTINE_FLAG)
		pKV->has_coroutine = dave_true;
	else
		pKV->has_coroutine = dave_false;

	pKV->redis_context = ramkv_redis_connect(pKV);

	if((pKV->local_redis_flag == dave_true) && (pKV->redis_context == NULL))
	{
		KVABNOR("connect failed! %s:%d", pKV->redis_address, pKV->redis_port);
		return dave_false;
	}

	KVDEBUG("connect success! %s:%d", pKV->redis_address, pKV->redis_port);

	return dave_true;
}

void
ramkv_free_redis(KVRedis *pKV)
{
	ramkv_redis_disconnect(pKV);

	KVDEBUG("disconnect success! %s:%d", pKV->redis_address, pKV->redis_port);

	dave_memset(pKV, 0x00, sizeof(KVRedis));
}

#endif

