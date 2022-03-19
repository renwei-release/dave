/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "dave_tools.h"
#include "log_lock.h"
#include "log_log.h"

#define LOG_BUFFER_NUMBER (1024)
#define LOG_BUFFER_LENGTH (16392)

typedef struct {
	TraceLevel level;
	s8 buffer[LOG_BUFFER_LENGTH];
	ub buffer_length;
} LogBuffer;

static LogBuffer _log_buffer[LOG_BUFFER_NUMBER];
static volatile ub _log_buffer_w_index = 0;
static volatile ub _log_buffer_r_index = 0;
static ub _log_lost_counter = 0;

static void
_log_buffer_reset(LogBuffer *pBuffer)
{
	dave_memset(pBuffer, 0x00, sizeof(LogBuffer));

	pBuffer->level = TRACELEVEL_MAX;
	pBuffer->buffer_length = 0;
}

static void
_log_buffer_reset_all(void)
{
	ub log_index;

	for(log_index=0; log_index<LOG_BUFFER_NUMBER; log_index++)
	{
		_log_buffer_reset(&_log_buffer[log_index]);
	}
}

static void
_log_buffer_inside_lost_message(ub counter)
{
	LogBuffer *pBuffer = NULL;

	log_lock();
	if((_log_buffer_w_index - _log_buffer_r_index) == LOG_BUFFER_NUMBER)
	{
		pBuffer = &_log_buffer[(_log_buffer_w_index - 1) % LOG_BUFFER_NUMBER];
	}
	else if((_log_buffer_w_index - _log_buffer_r_index) < LOG_BUFFER_NUMBER)
	{
		pBuffer = &_log_buffer[(_log_buffer_w_index ++) % LOG_BUFFER_NUMBER];
	}
	log_unlock();

	if(pBuffer != NULL)
	{
		pBuffer->level = TRACELEVEL_LOG;
		pBuffer->buffer_length = dave_sprintf(pBuffer->buffer,
			"***** Please note that there is not enough log space and %d logs are lost! *****",
			counter);
	}
}

// =====================================================================

void
log_buffer_init(void)
{
	_log_buffer_reset_all();

	_log_buffer_w_index = _log_buffer_r_index = 0;

	_log_lost_counter = 0;
}

void
log_buffer_exit(void)
{

}

void
log_buffer_put(TraceLevel level, s8 *log, ub log_len, dave_bool end_flag)
{
	dave_bool overflow_flag = dave_false;
	LogBuffer *pBuffer;

	log_lock();
	if((_log_buffer_w_index - _log_buffer_r_index) >= (LOG_BUFFER_NUMBER - 1))
	{
		overflow_flag = dave_true;
		_log_lost_counter ++;
	}
	log_unlock();

	if(overflow_flag == dave_true)
	{
		_log_buffer_inside_lost_message(_log_lost_counter);
		return;
	}

	_log_lost_counter = 0;

	log_lock();
	pBuffer = &_log_buffer[_log_buffer_w_index % LOG_BUFFER_NUMBER];

	if((pBuffer->buffer_length + log_len) >= (LOG_BUFFER_LENGTH - 1))
	{
		log_len = (LOG_BUFFER_LENGTH - 1) - pBuffer->buffer_length;
	}

	if((pBuffer->buffer_length + log_len) < (LOG_BUFFER_LENGTH - 1))
	{
		if(pBuffer->level == TRACELEVEL_MAX)
		{
			pBuffer->level = level;
		}

		dave_memcpy(&pBuffer->buffer[pBuffer->buffer_length], log, log_len);

		pBuffer->buffer_length += log_len;

		pBuffer->buffer[pBuffer->buffer_length] = '\0';
	}

	if(end_flag == dave_true)
	{
		_log_buffer_w_index ++;

		pBuffer = &_log_buffer[_log_buffer_w_index % LOG_BUFFER_NUMBER];
		pBuffer->level = TRACELEVEL_MAX;
		pBuffer->buffer_length = 0;
	}
	log_unlock();
}

ub
log_buffer_get(s8 *log_buf, ub log_buf_len, TraceLevel *level)
{
	LogBuffer *pBuffer;
	ub log_copy_len;

	pBuffer = NULL;

	log_lock();
	if(_log_buffer_w_index > _log_buffer_r_index)
	{
		pBuffer = &_log_buffer[(_log_buffer_r_index ++) % LOG_BUFFER_NUMBER];
	}
	log_unlock();

	if(pBuffer != NULL)
	{
		if(log_buf_len > pBuffer->buffer_length)
			log_copy_len = pBuffer->buffer_length;
		else
			log_copy_len = log_buf_len;
	
		dave_memcpy(log_buf, pBuffer->buffer, log_copy_len);

		*level = pBuffer->level;
	}
	else
	{
		log_copy_len = 0;
		*level = TRACELEVEL_MAX;
	}

	LOGDEBUG("<%d/%d> log_copy_len:%d level:%s",
		_log_buffer_w_index, _log_buffer_r_index,
		log_copy_len, trace_level_to_str(*level));

	if(log_copy_len > log_buf_len)
	{
		log_buf[log_copy_len] = '\0';
	}

	return log_copy_len;
}

ub
log_buffer_history(s8 *log_buf, ub log_buf_len)
{
	ub history_start_index, history_end_index, safe_counter, buffer_len;
	LogBuffer *pBuffer;

	history_start_index = history_end_index = _log_buffer_w_index - 1;

	safe_counter = 0;

	buffer_len = 0;

	while((safe_counter ++) < LOG_BUFFER_NUMBER)
	{
		pBuffer = &_log_buffer[history_start_index % LOG_BUFFER_NUMBER];
		if((pBuffer->level >= TRACELEVEL_MAX) || (pBuffer->buffer_length == 0))
		{
			break;
		}

		buffer_len += pBuffer->buffer_length;
		if(buffer_len > log_buf_len)
		{
			history_start_index += 1;

			break;
		}
		else
		{
			if(history_start_index == 0)
				break;
			history_start_index --;
		}
	}

	LOGDEBUG("<%d/%d> start:%ld end:%ld",
		_log_buffer_w_index, _log_buffer_r_index,
		history_start_index, history_end_index);

	safe_counter = 0;

	buffer_len = 0;

	while(((safe_counter ++) < LOG_BUFFER_NUMBER) && (history_start_index <= history_end_index))
	{
		pBuffer = &_log_buffer[(history_start_index ++) % LOG_BUFFER_NUMBER];

		if((pBuffer->level < TRACELEVEL_MAX) && (pBuffer->buffer_length != 0) && ((buffer_len + pBuffer->buffer_length) < log_buf_len))
		{
			dave_memcpy(&log_buf[buffer_len], pBuffer->buffer, pBuffer->buffer_length);

			buffer_len += pBuffer->buffer_length;
		}
	}

	if(buffer_len < log_buf_len)
	{
		log_buf[buffer_len] = '\0';
	}

	LOGDEBUG("<%d/%d> buffer_len:%d", _log_buffer_w_index, _log_buffer_r_index, buffer_len);

	return buffer_len;
}

