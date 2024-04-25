/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __MYSQL_STATISTICS_H__
#define __MYSQL_STATISTICS_H__

typedef struct {
	ub total_sql_time;
	ub total_sql_times;
	ub average_sql_time;
	ub max_sql_time;
	ub min_sql_time;
} MysqlStatistics;

void mysql_statistics_reset(MysqlStatistics *pStatistics);

void mysql_statistics(MysqlStatistics *pStatistics, ub consume_time, ThreadId src, s8 *sql);

ub mysql_statistics_info(s8 *msg_ptr, ub msg_len, MysqlStatistics *pStatistics);

#endif

