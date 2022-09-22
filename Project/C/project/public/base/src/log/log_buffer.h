/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifndef __LOG_BUFFER_H__
#define __LOG_BUFFER_H__

#define LOG_BUFFER_MAX (1000)
#define LOG_BUFFER_LENGTH (4096)

void log_buffer_init(void);

void log_buffer_exit(void);

void log_buffer_set(TraceLevel level, s8 *log_ptr, ub log_len);

ub log_buffer_get(s8 *log_ptr, ub log_len, TraceLevel *level);

ub log_buffer_history(s8 *log_ptr, ub log_len);

#endif

