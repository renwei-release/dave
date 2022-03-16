/*
 * ================================================================================
 * (c) Copyright 2020 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.10.22.
 * ================================================================================
 */

#ifndef __KV_REDIS_API_H__
#define __KV_REDIS_API_H__

dave_bool kv_redis_add(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len);

ub kv_redis_inq(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len);

ub kv_redis_del(KV *pKV, u8 *key_ptr, ub key_len);

#endif

