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
#include "kv_list_data.h"
#include "kv_log.h"

static inline ub
_kv_hash(u8 *key_ptr, ub key_len)
{
	ub key_index, hash_data;

	key_index = 0;
	hash_data = 0;

	while(key_index < key_len)
	{
		hash_data *= 255;

		hash_data += (ub)(key_ptr[key_index ++]);
	}

	return hash_data % KV_SLOT_NUM;
}

// ====================================================================

dave_bool
kv_hash(KV *pKV, KVHash *pHash, u8 *key_ptr, ub key_len, s8 *fun, ub line)
{
	ub hash_stride, hash_tail_stride, key_index, hash_index, slot_loop;

	if(key_len >= KV_KEY_MAX)
	{
		KVLOG("invalid key_len:%d thread:%s name:%s <%s:%d>",
			key_len, pKV->thread_name, pKV->name, fun, line);
		return dave_false;
	}

	hash_stride = key_len / KV_SLOT_DEPTH;
	if(hash_stride == 0)
	{
		hash_stride = 1;
		hash_tail_stride = 0;
	}
	else
	{
		hash_tail_stride = key_len % KV_SLOT_DEPTH;
	}
	if(hash_tail_stride == 0)
	{
		slot_loop = KV_SLOT_DEPTH;
	}
	else
	{
		slot_loop = KV_SLOT_DEPTH - 1;
	}

	if((hash_stride > KV_KEY_MAX) || (hash_tail_stride > KV_KEY_MAX))
	{
		KVLOG("invalid key_len:%d hash_stride:%d hash_tail_stride:%d slot_depth:%d",
			key_len, hash_stride, hash_tail_stride, slot_loop);		
	}

	key_index = hash_index = 0; 

	while((hash_index < slot_loop) && ((key_index + hash_stride) <= key_len))
	{
		pHash->hash_slot[hash_index ++] = _kv_hash(&key_ptr[key_index], hash_stride);
		key_index += hash_stride;
	}

	if((hash_tail_stride > 0) && ((key_index + hash_tail_stride) <= key_len))
	{
		pHash->hash_slot[hash_index ++] = _kv_hash(&key_ptr[key_index], hash_tail_stride);
	}

	if(hash_index > KV_SLOT_DEPTH)
	{
		KVLOG("invalid hash_index:%d key_len:%d hash_stride:%d hash_tail_stride:%d slot_depth:%d",
			hash_index, key_len, hash_stride, hash_tail_stride, slot_loop);
	}

	while(hash_index < KV_SLOT_DEPTH)
	{
		pHash->hash_slot[hash_index ++] = KV_SLOT_NUM;
	}

	return dave_true;
}

#endif

