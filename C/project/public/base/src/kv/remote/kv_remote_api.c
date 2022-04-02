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
#include "kv_param.h"
#include "kv_redis_struct.h"
#include "kv_redis_opt.h"
#include "kv_redis_api.h"
#include "kv_log.h"

// ====================================================================

dave_bool
kv_remote_add(KVAttrib attrib, KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len)
{
	if(attrib & KVAttrib_remote)
	{
		return kv_redis_add(pKV, key_ptr, key_len, value_ptr, value_len);
	}

	return dave_false;
}

ub
kv_remote_inq(KVAttrib attrib, KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len)
{
	if(attrib & KVAttrib_remote)
	{
		return kv_redis_inq(pKV, key_ptr, key_len, value_ptr, value_len);
	}

	return 0;
}

ub
kv_remote_del(KVAttrib attrib, KV *pKV, u8 *key_ptr, ub key_len)
{
	if(attrib & KVAttrib_remote)
	{
		return kv_redis_del(pKV, key_ptr, key_len);
	}

	return 0;
}

#endif

