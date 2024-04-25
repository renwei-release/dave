/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __REDIS_STATISTICS_H__
#define __REDIS_STATISTICS_H__

typedef struct {
	ub total_redis_time;
	ub total_redis_times;
	ub average_redis_time;
	ub max_redis_time;
	ub min_redis_time;
} RedisStatistics;

void redis_statistics_reset(RedisStatistics *pStatistics);

void redis_statistics(RedisStatistics *pStatistics, ub consume_time, ThreadId src, s8 *command);

ub redis_statistics_info(s8 *msg_ptr, ub msg_len, RedisStatistics *pStatistics);

#endif

