/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __LOG_FIFO_H__
#define __LOG_FIFO_H__

void log_fifo_init(void);

void log_fifo_exit(void);

// Copies accepted data. The 16 MiB budget includes nodes, data terminators,
// pending allocations and in-flight writes; over capacity, new logs are dropped.
// Drops are reported directly to the save backend when the FIFO drains.
void log_fifo(dave_bool trace_enable, dave_bool fix_flag, TraceLevel level, ub data_len, s8 *data_ptr);

// Includes FIFO byte usage, queue depth and total/pending dropped-log counts.
ub log_fifo_info(s8 *info_ptr, ub info_len);

#endif

