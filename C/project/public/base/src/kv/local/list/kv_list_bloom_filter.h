/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __KV_LIST_BLOOM_FILTER_H__
#define __KV_LIST_BLOOM_FILTER_H__
#include "kv_param.h"

void kv_bloom_filter_reset(KVBloomFilter *pFilter);

void kv_bloom_filter_build(KVBloomFilter *pFilter, u8 *key_ptr, ub key_len);

dave_bool kv_bloom_filter_check(KVBloomFilter *pFilter, u8 *key_ptr, ub key_len);

#endif
