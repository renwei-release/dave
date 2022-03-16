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
#include "kv_redis_api.h"
#include "kv_redis_opt.h"
#include "kv_redis_cfg.h"
#include "kv_log.h"

// ====================================================================

dave_bool
kv_malloc_redis(KVRedis *pKV, s8 *table_name)
{
	dave_memset(pKV, 0x00, sizeof(KVRedis));

	kv_redis_cfg(pKV);

	dave_strcpy(pKV->table_name, table_name, sizeof(pKV->table_name));
	pKV->table_name_len = dave_strlen(pKV->table_name);

	pKV->redis_context = kv_redis_connect(pKV);

	return dave_true;
}

void
kv_free_redis(KVRedis *pKV)
{
	kv_redis_disconnect(pKV);

	dave_memset(pKV, 0x00, sizeof(KVRedis));
}

#endif

