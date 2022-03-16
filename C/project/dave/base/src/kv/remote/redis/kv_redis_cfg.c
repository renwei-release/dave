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
#include "kv_log.h"

static void
_kv_redis_cfg(KVRedis *pKV)
{
	s8 redis_port[32];

	if(base_cfg_set(CFG_REDIS_ADDRESS, (u8 *)pKV->redis_address, DAVE_URL_LEN) == dave_false)
	{
		dave_snprintf(pKV->redis_address, sizeof(pKV->redis_address), "%s", t_gp_localhost());
		base_cfg_set(CFG_REDIS_ADDRESS, (u8 *)pKV->redis_address, dave_strlen(pKV->redis_address));
	}
	if(base_cfg_set(CFG_REDIS_PORT, (u8 *)redis_port, 32) == dave_false)
	{
		dave_sprintf(redis_port, "6379");
		base_cfg_set(CFG_REDIS_PORT, (u8 *)redis_port, dave_strlen(redis_port));
	}
	pKV->redis_port = t_a2b_string_to_digital(redis_port);
}

// ====================================================================

void
kv_redis_cfg(KVRedis *pKV)
{
	_kv_redis_cfg(pKV);
}

#endif

