/*
 * Copyright (c) 2026 Renwei
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
#include "pgsql_statistics.h"
#include "store_log.h"

// =====================================================================

void
pgsql_statistics_reset(PgsqlStatistics *pStatistics)
{
	dave_memset(pStatistics, 0x00, sizeof(PgsqlStatistics));

	pStatistics->total_sql_time = 0;
	pStatistics->total_sql_times = 0;
	pStatistics->average_sql_time = 0;
	pStatistics->max_sql_time = 0;
	pStatistics->min_sql_time = 9999999999999999;
}

void
pgsql_statistics(PgsqlStatistics *pStatistics, ub consume_time, ThreadId src, s8 *sql)
{
	pStatistics->total_sql_time += consume_time;
	pStatistics->total_sql_times ++;
	pStatistics->average_sql_time = (pStatistics->total_sql_time / pStatistics->total_sql_times);

	if(consume_time > pStatistics->max_sql_time)
		pStatistics->max_sql_time = consume_time;
	if(consume_time < pStatistics->min_sql_time)
		pStatistics->min_sql_time = consume_time;

	if(consume_time > 1000000)
	{
		BDATALOG("PGSQL-SLOW-OPT", "thread:%s sql:%s consumed:%ldus",
			thread_name(src), sql, consume_time);
	}
}

ub
pgsql_statistics_info(s8 *msg_ptr, ub msg_len, PgsqlStatistics *pStatistics)
{
	return dave_snprintf(msg_ptr, msg_len,
		"average:%ldus max:%ldus min:%ldus times:%ld",
		pStatistics->average_sql_time,
		pStatistics->max_sql_time,
		pStatistics->min_sql_time,
		pStatistics->total_sql_times);
}

#endif
