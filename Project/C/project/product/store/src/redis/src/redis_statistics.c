/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_PRODUCT_STORE__
#include "dave_tools.h"
#include "dave_3rdparty.h"
#include "dave_verno.h"
#include "dave_base.h"
#include "dave_bdata.h"
#include "store_msg.h"
#include "redis_statistics.h"
#include "store_log.h"

// =====================================================================

void
redis_statistics_reset(RedisStatistics *pStatistics)
{
	dave_memset(pStatistics, 0x00, sizeof(RedisStatistics));

	pStatistics->total_redis_time = 0;
	pStatistics->total_redis_times = 0;
	pStatistics->average_redis_time = 0;
	pStatistics->max_redis_time = 0;
	pStatistics->min_redis_time = 9999999999999999;
}

void
redis_statistics(RedisStatistics *pStatistics, ub consume_time, ThreadId src, s8 *command)
{
	pStatistics->total_redis_time += consume_time;
	pStatistics->total_redis_times ++;
	pStatistics->average_redis_time = (pStatistics->total_redis_time / pStatistics->total_redis_times);

	if(consume_time > pStatistics->max_redis_time)
		pStatistics->max_redis_time = consume_time;
	if(consume_time < pStatistics->min_redis_time)
		pStatistics->min_redis_time = consume_time;

	if(consume_time > 1000000)
	{
		BDATALOG("REDIS-SLOW-OPT", "thread:%s command:%s consumed:%ldus",
			thread_name(src), command, consume_time);
	}
}

ub
redis_statistics_info(s8 *msg_ptr, ub msg_len, RedisStatistics *pStatistics)
{
	return dave_snprintf(msg_ptr, msg_len,
		"average:%ldus max:%ldus min:%ldus times:%ld",
		pStatistics->average_redis_time,
		pStatistics->max_redis_time,
		pStatistics->min_redis_time,
		pStatistics->total_redis_times);
}

#endif

