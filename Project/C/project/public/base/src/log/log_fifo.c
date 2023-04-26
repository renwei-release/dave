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

typedef struct {
	TraceLevel level;
	ub data_len;
	u8 *data_ptr;

	void *next;
} LogFiFoChain;

static TLock _log_fifo_pv;
static LogFiFoChain *_log_fifo_chain_head = NULL;
static LogFiFoChain *_log_fifo_chain_tail = NULL;
static void *_log_fifo_thread_body = NULL;
static dave_bool _log_trace_enable = dave_false;

static inline LogFiFoChain *
_log_fifo_malloc_chain(TraceLevel level, ub data_len, u8 *data_ptr)
{
	LogFiFoChain *pChain = dave_malloc(sizeof(LogFiFoChain));

	pChain->level = level;
	pChain->data_len = data_len;
	pChain->data_ptr = dave_malloc(pChain->data_len + 1);
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
		if(pChain->data_ptr != NULL)
			dave_free(pChain->data_ptr);

		dave_free(pChain);
	}
}

static inline void
_log_fifo_output_data(void)
{
	ub safe_counter;
	LogFiFoChain *pChain;

	safe_counter = 0;

	while((safe_counter ++) < 102400)
	{
		pChain = NULL;
	
		SAFECODEv1(_log_fifo_pv, {

			if(_log_fifo_chain_head != NULL)
			{
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

		log_save_txt_file(log_info_product(), log_info_device(), pChain->level, (s8 *)(pChain->data_ptr), pChain->data_len);

		if((_log_trace_enable == dave_true)
			&& (pChain->level != TRACELEVEL_CATCHER))
		{
			dave_os_trace(pChain->level, pChain->data_len, pChain->data_ptr);
		}

		_log_fifo_free_chain(pChain);
	}
}

static inline void
_log_fifo_input_data(TraceLevel level, ub data_len, u8 *data_ptr)
{
	LogFiFoChain *pChain = _log_fifo_malloc_chain(level, data_len, data_ptr);

	SAFECODEv1(_log_fifo_pv, {

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
		_log_fifo_chain_head = _log_fifo_chain_tail = NULL;
	} );
}

// =====================================================================

void
log_fifo_init(void)
{
	_log_fifo_pre_init();

	log_save_init(30);
	
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
log_fifo(dave_bool trace_enable, TraceLevel level, ub data_len, u8 *data_ptr)
{
	_log_trace_enable = trace_enable;

	if((data_len == 0) || (data_ptr == NULL))
	{
		return;
	}

	_log_fifo_pre_init();

	_log_fifo_input_data(level, data_len, data_ptr);

	dave_os_thread_wakeup(_log_fifo_thread_body);
}

