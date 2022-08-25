/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RAMKV_REDIS_OPT_H__
#define __RAMKV_REDIS_OPT_H__

void * ramkv_redis_connect(KVRedis *pKV);

void ramkv_redis_disconnect(KVRedis *pKV);

dave_bool ramkv_redis_bin_add(KVRedis *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len);

sb ramkv_redis_bin_inq(KVRedis *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len);

dave_bool ramkv_redis_bin_del(KVRedis *pKV, u8 *key_ptr, ub key_len);

#endif

