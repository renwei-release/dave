/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __KV_REDIS_STRUCT_H__
#define __KV_REDIS_STRUCT_H__

dave_bool kv_malloc_redis(KVRedis *pKV, s8 *table_name);

void kv_free_redis(KVRedis *pKV);

#endif

