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

#define CFG_REDIS_ADDRESS "REDISADDRESS"
#define CFG_REDIS_PORT "REDISPORT"
#define CFG_REDIS_PWD "REDISPASSWORD"

static void
_ramkv_redis_cfg(KVRedis *pKV)
{
	cfg_get_str(CFG_REDIS_ADDRESS, pKV->redis_address, sizeof(pKV->redis_address), t_gp_localhost());
	pKV->redis_port = cfg_get_ub(CFG_REDIS_PORT, 6379);
	cfg_get_str(CFG_REDIS_PWD, pKV->redis_password, sizeof(pKV->redis_password), "");
}

// ====================================================================

void
ramkv_redis_cfg(KVRedis *pKV)
{
	_ramkv_redis_cfg(pKV);
}

#endif

