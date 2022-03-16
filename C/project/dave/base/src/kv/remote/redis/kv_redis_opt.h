/*
 * ================================================================================
 * (c) Copyright 2020 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.10.22.
 * ================================================================================
 */

#ifndef __KV_REDIS_OPT_H__
#define __KV_REDIS_OPT_H__

void * kv_redis_connect(KVRedis *pKV);

void kv_redis_disconnect(KVRedis *pKV);

dave_bool kv_redis_bin_add(KVRedis *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len);

ub kv_redis_bin_inq(KVRedis *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len);

dave_bool kv_redis_bin_del(KVRedis *pKV, u8 *key_ptr, ub key_len);

#endif

