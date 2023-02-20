/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_parameter.h"
#if defined(__DAVE_BASE__) && defined(ENABLE_THREAD_COROUTINE)
#include "dave_base.h"
#include "dave_os.h"
#include "thread_struct.h"
#include "thread_mem.h"
#include "thread_thread.h"
#include "thread_tools.h"
#include "thread_coroutine.h"
#include "coroutine_core.h"
#include "coroutine_arch.h"
#include "thread_log.h"

#define COROUTINE_CORE_STACK_DEFAULT_SIZE 128 * 1024
#define TID_MAX DAVE_SYS_THREAD_ID_MAX
#define CFG_COROUTINE_STACK_SIZE "CoroutineStackSize"

typedef void* (* co_swap_callback_fun)(void *param);

typedef struct {
	coroutine_core_fun fun_ptr;
	void *fun_param;

	CoSwap swap;

	CoThreadEnv *env;
} CoCore;

static CoThreadEnv *_co_thread_env[ TID_MAX ] = { 0 };
static ub _coroutine_stack_size = 0;

static void *
_coroutine_swap_function(void *param)
{
	CoCore *pCore = (CoCore *)param;

	if(pCore->fun_ptr != NULL)
	{
		pCore->fun_ptr(pCore->fun_param);
	}

	if(coroutine_yield(pCore) == dave_false)
	{
		THREADABNOR("yield failed!");
	}

	return NULL;
}

static inline void
_coroutine_swap_clean(CoSwap *pSwap)
{
	dave_memset(pSwap, 0x00, sizeof(CoSwap));
}

static inline CoThreadEnv *
_coroutine_env_malloc(void)
{
	return dave_ralloc(sizeof(CoThreadEnv));
}

static inline void
_coroutine_env_free(CoThreadEnv *pEnv)
{
	if(pEnv != NULL)
	{
		dave_free(pEnv);
	}
}

static inline ub
_coroutine_tid(void)
{
	return dave_os_thread_id() % TID_MAX;
}

static inline CoThreadEnv *
_coroutine_thread_env(void)
{
	ub tid_index;

	tid_index = _coroutine_tid();

	if(_co_thread_env[tid_index] == NULL)
	{
		_co_thread_env[tid_index] = _coroutine_env_malloc();

		coroutine_swap_make(&(_co_thread_env[tid_index]->base_swap), NULL, NULL);

		_co_thread_env[tid_index]->co_swap = NULL;
	}

	return _co_thread_env[tid_index];
}

static void
_coroutine_core_init(void)
{
	ub tid_index;

	for(tid_index=0; tid_index<TID_MAX; tid_index++)
	{
		_co_thread_env[tid_index] = NULL;
	}
}

static void
_coroutine_core_exit(void)
{
	ub tid_index;

	for(tid_index=0; tid_index<TID_MAX; tid_index++)
	{
		if(_co_thread_env[tid_index] != NULL)
		{
			_coroutine_env_free(_co_thread_env[tid_index]);
			_co_thread_env[tid_index] = NULL;
		}
	}
}

static inline void *
_coroutine_create(coroutine_core_fun fun_ptr, void *fun_param)
{
	CoCore *pCore = dave_malloc(sizeof(CoCore));

	pCore->fun_ptr = fun_ptr;
	pCore->fun_param = fun_param;

	pCore->swap.ss_size = _coroutine_stack_size;
	pCore->swap.ss_sp = dave_malloc(pCore->swap.ss_size);

	coroutine_swap_make(&(pCore->swap), _coroutine_swap_function, pCore);

	pCore->env = _coroutine_thread_env();

	return pCore;
}

static inline dave_bool
_coroutine_resume(void *co)
{
	CoCore *pCore = (CoCore *)co;
	CoThreadEnv *pEnv;

	if(pCore == NULL)
	{
		THREADABNOR("Arithmetic error! pCore is NULL");
		return dave_false;
	}

	pEnv = pCore->env;
	if(pEnv == NULL)
	{
		THREADABNOR("Arithmetic error! pEnv is NULL");
		return dave_false;
	}

	if(pEnv->co_swap != NULL)
	{
		THREADABNOR("Arithmetic error! co_swap:%x", pEnv->co_swap);
		return dave_false;
	}

	pEnv->co_swap = &(pCore->swap);

	coroutine_swap_run(&(pEnv->base_swap), pEnv->co_swap);

	return dave_true;
}

static inline dave_bool
_coroutine_yield(void *co)
{
	CoCore *pCore = (CoCore *)co;
	CoThreadEnv *pEnv;

	if(pCore == NULL)
	{
		THREADABNOR("Arithmetic error! pCore is NULL");
		return dave_false;
	}	

	pEnv = pCore->env;
	if(pEnv == NULL)
	{
		THREADABNOR("Arithmetic error! pEnv is NULL");
		return dave_false;
	}

	if(pEnv->co_swap == NULL)
	{
		THREADABNOR("Arithmetic error! pEnv->co_swap is NULL");
		return dave_false;
	}

	if(pEnv->co_swap != &(pCore->swap))
	{
		THREADABNOR("Arithmetic error! swap mismatch:%x/%x", pEnv->co_swap, &(pCore->swap));
		return dave_false;
	}

	pEnv->co_swap = NULL;

	coroutine_swap_run(&(pCore->swap), &(pEnv->base_swap));

	return dave_true;
}

static inline void
_coroutine_release(void *co)
{
	CoCore *pCore = (CoCore *)co;

	if(pCore != NULL)
	{
		dave_free(pCore->swap.ss_sp);

		dave_free(pCore);
	}
}

// =====================================================================

void
coroutine_core_init(void)
{
	_coroutine_core_init();

	_coroutine_stack_size = cfg_get_ub(CFG_COROUTINE_STACK_SIZE, COROUTINE_CORE_STACK_DEFAULT_SIZE);
}

void
coroutine_core_exit(void)
{
	_coroutine_core_exit();
}

void
coroutine_core_creat(void)
{
	_coroutine_stack_size = cfg_get_ub(CFG_COROUTINE_STACK_SIZE, COROUTINE_CORE_STACK_DEFAULT_SIZE);
}

void
coroutine_core_die(void)
{

}

void
coroutine_set_stack_size(ub size)
{
	_coroutine_stack_size = size;

	if(_coroutine_stack_size >= 8 * 1024 * 1024)
	{
		THREADABNOR("Note that you may have set a stack size(%ld) that is too large.",
			_coroutine_stack_size);
	}

	cfg_set_ub(CFG_COROUTINE_STACK_SIZE, _coroutine_stack_size);	
}

ub
coroutine_get_stack_size(void)
{
	return _coroutine_stack_size;
}

void *
coroutine_create(coroutine_core_fun fun_ptr, void *fun_param)
{
	return _coroutine_create(fun_ptr, fun_param);
}

dave_bool
coroutine_resume(void *co)
{
	return _coroutine_resume(co);
}

dave_bool
coroutine_yield(void *co)
{
	return _coroutine_yield(co);
}

void
coroutine_release(void *co)
{
	_coroutine_release(co);
}

#endif

