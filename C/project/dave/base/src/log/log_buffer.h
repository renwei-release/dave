/*
 * ================================================================================
 * (c) Copyright 2017 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2017.10.02.
 * ================================================================================
 */

#include "base_macro.h"
#ifndef __LOG_BUFFER_H__
#define __LOG_BUFFER_H__

void log_buffer_init(void);

void log_buffer_exit(void);

void log_buffer_put(TraceLevel level, s8 *log, ub log_len, dave_bool end_flag);

ub log_buffer_get(s8 *log_buf, ub log_buf_len, TraceLevel *level);

ub log_buffer_history(s8 *log_buf, ub log_buf_len);

#endif

