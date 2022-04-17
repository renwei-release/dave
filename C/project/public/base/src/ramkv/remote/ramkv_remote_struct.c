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
#include "ramkv_log.h"

static dave_bool
_ramkv_malloc_remote(KV *pKV, s8 *name, KvAttrib attrib)
{
	dave_bool ret = dave_false;

	if(attrib & KvAttrib_remote)
	{
		ret = ramkv_malloc_redis(&(pKV->remote.redis), name);
	}

	return ret;
}

static void
_ramkv_free_remote(KV *pKV, KvAttrib attrib)
{
	if(attrib & KvAttrib_remote)
	{
		ramkv_free_redis(&(pKV->remote.redis));
	}
}

// ====================================================================

dave_bool
ramkv_malloc_remote(KV *pKV, s8 *name, KvAttrib attrib)
{
	dave_bool ret = dave_false;

	SAFECODEv2W( pKV->ramkv_pv, ret = _ramkv_malloc_remote(pKV, name, attrib); );

	return ret;
}

void
ramkv_free_remote(KV *pKV, KvAttrib attrib)
{
	SAFECODEv2W( pKV->ramkv_pv, _ramkv_free_remote(pKV, attrib); );
}

#endif

