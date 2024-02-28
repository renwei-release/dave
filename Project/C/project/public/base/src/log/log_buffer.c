/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_3rdparty.h"
#include "log_buffer.h"
#include "log_lock.h"
#include "log_log.h"

#if defined(PERFTOOLS_3RDPARTY)
#define LOG_MALLOC dave_perftools_malloc
#define LOG_FREE dave_perftools_free
#elif defined(JEMALLOC_3RDPARTY)
#define LOG_MALLOC dave_jemalloc
#define LOG_FREE dave_jefree
#else
#define LOG_MALLOC malloc
#define LOG_FREE free
#endif

#define LOG_TID_MAX DAVE_SYS_THREAD_ID_MAX
#define LOG_LIST_MAX (LOG_BUFFER_MAX - 8)
#define INVALID_TID 0xffffffffffffffff

static volatile dave_bool __system_startup__ = dave_false;
static LogBuffer *_log_thread[LOG_TID_MAX];

static LogBuffer _log_buffer_ptr[LOG_BUFFER_MAX];
static ub _log_buffer_index = 0;

static LogBuffer *_log_list[LOG_LIST_MAX];
static ub _log_list_w_index = 0;
static ub _log_list_r_index = 0;

static ub _log_lost_counter = 0;
static LogBuffer _log_lost_buffer;

static inline void
_log_buffer_free(LogBuffer *pBuffer)
{
	pBuffer->fix_buffer_history_len = pBuffer->fix_buffer_index;
	pBuffer->fix_buffer_index = 0;

	if(pBuffer->dynamic_buffer_ptr != NULL)
	{
		LOG_FREE(pBuffer->dynamic_buffer_ptr);
		pBuffer->dynamic_buffer_ptr = NULL;
	}
	pBuffer->dynamic_buffer_len = 0;
	pBuffer->dynamic_buffer_index = 0;

	pBuffer->tid = INVALID_TID;

	pBuffer->level = TRACELEVEL_MAX;
}

static inline LogBuffer *
_log_buffer_malloc(void)
{
	LogBuffer *pBuffer;
	dave_bool overflow = dave_false;

	log_lock();
	pBuffer = &_log_buffer_ptr[_log_buffer_index++ % LOG_BUFFER_MAX];
	if(pBuffer->level != TRACELEVEL_MAX)
	{
		overflow = dave_true;
	}
	pBuffer->level = TRACELEVEL_MAX;
	pBuffer->fix_buffer_index = 0;
	pBuffer->fix_buffer_history_len = 0;

	pBuffer->dynamic_buffer_index = 0;
	log_unlock();

	if(overflow == dave_true)
	{
		LOGLTRACE(60,1,"The log is generated too fast, please define a larger cache(%d)! lost:%d",
			LOG_BUFFER_MAX, _log_lost_counter);
	}

	return pBuffer;
}

static inline void
_log_buffer_transfer(LogBuffer *pBuffer, ub buffer_len)
{
	if((pBuffer->dynamic_buffer_ptr == NULL) && (buffer_len >= LOG_FIX_BUFFER_LEN))
	{
		buffer_len += 1024;

		pBuffer->dynamic_buffer_ptr = LOG_MALLOC(buffer_len);
		pBuffer->dynamic_buffer_len = buffer_len;
		pBuffer->dynamic_buffer_index = dave_memcpy(pBuffer->dynamic_buffer_ptr, pBuffer->fix_buffer_ptr, pBuffer->fix_buffer_index);
	}
}

static inline void
_log_buffer_reset(LogBuffer *pBuffer)
{
	dave_memset(pBuffer, 0x00, sizeof(LogBuffer));

	pBuffer->dynamic_buffer_ptr = NULL;

	_log_buffer_free(pBuffer);
}

static inline void
_log_buffer_reset_all(void)
{
	ub thread_index, buffer_index, list_index;

	for(thread_index=0; thread_index<LOG_TID_MAX; thread_index++)
	{
		_log_thread[thread_index] = NULL;
	}

	for(buffer_index=0; buffer_index<LOG_BUFFER_MAX; buffer_index++)
	{
		_log_buffer_reset(&_log_buffer_ptr[buffer_index]);
	}
	_log_buffer_index = 0;

	for(list_index=0; list_index<LOG_LIST_MAX; list_index++)
	{
		_log_list[list_index] = NULL;
	}
	_log_list_w_index = _log_list_r_index = 0;

	_log_lost_counter = 0;
	_log_buffer_reset(&_log_lost_buffer);
}

static inline dave_bool
_log_buffer_list_set(LogBuffer *pBuffer)
{
	dave_bool ret = dave_true;

	log_lock();
	if((_log_list_w_index - _log_list_r_index) < LOG_LIST_MAX)
	{
		_log_list[(_log_list_w_index ++) % LOG_LIST_MAX] = pBuffer;
	}
	else
	{
		_log_buffer_free(pBuffer);

		ret = dave_false;
	}
	log_unlock();

	return ret;
}

static inline LogBuffer *
_log_buffer_list_get(void)
{
	LogBuffer *pBuffer;

	log_lock();
	if(_log_list_w_index > _log_list_r_index)
	{
		pBuffer = _log_list[(_log_list_r_index ++) % LOG_LIST_MAX];
	}
	else
	{
		pBuffer = NULL;
		_log_list_w_index = _log_list_r_index = 0;
	}
	log_unlock();

	return pBuffer;
}

static inline LogBuffer *
_log_buffer_thread_build(void)
{
	ub tid, tid_index;
	LogBuffer *pBuffer;

	tid = dave_os_thread_id();
	tid_index = tid % LOG_TID_MAX;

	if(_log_thread[tid_index] == NULL)
	{
		pBuffer = _log_buffer_malloc();

		if(pBuffer != NULL)
		{
			pBuffer->tid = tid;

			_log_thread[tid_index] = pBuffer;
		}
	}

	pBuffer = _log_thread[tid_index];

	if(pBuffer != NULL)
	{
		if(tid != pBuffer->tid)
		{
			LOGABNOR("overflow tid zone! tid:%ld/%ld",
				tid, pBuffer->tid);
			pBuffer = NULL;
		}
	}

	return pBuffer;
}

static inline void
_log_buffer_thread_clean(ub tid)
{
	ub tid_index = tid % LOG_TID_MAX;

	_log_thread[tid_index] = NULL;
}

static inline void
_log_buffer_lost_msg(void)
{
	log_lock();
	_log_lost_buffer.level = TRACELEVEL_LOG;
	_log_lost_buffer.fix_buffer_index += dave_snprintf(
			&_log_lost_buffer.fix_buffer_ptr[_log_lost_buffer.fix_buffer_index],
			sizeof(_log_lost_buffer.fix_buffer_ptr) - _log_lost_buffer.fix_buffer_index,
			"***** Please note that there is not enough log space and %d logs are lost! *****\n",
			_log_lost_counter);

	_log_lost_counter = 0;
	log_unlock();
}

static inline void
_log_buffer_set(LogBuffer *pBuffer)
{
	if(pBuffer == NULL)
	{
		log_lock();
		_log_lost_counter ++;
		log_unlock();
		return;
	}

	_log_buffer_thread_clean(pBuffer->tid);

	pBuffer->fix_buffer_history_len = pBuffer->fix_buffer_index;

	if(_log_buffer_list_set(pBuffer) == dave_false)
	{
		log_lock();
		_log_lost_counter ++;
		log_unlock();
		return;
	}

	if(_log_lost_counter > 0)
	{
		_log_buffer_lost_msg();
	}
}

static inline ub
_log_buffer_get(s8 *log_ptr, ub log_len, TraceLevel *level)
{
	LogBuffer *pBuffer;
	ub log_copy_len;

	pBuffer = _log_buffer_list_get();

	if(pBuffer != NULL)
	{
		if(pBuffer->dynamic_buffer_ptr == NULL)
		{
			if(log_len > pBuffer->fix_buffer_index)
				log_copy_len = pBuffer->fix_buffer_index;
			else
				log_copy_len = log_len;

			dave_memcpy(log_ptr, pBuffer->fix_buffer_ptr, log_copy_len);
		}
		else
		{
			if(log_len > pBuffer->dynamic_buffer_index)
				log_copy_len = pBuffer->dynamic_buffer_index;
			else
				log_copy_len = log_len;
		
			dave_memcpy(log_ptr, pBuffer->dynamic_buffer_ptr, log_copy_len);
		}
		*level = pBuffer->level;

		_log_buffer_free(pBuffer);
	}
	else if(_log_lost_buffer.level != TRACELEVEL_MAX)
	{
		log_lock();
		if(log_len > _log_lost_buffer.fix_buffer_index)
			log_copy_len = _log_lost_buffer.fix_buffer_index;
		else
			log_copy_len = log_len;

		dave_memcpy(log_ptr, _log_lost_buffer.fix_buffer_ptr, log_copy_len);
		*level = _log_lost_buffer.level;

		_log_lost_buffer.level = TRACELEVEL_MAX;
		_log_lost_buffer.fix_buffer_index = 0;
		log_unlock();
	}
	else
	{
		log_copy_len = 0;
		*level = TRACELEVEL_MAX;
	}

	if(log_len > log_copy_len)
	{
		log_ptr[log_copy_len] = '\0';
	}

	return log_copy_len;
}

static inline ub
_log_buffer_history_start_index(ub history_len)
{
	ub buffer_index;
	ub history_index, buffer_loop;
	LogBuffer *pBuffer;

	buffer_index = _log_buffer_index;
	history_index = 0;

	for(buffer_loop=0; buffer_loop<LOG_BUFFER_MAX; buffer_loop++)
	{
		pBuffer = &_log_buffer_ptr[(-- buffer_index) % LOG_BUFFER_MAX];
		if((history_index + pBuffer->fix_buffer_history_len) > history_len)
		{
			buffer_index ++;
			break;
		}
		history_index += pBuffer->fix_buffer_history_len;
	}

	return buffer_index;
}

// =====================================================================

void
log_buffer_init(void)
{
	__system_startup__ = dave_false;
	_log_buffer_reset_all();
	__system_startup__ = dave_true;
}

void
log_buffer_exit(void)
{

}

LogBuffer *
log_buffer_thread(void)
{
	if(__system_startup__ == dave_false)
		return NULL;

	return _log_buffer_thread_build();
}

LogBuffer *
log_buffer_transfer(ub buffer_len)
{
	LogBuffer *pBuffer = log_buffer_thread();

	if(pBuffer != NULL)
	{
		_log_buffer_transfer(pBuffer, buffer_len);
	}

	return pBuffer;
}

void
log_buffer_set(LogBuffer *pBuffer)
{
	if(__system_startup__ == dave_false)
		return;

	_log_buffer_set(pBuffer);
}

ub
log_buffer_get(s8 *log_ptr, ub log_len, TraceLevel *level)
{
	if(__system_startup__ == dave_false)
		return 0;

	return _log_buffer_get(log_ptr, log_len, level);
}

dave_bool
log_buffer_has_data(void)
{
	if(__system_startup__ == dave_false)
		return dave_false;

	if((_log_list_w_index > _log_list_r_index)
		|| (_log_lost_buffer.fix_buffer_index > 0))
	{
		return dave_true;
	}

	return dave_false;
}

ub
log_buffer_history(s8 *history_ptr, ub history_len)
{
	ub buffer_index;
	ub history_index, buffer_loop;
	LogBuffer *pBuffer;

	if((history_ptr == NULL) || (history_len < 1))
	{
		return 0;
	}
	// reserved space for '\0'
	history_len -= 1;

	buffer_index = _log_buffer_history_start_index(history_len);
	history_index = 0;

	for(buffer_loop=0; buffer_loop<LOG_BUFFER_MAX; buffer_loop++)
	{
		pBuffer = &_log_buffer_ptr[(buffer_index ++) % LOG_BUFFER_MAX];

		if((history_index + pBuffer->fix_buffer_history_len) > history_len)
			break;
		
		history_index += dave_memcpy(&history_ptr[history_index], pBuffer->fix_buffer_ptr, pBuffer->fix_buffer_history_len);
	}

	if(history_index == 0)
	{
		history_index += dave_snprintf(&history_ptr[history_index], history_len-history_index,
			"log history is empty! buffer_index:%ld w_index:%ld r_index:%ld",
			_log_buffer_index, _log_list_w_index, _log_list_r_index);
	}
	history_ptr[history_index] = '\0';

	return history_index;
}

