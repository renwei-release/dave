/*
 * Copyright (c) 2026 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __ORACLE_STATISTICS_H__
#define __ORACLE_STATISTICS_H__

typedef struct {
	ub total_sql_time;
	ub total_sql_times;
	ub average_sql_time;
	ub max_sql_time;
	ub min_sql_time;
} OracleStatistics;

void oracle_statistics_reset(OracleStatistics *pStatistics);

void oracle_statistics(OracleStatistics *pStatistics, ub consume_time, ThreadId src, s8 *sql);

ub oracle_statistics_info(s8 *msg_ptr, ub msg_len, OracleStatistics *pStatistics);

#endif
