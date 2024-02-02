/*
 * Copyright (c) 2023 Renwei
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
#include <stdarg.h>

#define TRACE_BUFFER_MAX 16
#define TRACE_BUFFER_LEN 1024

static s8 _trace_buffer[TRACE_BUFFER_MAX][TRACE_BUFFER_LEN];
static ub _trace_index = 0;
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

	pBuffer->fix_buffer_index = dave_snprintf(pBuffer->fix_buffer_ptr, LOG_FIX_BUFFER_LEN,
		"(%s.%s.%s)<%04d.%02d.%02d %02d:%02d:%02d>{%lu}",
		VERSION_MAIN, VERSION_SUB, VERSION_REV,
		date.year, date.month, date.day, date.hour, date.minute, date.second,
		_log_buffer_counter());
}

static inline void
_log_buffer_dynamic(void)
{
	log_buffer_transfer(LOG_DYNAMIC_BUFFER_LEN);
}

static inline s8 *
___log_trace___(ub *log_len, TraceLevel level, const char *fmt, va_list list_args)
{
	s8 *trace_buffer = _trace_buffer[_trace_index % TRACE_BUFFER_MAX];
	ub trace_len = dave_strlen(trace_buffer);

	if((trace_buffer[trace_len - 1] == '\r')
		|| (trace_buffer[trace_len - 1] == '\n')
		|| (trace_len > (TRACE_BUFFER_LEN - 128)))
	{
		trace_len = 0;
	}

	trace_len += (ub)vsnprintf(&trace_buffer[trace_len], TRACE_BUFFER_LEN-trace_len, fmt, list_args);

	if((trace_buffer[trace_len - 1] == '\r')
		|| (trace_buffer[trace_len - 1] == '\n')
		|| (trace_len > (TRACE_BUFFER_LEN - 128)))
	{
		_trace_index ++;
		*log_len = trace_len;
		return trace_buffer;
	}
	else
	{
		*log_len = 0;
		return NULL;
	}
}

static inline dave_bool
__log_trace__(dave_bool *fix_flag, s8 **log_buf, ub *log_len, TraceLevel level, const char *fmt, va_list list_args)
{
	log_lock();
	*log_buf = ___log_trace___(log_len, level, fmt, list_args);
	log_unlock();

	*fix_flag = dave_true;

	return dave_true;
}

static inline dave_bool
___log_buffer___(dave_bool *fix_flag, dave_bool *end_flag, LogBuffer *pBuffer, const char *fmt, va_list list_args)
{
	*end_flag = dave_false;

	if(pBuffer->dynamic_buffer_ptr == NULL)
	{
		size_t buffer_len = LOG_FIX_BUFFER_LEN - pBuffer->fix_buffer_index;
		int vsnprintf_len;

		vsnprintf_len = vsnprintf(&pBuffer->fix_buffer_ptr[pBuffer->fix_buffer_index], buffer_len, fmt, list_args);
		/*
		 * Maybe the fixed BUF is too small.
		 */
		if((vsnprintf_len + 8) >= buffer_len)
		{
			return dave_false;
		}
		pBuffer->fix_buffer_index += vsnprintf_len;

		if(pBuffer->fix_buffer_index < LOG_FIX_BUFFER_LEN)
		{
			pBuffer->fix_buffer_ptr[pBuffer->fix_buffer_index] = '\0';
		}

		if((pBuffer->fix_buffer_ptr[pBuffer->fix_buffer_index - 1] == '\n')
			|| (pBuffer->fix_buffer_ptr[pBuffer->fix_buffer_index - 1] == '\r'))
		{
			*end_flag = dave_true;
			log_buffer_set(pBuffer);
		}

		*fix_flag = dave_true;

		return dave_true;
	}
	else
	{
		size_t buffer_len = pBuffer->dynamic_buffer_len - pBuffer->dynamic_buffer_index;
		int vsnprintf_len;

		vsnprintf_len = (ub)vsnprintf(&pBuffer->dynamic_buffer_ptr[pBuffer->dynamic_buffer_index], buffer_len, fmt, list_args);
		if((vsnprintf_len + 8) >= buffer_len)
		{
			return dave_false;
		}		
		pBuffer->dynamic_buffer_index += vsnprintf_len;

		if(pBuffer->dynamic_buffer_index < pBuffer->dynamic_buffer_len)
		{
			pBuffer->dynamic_buffer_ptr[pBuffer->dynamic_buffer_index] = '\0';
		}
	
		if((pBuffer->dynamic_buffer_ptr[pBuffer->dynamic_buffer_index - 1] == '\n')
			|| (pBuffer->dynamic_buffer_ptr[pBuffer->dynamic_buffer_index - 1] == '\r')
			|| ((pBuffer->dynamic_buffer_index + 32) >= pBuffer->dynamic_buffer_len))
		{
			*end_flag = dave_true;
			log_buffer_set(pBuffer);
		}

		*fix_flag = dave_false;

		return dave_true;
	}
}

static inline dave_bool
__log_buffer__(dave_bool *fix_flag, s8 **log_buf, ub *log_len, TraceLevel level, const char *fmt, va_list list_args)
{
	LogBuffer *pBuffer;
	dave_bool end_flag;

	*log_buf = NULL;
	*log_len = 0;
	*fix_flag = end_flag = dave_false;

	pBuffer = log_buffer_thread();
	if(pBuffer == NULL)
	{
		return dave_true;
	}

	if((pBuffer->fix_buffer_index == 0) && (pBuffer->dynamic_buffer_index == 0))
	{
		_log_buffer_log_head(pBuffer, level);
	}

	if(___log_buffer___(fix_flag, &end_flag, pBuffer, fmt, list_args) == dave_false)
	{
		return dave_false;
	}

	if(end_flag == dave_true)
	{
		if(pBuffer->dynamic_buffer_ptr == NULL)
		{
			*log_buf = pBuffer->fix_buffer_ptr;
			*log_len = pBuffer->fix_buffer_index;
		}
		else
		{
			*log_buf = pBuffer->dynamic_buffer_ptr;
			*log_len = pBuffer->dynamic_buffer_index;
		}
	}

	return dave_true;
}

static inline dave_bool
__log_log__(s8 **log_buf, TraceLevel level, const char *fmt, va_list list_args)
{
	ub log_len = 0;
	dave_bool fix_flag, error_flag;

	*log_buf = NULL;

	if((level == TRACELEVEL_DEBUG) || (level == TRACELEVEL_ASSERT))
	{
		error_flag = __log_trace__(&fix_flag, log_buf, &log_len, level, fmt, list_args);
	}
	else
	{
		error_flag = __log_buffer__(&fix_flag, log_buf, &log_len, level, fmt, list_args);
	}

	if(error_flag == dave_false)
	{
		return dave_false;
	}

	if(*log_buf != NULL)
	{
		if((level == TRACELEVEL_DEBUG) || (level == TRACELEVEL_ASSERT))
		{
			dave_os_trace(level, log_len, *log_buf);
		}
		else
		{
			log_fifo(_log_trace_enable, fix_flag, level, log_len, *log_buf);
		}
	}

	return dave_true;
}

// =====================================================================

void
__base_debug__(const char *args, ...)
{
	va_list list_args;
	s8 *log_buf;

	if(args == NULL)
		return;

	va_start(list_args, args);
	if(__log_log__(&log_buf, TRACELEVEL_DEBUG, args, list_args) == dave_false)
	{
		_log_buffer_dynamic();

		va_start(list_args, args);

		__log_log__(&log_buf, TRACELEVEL_DEBUG, args, list_args);
	}
	va_end(list_args);
}

void
__base_catcher__(const char *args, ...)
{
	va_list list_args;
	s8 *log_buf;

	if(args == NULL)
		return;

	va_start(list_args, args);
	if(__log_log__(&log_buf, TRACELEVEL_CATCHER, args, list_args) == dave_false)
	{
		_log_buffer_dynamic();

		va_start(list_args, args);

		__log_log__(&log_buf, TRACELEVEL_CATCHER, args, list_args);
	}
	va_end(list_args);
}

void
__base_trace__(const char *args, ...)
{
	va_list list_args;
	s8 *log_buf;

	if(args == NULL)
		return;

	va_start(list_args, args);
	if(__log_log__(&log_buf, TRACELEVEL_TRACE, args, list_args) == dave_false)
	{
		_log_buffer_dynamic();

		va_start(list_args, args);

		__log_log__(&log_buf, TRACELEVEL_TRACE, args, list_args);
	}
	va_end(list_args);
}

void
__base_log__(const char *args, ...)
{
	va_list list_args;
	s8 *log_buf;

	if(args == NULL)
		return;

	va_start(list_args, args);
	if(__log_log__(&log_buf, TRACELEVEL_LOG, args, list_args) == dave_false)
	{
		_log_buffer_dynamic();

		va_start(list_args, args);

		__log_log__(&log_buf, TRACELEVEL_LOG, args, list_args);
	}
	va_end(list_args);
}

void
__base_abnormal__(const char *args, ...)
{
	va_list list_args;
	s8 *log_buf;

	if(args == NULL)
		return;

	va_start(list_args, args);
	if(__log_log__(&log_buf, TRACELEVEL_ABNORMAL, args, list_args) == dave_false)
	{
		_log_buffer_dynamic();

		va_start(list_args, args);

		__log_log__(&log_buf, TRACELEVEL_ABNORMAL, args, list_args);
	}
	va_end(list_args);
}

void
__base_assert__(int assert_flag, const char *fun, int line, const char *args, ...)
{
	if(assert_flag == 0)
	{
		s8 args_str[1024];
		s8 poweroff_message[2048];
		va_list list_args;
		s8 *log_buf;

		dave_snprintf(args_str, sizeof(args_str), "%s\n", args);

		va_start(list_args, args);
		__log_log__(&log_buf, TRACELEVEL_ASSERT, args_str, list_args);
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

dave_bool
base_log_has_data(void)
{
	return log_buffer_has_data();
}

ub
base_log_history(s8 *history_ptr, ub history_len)
{
	return log_buffer_history(history_ptr, history_len);
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
	dave_bool current_flag = _log_trace_enable;

	_log_trace_enable = dave_true;

	if(write_cfg == dave_true)
	{
		cfg_set_bool(CFG_LOG_TRACE_ENABLE, _log_trace_enable);
	}

	if(current_flag == dave_false)
	{
		LOGLOG("log trace enable!");
	}
}

void
base_log_trace_disable(dave_bool write_cfg)
{
	if(_log_trace_enable == dave_true)
	{
		LOGLOG("log trace disable!");
	}

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

