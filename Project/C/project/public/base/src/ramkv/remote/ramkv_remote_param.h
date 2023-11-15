/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RAMKV_REMOTE_PARAM_H__
#define __RAMKV_REMOTE_PARAM_H__

typedef struct {
	dave_bool local_redis_flag;

	s8 redis_address[DAVE_URL_LEN];
	ub redis_port;
	s8 redis_password[DAVE_PASSWORD_LEN];
	void *redis_context;

	s8 table_name_ptr[DAVE_NORMAL_NAME_LEN];
	ub table_name_len;
} KVRedis;

typedef struct {
	KVRedis redis;
} KVRemote;

#endif

