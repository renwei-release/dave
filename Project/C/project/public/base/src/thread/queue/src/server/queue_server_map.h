/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __QUEUE_SERVER_MAP_H__
#define __QUEUE_SERVER_MAP_H__
#include "dave_base.h"
#include "thread_parameter.h"

#define QUEUE_SERVER_MAP_MAX THREAD_CLIENT_MAX

typedef struct {
	s8 thread_name[DAVE_THREAD_NAME_LEN];

	ub client_index;
	ub client_number;
	s8 client_gid[QUEUE_SERVER_MAP_MAX][DAVE_GLOBALLY_IDENTIFIER_LEN];
} QueueServerMap;

void queue_server_map_init(void);
void queue_server_map_exit(void);

void queue_server_map_add(s8 *thread_name, s8 *gid);
void queue_server_map_del(s8 *thread_name, s8 *gid);
QueueServerMap * queue_server_map_inq(s8 *thread_name);

ub queue_server_map_info(s8 *info_ptr, ub info_len);

#endif

