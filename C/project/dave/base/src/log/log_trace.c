/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_os.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "log_log.h"

#define TRACE_ID_MAX (1)
#define TRACE_ID_LEN (256)
#define TRACE_LINE_MAX (8192)
#define TRACE_BASE_TIME_FUN dave_os_time_s()

typedef struct {
	s8 trace_id[TRACE_ID_LEN];
	ub trace_id_len;
	ub trace_action_time;
} TraceId;

typedef struct {
	s8 *set_fun;
	ub set_line;
	ub set_time;
	ub set_number;

	ub cur_time;
	ub cur_number;
} TraceLine;

static sb _trace_id_number = 0;
static TraceId _trace_id[TRACE_ID_MAX];
static sb _trace_line_number = 0;
static TraceLine _trace_line[TRACE_LINE_MAX];

static inline void
_trace_id_reset(TraceId *pId)
{
	dave_memset(pId, 0x00, sizeof(TraceId));

	pId->trace_id[0] = '\0';
	pId->trace_action_time = 0;
}

static inline void
_trace_line_reset(TraceLine *pLine)
{
	dave_memset(pLine, 0x00, sizeof(TraceLine));

	pLine->set_fun = NULL;
}

static inline void
_trace_reset_all(void)
{
	ub id_index, line_index;

	_trace_id_number = 0;

	for(id_index=0; id_index<TRACE_ID_MAX; id_index++)
	{
		_trace_id_reset(&_trace_id[id_index]);
	}

	_trace_line_number = 0;

	for(line_index=0; line_index<TRACE_LINE_MAX; line_index++)
	{
		_trace_line_reset(&_trace_line[line_index]);
	}
}

static inline dave_bool
_trace_id_add(s8 *trace_id)
{
	ub id_index;
	ub small_id_index, small_action_time;

	small_id_index = 0;
	small_action_time = _trace_id[small_id_index].trace_action_time;

	for(id_index=0; id_index<TRACE_ID_MAX; id_index++)
	{
		if(_trace_id[id_index].trace_action_time == 0)
			break;

		if(small_action_time > _trace_id[id_index].trace_action_time)
		{
			small_id_index = id_index;
			small_action_time = _trace_id[id_index].trace_action_time;
		}
	}

	if(id_index >= TRACE_ID_MAX)
	{
		id_index = small_id_index;

		LOGLOG("remove old trace id:%s", _trace_id[id_index].trace_id);
	}
	else
	{
		_trace_id_number ++;
	}

	dave_strcpy(_trace_id[id_index].trace_id, trace_id, TRACE_ID_LEN);
	_trace_id[id_index].trace_id_len = dave_strlen(_trace_id[id_index].trace_id);
	_trace_id[id_index].trace_action_time = dave_os_time_us();

	return dave_true;
}

static inline dave_bool
_trace_id_del(s8 *trace_id)
{
	ub id_index;

	if((trace_id == NULL) || (trace_id[0] == '\0'))
	{
		_trace_reset_all();
	}
	else
	{
		for(id_index=0; id_index<TRACE_ID_MAX; id_index++)
		{
			if(dave_strcmp(_trace_id[id_index].trace_id, trace_id) == dave_true)
			{
				_trace_id_reset(&_trace_id[id_index]);

				_trace_id_number --;
			}
		}
	}

	return dave_true;
}

static inline dave_bool
_trace_line_recording(s8 *fun, ub line, ub time, ub number)
{
	ub safe_counter, line_index;

	if((fun == NULL) || (line == 0) || (time == 0) || (number == 0))
	{
		return dave_false;
	}

	if(_trace_line_number >= TRACE_LINE_MAX)
	{
		return dave_false;
	}

	line_index = ((ub)fun) % TRACE_LINE_MAX;

	for(safe_counter=0; safe_counter<TRACE_LINE_MAX; safe_counter++)
	{
		if(line_index >= TRACE_LINE_MAX)
		{
			line_index = 0;
		}

		if(_trace_line[line_index].set_fun == NULL)
		{
			_trace_line[line_index].set_fun = fun;
			_trace_line[line_index].set_line = line;
			_trace_line[line_index].set_time = time;
			_trace_line[line_index].set_number = number;

			_trace_line[line_index].cur_time = TRACE_BASE_TIME_FUN;
			_trace_line[line_index].cur_number = 1;

			_trace_line_number ++;

			return dave_true;
		}

		line_index ++;
	}

	return dave_false;
}

static inline dave_bool
_trace_line_discriminator(TraceLine *pLine, ub cur_time)
{
	if((cur_time - pLine->cur_time) > pLine->set_time)
	{
		pLine->cur_time = cur_time;
		pLine->cur_number = 0;
	}

	if((pLine->cur_number ++) >= pLine->set_number)
	{
		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

static inline dave_bool
_trace_line_safe_discriminator(TraceLine *pLine, ub cur_time)
{
	dave_bool ret;

	t_lock_spin(NULL);
	ret = _trace_line_discriminator(pLine, cur_time);
	t_unlock_spin(NULL);

	return ret;
}

static inline dave_bool
_trace_line_safe_recording(s8 *fun, ub line, ub time, ub number)
{
	dave_bool ret;

	t_lock_spin(NULL);
	ret = _trace_line_recording(fun, line, time, number);
	t_unlock_spin(NULL);

	if(ret == dave_false)
	{
		LOGLOG("fun:%s line:%d recording failed!", fun, line);
	}

	return ret;
}

static inline dave_bool
_trace_id_enable(s8 *trace_id)
{
	ub id_index;

	if(trace_id == NULL)
	{
		return dave_true;
	}

	if(trace_id[0] == '\0')
	{
		return dave_false;
	}

	if(_trace_id_number <= 0)
	{
		return dave_false;
	}

	for(id_index=0; id_index<TRACE_ID_MAX; id_index++)
	{
		if((_trace_id[id_index].trace_action_time != 0)
			&& ((_trace_id[id_index].trace_id[0] == '*')
				|| (dave_memcmp(_trace_id[id_index].trace_id, trace_id, _trace_id[id_index].trace_id_len) == dave_true)))
		{
			return dave_true;
		}
	}

	return dave_false;
}

static inline sb
_trace_line_enable(s8 *fun, ub line)
{
	ub safe_counter, line_index;
	ub cur_time;

	if((fun == NULL) || (line == 0))
	{
		return -1;
	}

	if(_trace_line_number <= 0)
	{
		return -2;
	}

	line_index = ((ub)fun) % TRACE_LINE_MAX;

	cur_time = TRACE_BASE_TIME_FUN;

	for(safe_counter=0; safe_counter<TRACE_LINE_MAX; safe_counter++)
	{
		if(line_index >= TRACE_LINE_MAX)
		{
			line_index = 0;
		}

		if((_trace_line[line_index].set_fun == fun)
			&& (_trace_line[line_index].set_line == line))
		{
			if(_trace_line_safe_discriminator(&_trace_line[line_index], cur_time) == dave_true)
			{
				return 0;
			}
			else
			{
				return -1;
			}
		}

		line_index ++;
	}

	return -2;
}

// =====================================================================

void
log_trace_init(void)
{
	_trace_reset_all();
}

void
log_trace_exit(void)
{
	_trace_reset_all();
}

dave_bool
log_trace_add_id(s8 *trace_id)
{
	if(trace_id != NULL)
	{
		return _trace_id_add(trace_id);
	}
	else
	{
		return dave_false;
	}
}

dave_bool
log_trace_del_id(s8 *trace_id)
{
	if(trace_id != NULL)
	{
		return _trace_id_del(trace_id);
	}
	else
	{
		return dave_false;
	}
}

dave_bool
log_trace_id_enable(s8 *trace_id)
{
	if(trace_id != NULL)
	{
		return _trace_id_enable(trace_id);
	}
	else
	{
		return dave_false;
	}
}

dave_bool
log_trace_line_enable(s8 *fun, ub line, ub time, ub number)
{
	sb ret;

	ret = _trace_line_enable(fun, line);
	if(ret == 0)
	{
		return dave_true;
	}
	else if(ret == -1)
	{
		return dave_false;
	}
	else
	{
		_trace_line_safe_recording(fun, line, time, number);

		return dave_true;
	}
}

#endif

