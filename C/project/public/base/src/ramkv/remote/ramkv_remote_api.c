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
ramkv_remote_add(KvAttrib attrib, KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len)
{
	if(attrib & KvAttrib_remote)
	{
		return ramkv_redis_add(pKV, key_ptr, key_len, value_ptr, value_len);
	}

	return dave_false;
}

ub
ramkv_remote_inq(KvAttrib attrib, KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len)
{
	if(attrib & KvAttrib_remote)
	{
		return ramkv_redis_inq(pKV, key_ptr, key_len, value_ptr, value_len);
	}

	return 0;
}

ub
ramkv_remote_del(KvAttrib attrib, KV *pKV, u8 *key_ptr, ub key_len)
{
	if(attrib & KvAttrib_remote)
	{
		return ramkv_redis_del(pKV, key_ptr, key_len);
	}

	return 0;
}

#endif

