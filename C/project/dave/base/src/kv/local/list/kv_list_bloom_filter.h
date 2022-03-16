/*
 * ================================================================================
 * (c) Copyright 2020 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.10.22.
 * ================================================================================
 */

#ifndef __KV_LIST_BLOOM_FILTER_H__
#define __KV_LIST_BLOOM_FILTER_H__
#include "kv_param.h"

void kv_bloom_filter_reset(KVBloomFilter *pFilter);

void kv_bloom_filter_build(KVBloomFilter *pFilter, u8 *key_ptr, ub key_len);

dave_bool kv_bloom_filter_check(KVBloomFilter *pFilter, u8 *key_ptr, ub key_len);

#endif

