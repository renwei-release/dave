/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_STATISTICS_PARAM_H__
#define __THREAD_STATISTICS_PARAM_H__

typedef struct {
	ThreadId msg_src;
	ThreadId msg_dst;
	ub msg_id;

	ub msg_counter;
} ThreadStatistics;

#endif

