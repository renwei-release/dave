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
#include "ramkv_list_data.h"
#include "ramkv_list_slot.h"
#include "ramkv_list_hash.h"
#include "ramkv_list_tools.h"
#include "ramkv_log.h"

#define KV_BLOOM_FILTER_BIT_MAX (KV_BLOOM_FILTER_MAX * 64)

static inline u64
_ramkv_bloom_filter_hash(u8 *key_ptr, ub key_len)
{
	unsigned int *int_ptr = (unsigned int *)key_ptr;
	u64 hash_data;

	hash_data = 0;

	while(key_len >= 4)
	{
		hash_data += (ub)(*(int_ptr ++));
		key_len -= 4;
	}

	return hash_data % KV_BLOOM_FILTER_BIT_MAX;
}

static inline void
_ramkv_bloom_filter_index(u64 *filter_index, u64 *filter_bit, u8 *key_ptr, ub key_len)
{
	u64 filter_hash = _ramkv_bloom_filter_hash(key_ptr, key_len);

	*filter_index = filter_hash / 64;
	if(*filter_index >= KV_BLOOM_FILTER_MAX)
	{
		KVABNOR("invalid filter_index:%d", *filter_index);
		*filter_index = 0;
	}

	*filter_bit = filter_hash % 64;
}

// ====================================================================

void
ramkv_bloom_filter_reset(KVBloomFilter *pFilter)
{
	ub filter_index;

	dave_memset(pFilter, 0x00, sizeof(KVBloomFilter));

	for(filter_index=0; filter_index<KV_BLOOM_FILTER_MAX; filter_index++)
	{
		pFilter->bloom_filter[filter_index] = 0x00;
	}
}

void
ramkv_bloom_filter_build(KVBloomFilter *pFilter, u8 *key_ptr, ub key_len)
{
	u64 filter_index, filter_bit;

	_ramkv_bloom_filter_index(&filter_index, &filter_bit, key_ptr, key_len);

	pFilter->bloom_filter[filter_index] |= (0x1 << filter_bit);
}

dave_bool
ramkv_bloom_filter_check(KVBloomFilter *pFilter, u8 *key_ptr, ub key_len)
{
	u64 filter_index, filter_bit, bloom_filter;

	_ramkv_bloom_filter_index(&filter_index, &filter_bit, key_ptr, key_len);

	bloom_filter = (0x1 << filter_bit);

	if(pFilter->bloom_filter[filter_index] & bloom_filter)
	{
		return dave_true;
	}
	else
	{
		return dave_false;
	}
}

#endif

