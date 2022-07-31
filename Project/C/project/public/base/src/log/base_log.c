/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "dave_os.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "log_lock.h"
#include "log_buffer.h"
#include "log_stack.h"
#include "log_trace.h"
#include <stdio.h>
#include <stdlib.h>

#define MAXBUF (32)
#define MAXCHARS (LOG_BUFFER_LENGTH)

static s8 _trace_buf[MAXBUF][MAXCHARS];
static ub _trace_index = 0;

static inline s8 *
__log_log__(TraceLevel level, const char *fmt, va_list list_args)
{
	char *log_buf;
	ub log_len;

	if(fmt == NULL)
	{
		return NULL;
	}

	log_lock();
	log_buf = _trace_buf[(_trace_index ++) % MAXBUF];
	log_unlock();

	log_len = (ub)vsnprintf(log_buf, MAXCHARS, fmt, list_args);

	if(level != TRACELEVEL_CATCHER)
	{
		dave_os_trace(level, log_len, (u8 *)log_buf);
	}

	if(level != TRACELEVEL_DEBUG)
	{
		log_buffer_set(level, log_buf, log_len);
	}

	return log_buf;
}

// =====================================================================

void
__base_debug__(const char *args, ...)
{
	va_list list_args;

	va_start(list_args, args);
	__log_log__(TRACELEVEL_DEBUG, args, list_args);
	va_end(list_args);
}

void
__base_catcher__(const char *args, ...)
{
	va_list list_args;

	va_start(list_args, args);
	__log_log__(TRACELEVEL_CATCHER, args, list_args);
	va_end(list_args);
}

void
__base_trace__(const char *args, ...)
{
	va_list list_args;

	va_start(list_args, args);
	__log_log__(TRACELEVEL_TRACE, args, list_args);
	va_end(list_args);
}

void
__base_log__(const char *args, ...)
{
	va_list list_args;

	va_start(list_args, args);
	__log_log__(TRACELEVEL_LOG, args, list_args);
	va_end(list_args);
}

void
__base_abnormal__(const char *args, ...)
{
	va_list list_args;

	va_start(list_args, args);
	__log_log__(TRACELEVEL_ABNORMAL, args, list_args);
	va_end(list_args);
}

void
__base_assert__(int assert_flag, const char *fun, int line, const char *args, ...)
{
	if(assert_flag == 0)
	{
		s8 poweroff_message[2048];
		s8 *log_buf;
		va_list list_args;

		va_start(list_args, args);
		log_buf = __log_log__(TRACELEVEL_ASSERT, args, list_args);
		va_end(list_args);

		dave_snprintf(poweroff_message, sizeof(poweroff_message), "%s:%d:%s", fun, line, log_buf);

		base_power_off(poweroff_message);
	}
}

dave_bool
base_log_id_add(s8 *trace_id)
{
	return log_trace_add_id(trace_id);
}

dave_bool
base_log_id_del(s8 *trace_id)
{
	return log_trace_del_id(trace_id);
}

dave_bool
base_log_id_enable(s8 *trace_id)
{
	return log_trace_id_enable(trace_id);
}

dave_bool
base_log_line_enable(s8 *fun, ub line, ub time, ub number)
{
	return log_trace_line_enable(fun, line, time, number);
}

ub
base_log_load(s8 *log_buf, ub log_buf_len, TraceLevel *level)
{
	if((log_buf == NULL) || (log_buf_len == 0))
	{
        if(log_buf != NULL)
        {
            log_buf[0] = '\0';
        }
		*level = TRACELEVEL_MAX;
		return 0;
	}
	else
	{
		return log_buffer_get(log_buf, log_buf_len, level);
	}
}

void
base_log_init(void)
{
	log_trace_init();
	dave_memset(_trace_buf, 0x00, sizeof(_trace_buf));
	_trace_index = 0;
	log_buffer_init();
}

void
base_log_exit(void)
{
	log_buffer_exit();
	_trace_index = 0;
	log_trace_exit();
}

void
base_log_stack_init(void)
{
	log_stack_init();
}

void
base_log_stack_exit(void)
{
	log_stack_exit();
}

