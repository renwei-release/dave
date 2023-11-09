/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifndef __LOG_BUFFER_H__
#define __LOG_BUFFER_H__

// Equivalent to (LOG_BUFFER_MAX) logs per second
#define LOG_BUFFER_MAX (2048)

#define LOG_FIX_BUFFER_LEN (512)
#define LOG_DYNAMIC_BUFFER_LEN (1024 * 128)

typedef struct {
	TraceLevel level;

	s8 fix_buffer_ptr[LOG_FIX_BUFFER_LEN];
	ub fix_buffer_index;

	s8 *dynamic_buffer_ptr;
	ub dynamic_buffer_len;
	ub dynamic_buffer_index;

	ub tid;
} LogBuffer;

void log_buffer_init(void);

void log_buffer_exit(void);

LogBuffer * log_buffer_thread(ub buffer_len);

void log_buffer_transfer(LogBuffer *pBuffer, ub buffer_len);

void log_buffer_set(LogBuffer *pBuffer);

ub log_buffer_get(s8 *log_ptr, ub log_len, TraceLevel *level);

dave_bool log_buffer_has_data(void);

ub log_buffer_history(s8 *log_ptr, ub log_len);

#endif

