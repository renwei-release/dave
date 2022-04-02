/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __KV_REDIS_API_H__
#define __KV_REDIS_API_H__

dave_bool kv_redis_add(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len);

ub kv_redis_inq(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len);

ub kv_redis_del(KV *pKV, u8 *key_ptr, ub key_len);

#endif

