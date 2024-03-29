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

// ====================================================================

dave_bool
ramkv_redis_add(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len)
{
	dave_bool ret = dave_false;

	SAFECODEv2W(pKV->ramkv_pv, ret = ramkv_redis_bin_add(&(pKV->remote.redis), key_ptr, key_len, value_ptr, value_len); );

	return ret;
}

sb
ramkv_redis_inq(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len)
{
	sb ret_value_len = -1;

	SAFECODEv2R(pKV->ramkv_pv, ret_value_len = ramkv_redis_bin_inq(&(pKV->remote.redis), key_ptr, key_len, value_ptr, value_len); );

	return ret_value_len;
}

ub
ramkv_redis_del(KV *pKV, u8 *key_ptr, ub key_len)
{
	dave_bool ret = dave_false;

	SAFECODEv2W(pKV->ramkv_pv, ret = ramkv_redis_bin_del(&(pKV->remote.redis), key_ptr, key_len); );

	if(ret == dave_false)
		return 0;
	else
		return 1;
}

#endif

