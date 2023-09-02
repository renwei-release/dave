/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __QUEUE_CLIENT_MAP_H__
#define __QUEUE_CLIENT_MAP_H__
#include "dave_base.h"
#include "thread_parameter.h"

#define QUEUE_CLIENT_MAP_MAX THREAD_CLIENT_MAX

typedef struct {
	s8 thread_name[DAVE_THREAD_NAME_LEN];
	ub queue_index;
	ub queue_number;
	s8 queue_gid[QUEUE_CLIENT_MAP_MAX][DAVE_GLOBALLY_IDENTIFIER_LEN];

	ub run_req_counter;
	ub run_rsp_counter;

	TLock pv;
} QueueClientMap;

void queue_client_map_init(void);
void queue_client_map_exit(void);

void queue_client_map_add(s8 *thread_name);
void queue_client_map_del(s8 *thread_name);
void queue_client_map_queue_add(QueueClientMap *pMap, s8 *queue_gid);
void queue_client_map_queue_del(QueueClientMap *pMap, s8 *queue_gid);
s8 * queue_client_map_queue_inq(QueueClientMap *pMap, s8 *queue_gid_ptr, ub queue_gid_len, ub queue_index);
void queue_client_map_queue_del_all(s8 *queue_gid);
QueueClientMap * queue_client_map_inq(s8 *thread_name);

void queue_client_gid_map_add(s8 *thread_name, s8 *gid);
void queue_client_gid_map_del(s8 *gid);
QueueClientMap * queue_client_gid_map_inq(s8 *gid);

ub queue_client_map_info(s8 *info_ptr, ub info_len);

#endif

