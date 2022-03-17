/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __KV_REMOTE_PARAM_H__
#define __KV_REMOTE_PARAM_H__

typedef struct {
	s8 redis_address[DAVE_URL_LEN];
	ub redis_port;

	s8 table_name[DAVE_NORMAL_NAME_LEN];
	ub table_name_len;

	void *redis_context;
} KVRedis;

typedef struct {
	KVRedis redis;
} KVRemote;

#endif

