/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "log_save.h"
#include "log_info.h"
#include "log_log.h"

#define LOG_FIFO_BYTES_MAX (16 * 1024 * 1024UL)

typedef struct {
	TraceLevel level;
	ub data_len;
	s8 *data_ptr;

	void *next;
} LogFiFoChain;

static TLock _log_fifo_pv;
static sb _log_fifo_counter = 0;
static ub _log_fifo_bytes = 0;
static ub _log_fifo_dropped = 0;
static ub _log_fifo_pending_dropped = 0;
static LogFiFoChain *_log_fifo_chain_head = NULL;
static LogFiFoChain *_log_fifo_chain_tail = NULL;
static void *_log_fifo_thread_body = NULL;
static dave_bool _log_trace_enable = dave_false;

static inline LogFiFoChain *
_log_fifo_malloc_chain(TraceLevel level, ub data_len, s8 *data_ptr)
{
	LogFiFoChain *pChain = dave_malloc(sizeof(LogFiFoChain));

	if(pChain == NULL)
		return NULL;
	pChain->level = level;
	pChain->data_len = data_len;
	// Dropped logs still advance the producer's ring; accepted data must be owned.
	pChain->data_ptr = dave_malloc(data_len + 1);
	if(pChain->data_ptr == NULL)
	{
		dave_free(pChain);
		return NULL;
	}
	dave_memcpy(pChain->data_ptr, data_ptr, data_len);
	pChain->data_ptr[data_len] = '\0';
	pChain->next = NULL;

	return pChain;
}

static inline void
_log_fifo_free_chain(LogFiFoChain *pChain)
{
	if(pChain != NULL)
	{
		dave_free(pChain->data_ptr);
		dave_free(pChain);
	}
}

static inline void
_log_fifo_drop(void)
{
	if(_log_fifo_dropped != ~(ub)0)
		_log_fifo_dropped ++;
	if(_log_fifo_pending_dropped != ~(ub)0)
		_log_fifo_pending_dropped ++;
}

static inline void
_log_fifo_report_dropped(void)
{
	ub dropped = 0;
	ub len;
	s8 message[192];
	dave_bool trace_enable = dave_false;

	SAFECODEv1(_log_fifo_pv, {
		if(_log_fifo_bytes == 0)
		{
			dropped = _log_fifo_pending_dropped;
			_log_fifo_pending_dropped = 0;
		}
	});
	if(dropped == 0)
		return;

	len = dave_snprintf(message, sizeof(message),
		"***** Log FIFO dropped %lu new logs (capacity %lu bytes). *****\n",
		dropped, (ub)LOG_FIFO_BYTES_MAX);
	// Bypass the FIFO so its own overflow report cannot recursively fill it.
	if(log_save_txt_file(log_info_product(), log_info_device(), TRACELEVEL_LOG, message, len) == dave_false)
	{
		SAFECODEv1(_log_fifo_pv, {
			if(dropped > (~(ub)0 - _log_fifo_pending_dropped))
				_log_fifo_pending_dropped = ~(ub)0;
			else
				_log_fifo_pending_dropped += dropped;
		});
	}
	SAFECODEv1(_log_fifo_pv, { trace_enable = _log_trace_enable; });
	if(trace_enable == dave_true)
		dave_os_trace(TRACELEVEL_LOG, len, message);
}

static inline void
_log_fifo_output_data(void)
{
	ub safe_counter;
	ub chain_bytes;
	LogFiFoChain *pChain;
	dave_bool trace_enable;
	dave_bool more = dave_false;

	safe_counter = 0;

	while((safe_counter ++) < 102400)
	{
		pChain = NULL;
		trace_enable = dave_false;
	
		SAFECODEv1(_log_fifo_pv, {

			if(_log_fifo_chain_head != NULL)
			{
				if(_log_fifo_counter > 0)
				{
					_log_fifo_counter --;
				}
				else
				{
					LOGLOG("invalid fifo counter:%d", _log_fifo_counter);
					_log_fifo_counter = 0;
				}

				pChain = _log_fifo_chain_head;

				_log_fifo_chain_head = _log_fifo_chain_head->next;

				if(_log_fifo_chain_head == NULL)
				{
					_log_fifo_chain_tail = NULL;
				}
			}

		});

		if(pChain == NULL)
		{
			break;
		}

		log_save_txt_file(log_info_product(), log_info_device(), pChain->level, pChain->data_ptr, pChain->data_len);

		SAFECODEv1(_log_fifo_pv, { trace_enable = _log_trace_enable; });
		if((trace_enable == dave_true)
			&& (pChain->level != TRACELEVEL_CATCHER))
		{		
			dave_os_trace(pChain->level, pChain->data_len, pChain->data_ptr);
		}

		chain_bytes = sizeof(*pChain) + pChain->data_len + 1;
		_log_fifo_free_chain(pChain);
		SAFECODEv1(_log_fifo_pv, { _log_fifo_bytes -= chain_bytes; });
	}
	SAFECODEv1(_log_fifo_pv, { more = (_log_fifo_chain_head != NULL); });
	if(more == dave_true)
		dave_os_thread_wakeup(_log_fifo_thread_body);
	else
		_log_fifo_report_dropped();
}

static inline void
_log_fifo_input_data(dave_bool trace_enable, TraceLevel level, ub data_len, s8 *data_ptr)
{
	LogFiFoChain *pChain;
	ub chain_bytes = 0;
	dave_bool reserved = dave_false;

	SAFECODEv1(_log_fifo_pv, {
		_log_trace_enable = trace_enable;
		if((data_ptr != NULL) && (data_len > 0))
		{
			if(data_len <= (LOG_FIFO_BYTES_MAX - sizeof(LogFiFoChain) - 1))
			{
				chain_bytes = sizeof(LogFiFoChain) + data_len + 1;
				if(chain_bytes <= (LOG_FIFO_BYTES_MAX - _log_fifo_bytes))
				{
					_log_fifo_bytes += chain_bytes;
					reserved = dave_true;
				}
			}
			if(reserved == dave_false)
				_log_fifo_drop();
		}
	});
	if(reserved == dave_false)
		return;

	// Reserve before allocating; concurrent producers cannot exceed the budget.
	pChain = _log_fifo_malloc_chain(level, data_len, data_ptr);
	if(pChain == NULL)
	{
		SAFECODEv1(_log_fifo_pv, {
			_log_fifo_bytes -= chain_bytes;
			_log_fifo_drop();
		});
		LOGLOG("allocate log FIFO entry failed!");
		return;
	}
	SAFECODEv1(_log_fifo_pv, {
		_log_fifo_counter ++;

		if(_log_fifo_chain_head == NULL)
		{
			_log_fifo_chain_head = _log_fifo_chain_tail = pChain;
		}
		else
		{
			_log_fifo_chain_tail->next = pChain;

			_log_fifo_chain_tail = pChain;
		}

	});
}

static void *
_log_fifo_thread(void *arg)
{
	while(dave_os_thread_canceled(_log_fifo_thread_body) == dave_false)
	{
		dave_os_thread_sleep(_log_fifo_thread_body);
	
		_log_fifo_output_data();
	}

	_log_fifo_output_data();

	return NULL;
}

static inline void
_log_fifo_pre_init(void)
{
	static volatile sb __safe_pre_flag__ = 0;

	SAFEPre(__safe_pre_flag__, {
		t_lock_reset(&_log_fifo_pv);
		_log_fifo_counter = 0;
		_log_fifo_bytes = 0;
		_log_fifo_dropped = _log_fifo_pending_dropped = 0;
		_log_fifo_chain_head = _log_fifo_chain_tail = NULL;
	} );
}

// =====================================================================

void
log_fifo_init(void)
{
	_log_fifo_pre_init();

	log_save_init();

	_log_fifo_thread_body = dave_os_create_thread("log-fifo", _log_fifo_thread, NULL);
	if(_log_fifo_thread_body == NULL)
	{
		LOGLOG("i can not start log fifo thread!");
	}
}

void
log_fifo_exit(void)
{
	if(_log_fifo_thread_body != NULL)
	{
		dave_os_release_thread(_log_fifo_thread_body);
	}

	log_save_exit();
}

void
log_fifo(dave_bool trace_enable, dave_bool fix_flag, TraceLevel level, ub data_len, s8 *data_ptr)
{
	(void)fix_flag;
	_log_fifo_pre_init();
	_log_fifo_input_data(trace_enable, level, data_len, data_ptr);

	if((data_len == 0) || (data_ptr == NULL))
	{
		return;
	}

	dave_os_thread_wakeup(_log_fifo_thread_body);
}

ub
log_fifo_info(s8 *info_ptr, ub info_len)
{
	ub used, bytes = 0, dropped = 0, pending = 0;
	sb queued = 0;
	dave_bool snapshot = dave_false;

	_log_fifo_pre_init();
	SAFECODEv1(_log_fifo_pv, {
		bytes = _log_fifo_bytes;
		queued = _log_fifo_counter;
		dropped = _log_fifo_dropped;
		pending = _log_fifo_pending_dropped;
		snapshot = dave_true;
	});
	if(snapshot == dave_false)
	{
		LOGLOG("log FIFO statistics lock failed!");
		return 0;
	}
	used = log_save_info(info_ptr, info_len);
	if((info_ptr != NULL) && (used < info_len))
	{
		used += dave_snprintf(&info_ptr[used], info_len-used,
			"\nFIFO bytes:%lu/%lu queued:%ld dropped:%lu pending:%lu\n",
			bytes, (ub)LOG_FIFO_BYTES_MAX, queued, dropped, pending);
	}
	return used;
}

