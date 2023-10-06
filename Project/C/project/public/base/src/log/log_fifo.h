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

void log_fifo(dave_bool trace_enable, TraceLevel level, ub data_len, s8 *data_ptr);

ub log_fifo_info(s8 *info_ptr, ub info_len);

#endif

