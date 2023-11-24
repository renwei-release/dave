/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __STORE_REDIS_H__
#define __STORE_REDIS_H__

void store_redis_init(ub thread_number);

void store_redis_exit(void);

void store_redis_command(ThreadId src, ub thread_index, StoreRedisReq *pReq);

ub store_redis_info(s8 *info_ptr, ub info_len);

#endif

