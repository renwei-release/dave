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
#include <stdio.h>
#include <stdlib.h>

#define MAXBUF		32
#define MAXCHARS	16392
#define ZEROPAD		1		/* pad with zero */
#define SIGN		2		/* unsigned/signed long */
#define PLUS		4		/* show plus */
#define SPACE		8		/* space if plus */
#define LEFT		16		/* left justified */
#define SMALL		32		/* Must be 32 == 0x20 */
#define SPECIAL		64		/* 0x */

static s8 _trace_buf[MAXBUF][MAXCHARS + 16];
static volatile ub _trace_index = 0;
static volatile ub _trace_logic_number_counter = 0;
static volatile dave_bool _inside_time_flag = dave_true;

static ub
_log_inside_time_format(s8 *log, ub log_len)
{
	dave_bool time_flag;
	DateStruct date;
	ub time_len = 0;
	ub counter = 0;

	log_lock();
	time_flag = _inside_time_flag;
	if(time_flag == dave_true)
	{
		counter = _trace_logic_number_counter ++;
		_inside_time_flag = dave_false;
	}
	log_unlock();

	if(time_flag == dave_true)
	{
		time_len += dave_snprintf(&log[time_len], log_len-time_len,
			"(%s.%s.%s)",
			VERSION_MAIN, VERSION_SUB, VERSION_REV);

		t_time_get_date(&date);

		time_len += dave_snprintf(&log[time_len], log_len-time_len,
			"<%04d.%02d.%02d %02d:%02d:%02d:%x>",
			date.year, date.month, date.day,
			date.hour, date.minute, date.second,
			counter);
    }

	return time_len;
}

static void
_log_inside_time_flag(s8 *log, ub log_len, ub log_total_len)
{
	if((log_len >= 1) && (log_len <= log_total_len))
	{
		if((log[log_len - 1] == '\n') || (log[log_len - 1] == '\r'))
		{
			log_lock();
			_inside_time_flag = dave_true;
			log_unlock();
		}
	}
}

static void
_log_log(TraceLevel level, const char *fun, int line, const char *fmt, va_list list_args)
{
	ub len = 0;
	s8 *buf;

	log_lock();
	buf = _trace_buf[(_trace_index ++) % MAXBUF];
	log_unlock();

	len += _log_inside_time_format(&buf[len], MAXCHARS-len);

	if(fmt != NULL)
	{
		len += (ub)vsnprintf((char *)&buf[len], MAXCHARS-len, fmt, list_args);
	}

	_log_inside_time_flag(buf, len, MAXCHARS);

	if(level != TRACELEVEL_CATCHER)
	{
		dave_os_trace(level, len, (u8 *)buf);
	}

	if(level != TRACELEVEL_DEBUG)
	{
		log_buffer_put(level, buf, len, _inside_time_flag);
	}

	if(level == TRACELEVEL_ASSERT)
	{
		s8 poweroff_message[2048];

		dave_snprintf(poweroff_message, sizeof(poweroff_message), "%s:%d:%s", fun, line, buf);
		base_power_off((char *)poweroff_message);
	}
}

// =====================================================================

void
__base_debug__(const char *args, ...)
{
	va_list list_args;

	va_start(list_args, args);
	_log_log(TRACELEVEL_DEBUG, __func__, __LINE__, args, list_args);
	va_end(list_args);
}

void
__base_catcher__(const char *args, ...)
{
	va_list list_args;

	va_start(list_args, args);
	_log_log(TRACELEVEL_CATCHER, __func__, __LINE__, args, list_args);
	va_end(list_args);
}

void
__base_trace__(const char *args, ...)
{
	va_list list_args;

	va_start(list_args, args);
	_log_log(TRACELEVEL_TRACE, __func__, __LINE__, args, list_args);
	va_end(list_args);
}

void
__base_log__(const char *args, ...)
{
	va_list list_args;

	va_start(list_args, args);
	_log_log(TRACELEVEL_LOG, __func__, __LINE__, args, list_args);
	va_end(list_args);
}

void
__base_abnormal__(const char *args, ...)
{
	va_list list_args;

	va_start(list_args, args);
	_log_log(TRACELEVEL_ABNORMAL, __func__, __LINE__, args, list_args);
	va_end(list_args);
}

void
__base_assert__(int assert_flag, const char *fun, int line, const char *args, ...)
{
	va_list list_args;

	if(assert_flag == 0)
	{
		va_start(list_args, args);
		_log_log(TRACELEVEL_ASSERT, fun, line, args, list_args);
		va_end(list_args);
	}
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

ub
base_log_history(s8 *log_buf, ub log_buf_len)
{
	if((log_buf == NULL) || (log_buf_len == 0))
	{
		return 0;
	}
	return log_buffer_history(log_buf, log_buf_len);
}

void
base_log_init(void)
{
	log_trace_init();
	dave_memset(_trace_buf, 0x00, sizeof(_trace_buf));
	_trace_index = 0;
	_trace_logic_number_counter = 0;
	log_buffer_init();
	_inside_time_flag = dave_true;
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

