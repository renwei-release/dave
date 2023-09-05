/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DISTRIBUTOR_INFO_H__
#define __DISTRIBUTOR_INFO_H__
#include "http_param.h"

typedef struct {
	s8 thread_name[DAVE_THREAD_NAME_LEN];
	s8 path[DAVE_PATH_LEN];
	ub listening_seconds_life;
	ub listening_seconds_time;

	TLock pv;
	ub receive_counter;
} HttpDistributorInfo;

void distributor_info_init(void);

void distributor_info_exit(void);

dave_bool distributor_info_malloc(ThreadId src, s8 *path, ub listening_seconds_time);

void distributor_info_free(s8 *path);

HttpDistributorInfo * distributor_info_inq(s8 *path);

ub distributor_info_info(s8 *info_ptr, ub info_len);

#endif

