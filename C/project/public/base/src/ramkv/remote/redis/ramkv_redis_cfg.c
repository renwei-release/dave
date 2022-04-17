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
#include "ramkv_log.h"

#define CFG_REDIS_ADDRESS "REDISADDRESS"
#define CFG_REDIS_PORT "REDISPORT"
#define CFG_REDIS_PWD "REDISPASSWORD"

static void
_ramkv_redis_cfg(KVRedis *pKV)
{
	s8 redis_port[32];

	if(cfg_get(CFG_REDIS_ADDRESS, (u8 *)pKV->redis_address, DAVE_URL_LEN) == dave_false)
	{
		dave_snprintf(pKV->redis_address, sizeof(pKV->redis_address), "%s", t_gp_localhost());
		cfg_set(CFG_REDIS_ADDRESS, (u8 *)pKV->redis_address, dave_strlen(pKV->redis_address));
	}
	if(cfg_get(CFG_REDIS_PORT, (u8 *)redis_port, 32) == dave_false)
	{
		dave_sprintf(redis_port, "6379");
		cfg_set(CFG_REDIS_PORT, (u8 *)redis_port, dave_strlen(redis_port));
	}
	pKV->redis_port = stringdigital(redis_port);
}

// ====================================================================

void
ramkv_redis_cfg(KVRedis *pKV)
{
	_ramkv_redis_cfg(pKV);
}

#endif

