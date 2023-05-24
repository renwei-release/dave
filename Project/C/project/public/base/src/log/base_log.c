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
#include "dave_verno.h"
#include "log_lock.h"
#include "log_buffer.h"
#include "log_stack.h"
#include "log_trace.h"
#include "log_fifo.h"
#include "log_param.h"
#include "log_log.h"
#undef vsnprintf
#include <stdio.h>
#include <stdlib.h>

static s8 _trace_buffer[LOG_BUFFER_LENGTH];
static ub _log_log_counter = 0;
static dave_bool _log_trace_enable = LOG_TRACE_DEFAULT_CFG;

static inline ub
_log_buffer_counter(void)
{
	ub counter;

	log_lock();
	counter = _log_log_counter ++;
	log_unlock();

	return counter;
}

static inline void
_log_buffer_log_head(LogBuffer *pBuffer, TraceLevel level)
{
	DateStruct date = t_time_get_date(NULL);

	pBuffer->level = level;

	pBuffer->buffer_length = dave_snprintf(pBuffer->buffer_ptr, LOG_BUFFER_LENGTH,
		"(%s.%s.%s)<%04d.%02d.%02d %02d:%02d:%02d>{%lu}",
		VERSION_MAIN, VERSION_SUB, VERSION_REV,
		date.year, date.month, date.day, date.hour, date.minute, date.second,
		_log_buffer_counter());
}

static inline s8 *
__log_buffer__(ub *log_len, TraceLevel level, const char *fmt, va_list list_args)
{
	LogBuffer *pBuffer;
	s8 *log_buf;

	log_buf = NULL;
	*log_len = 0;

	pBuffer = log_buffer_thread();
	if(pBuffer == NULL)
	{
		return NULL;
	}

	if((pBuffer->buffer_length == 0) || (pBuffer->buffer_length >= LOG_BUFFER_LENGTH))
	{
		_log_buffer_log_head(pBuffer, level);
	}

	pBuffer->buffer_length += (ub)vsnprintf(&pBuffer->buffer_ptr[pBuffer->buffer_length], LOG_BUFFER_LENGTH-pBuffer->buffer_length, fmt, list_args);
	if(pBuffer->buffer_length < LOG_BUFFER_LENGTH)
	{
		pBuffer->buffer_ptr[pBuffer->buffer_length] = '\0';
	}

	if((pBuffer->buffer_ptr[pBuffer->buffer_length - 1] == '\n')
		|| (pBuffer->buffer_ptr[pBuffer->buffer_length - 1] == '\r')
		|| ((pBuffer->buffer_length + 32) >= LOG_BUFFER_LENGTH))
	{
		log_buffer_set(pBuffer);

		log_buf = pBuffer->buffer_ptr;
		*log_len = pBuffer->buffer_length;
	}

	return log_buf;
}

static inline s8 *
__log_trace__(ub *log_len, TraceLevel level, const char *fmt, va_list list_args)
{
	ub trace_len = dave_strlen(_trace_buffer);

	if((_trace_buffer[trace_len - 1] == '\r')
		|| (_trace_buffer[trace_len - 1] == '\n')
		|| (trace_len > (sizeof(_trace_buffer) - 128)))
	{
		trace_len = 0;
	}

	trace_len += (ub)vsnprintf(&_trace_buffer[trace_len], sizeof(_trace_buffer)-trace_len, fmt, list_args);

	if((_trace_buffer[trace_len - 1] == '\r')
		|| (_trace_buffer[trace_len - 1] == '\n')
		|| (trace_len > (sizeof(_trace_buffer) - 128)))
	{
		*log_len = trace_len;
		return _trace_buffer;
	}
	else
	{
		*log_len = 0;
		return NULL;
	}
}

static inline s8 *
__log_log__(TraceLevel level, const char *fmt, va_list list_args)
{
	s8 *log_buf;
	ub log_len;

	if(fmt == NULL)
	{
		return NULL;
	}

	if((level == TRACELEVEL_DEBUG) || (level == TRACELEVEL_ASSERT))
	{
		log_buf = __log_trace__(&log_len, level, fmt, list_args);
	}
	else
	{
		log_buf = __log_buffer__(&log_len, level, fmt, list_args);
	}

	log_fifo(_log_trace_enable, level, log_len, log_buf);

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
		s8 args_str[1024];
		s8 poweroff_message[2048];
		s8 *log_buf;
		va_list list_args;

		dave_snprintf(args_str, sizeof(args_str), "%s\n", args);

		va_start(list_args, args);
		log_buf = __log_log__(TRACELEVEL_ASSERT, args_str, list_args);
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
base_log_line_enable(s8 *fun, ub line, ub second, ub number)
{
	return log_trace_line_enable(fun, line, second, number);
}

ub
base_log_load(s8 *log_ptr, ub log_len, TraceLevel *level)
{
	return log_buffer_get(log_ptr, log_len, level);
}

ub
base_log_history(s8 *log_ptr, ub log_len)
{
	return log_buffer_history(log_ptr, log_len);
}

void
base_log_init(void)
{
	dave_memset(_trace_buffer, 0x00, sizeof(_trace_buffer));
	_log_log_counter = 0;
	_log_trace_enable = cfg_get_bool(CFG_LOG_TRACE_ENABLE, LOG_TRACE_DEFAULT_CFG);

	log_trace_init();
	log_buffer_init();
}

void
base_log_exit(void)
{
	log_buffer_exit();
	log_trace_exit();
}

void
base_log_trace_enable(dave_bool write_cfg)
{
	_log_trace_enable = dave_true;

	if(write_cfg == dave_true)
	{
		cfg_set_bool(CFG_LOG_TRACE_ENABLE, _log_trace_enable);
	}

	LOGLOG("log trace enable!");
}

void
base_log_trace_disable(dave_bool write_cfg)
{
	LOGLOG("log trace disable!");

	_log_trace_enable = dave_false;

	if(write_cfg == dave_true)
	{
		cfg_set_bool(CFG_LOG_TRACE_ENABLE, _log_trace_enable);
	}
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

