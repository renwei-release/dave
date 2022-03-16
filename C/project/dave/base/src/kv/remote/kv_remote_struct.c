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
#include "kv_log.h"

static dave_bool
_kv_malloc_remote(KV *pKV, s8 *name, KVAttrib attrib)
{
	dave_bool ret = dave_false;

	if(attrib & KVAttrib_remote)
	{
		ret = kv_malloc_redis(&(pKV->remote.redis), name);
	}

	return ret;
}

static void
_kv_free_remote(KV *pKV, KVAttrib attrib)
{
	if(attrib & KVAttrib_remote)
	{
		kv_free_redis(&(pKV->remote.redis));
	}
}

// ====================================================================

dave_bool
kv_malloc_remote(KV *pKV, s8 *name, KVAttrib attrib)
{
	dave_bool ret = dave_false;

	SAFEZONEv5W( pKV->kv_pv, ret = _kv_malloc_remote(pKV, name, attrib); );

	return ret;
}

void
kv_free_remote(KV *pKV, KVAttrib attrib)
{
	SAFEZONEv5W( pKV->kv_pv, _kv_free_remote(pKV, attrib); );
}

#endif

