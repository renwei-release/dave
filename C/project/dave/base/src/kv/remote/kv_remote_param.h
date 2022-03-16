/*
 * ================================================================================
 * (c) Copyright 2020 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2021.04.23.
 * ================================================================================
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

