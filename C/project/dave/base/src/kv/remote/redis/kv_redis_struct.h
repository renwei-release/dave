/*
 * ================================================================================
 * (c) Copyright 2020 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.10.22.
 * ================================================================================
 */

#ifndef __KV_REDIS_STRUCT_H__
#define __KV_REDIS_STRUCT_H__

dave_bool kv_malloc_redis(KVRedis *pKV, s8 *table_name);

void kv_free_redis(KVRedis *pKV);

#endif

