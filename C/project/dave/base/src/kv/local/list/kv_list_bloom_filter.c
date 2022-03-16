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
#include "kv_list_data.h"
#include "kv_list_slot.h"
#include "kv_list_hash.h"
#include "kv_list_tools.h"
#include "kv_log.h"

#define KV_BLOOM_FILTER_BIT_MAX (KV_BLOOM_FILTER_MAX * 64)

static inline u64
_kv_bloom_filter_hash(u8 *key_ptr, ub key_len)
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
_kv_bloom_filter_index(u64 *filter_index, u64 *filter_bit, u8 *key_ptr, ub key_len)
{
	u64 filter_hash = _kv_bloom_filter_hash(key_ptr, key_len);

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
kv_bloom_filter_reset(KVBloomFilter *pFilter)
{
	ub filter_index;

	dave_memset(pFilter, 0x00, sizeof(KVBloomFilter));

	for(filter_index=0; filter_index<KV_BLOOM_FILTER_MAX; filter_index++)
	{
		pFilter->bloom_filter[filter_index] = 0x00;
	}
}

void
kv_bloom_filter_build(KVBloomFilter *pFilter, u8 *key_ptr, ub key_len)
{
	u64 filter_index, filter_bit;

	_kv_bloom_filter_index(&filter_index, &filter_bit, key_ptr, key_len);

	pFilter->bloom_filter[filter_index] |= (0x1 << filter_bit);
}

dave_bool
kv_bloom_filter_check(KVBloomFilter *pFilter, u8 *key_ptr, ub key_len)
{
	u64 filter_index, filter_bit, bloom_filter;

	_kv_bloom_filter_index(&filter_index, &filter_bit, key_ptr, key_len);

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

