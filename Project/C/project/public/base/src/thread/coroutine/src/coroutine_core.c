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
#include "coroutine_mem.h"
#include "coroutine_arch.h"
#include "thread_log.h"

#define COCORE_MAGIC_DATA 0xabc123eeff
#define COROUTINE_CORE_STACK_DEFAULT_SIZE 128 * 1024
#define CALL_STACK_MAX 128
#define TID_MAX DAVE_SYS_THREAD_ID_MAX
#define CFG_COROUTINE_STACK_SIZE "CoroutineStackSize"

typedef void* (* co_swap_callback_fun)(void *param);

typedef struct {
	CoSwap base_swap;

	sb call_stack_index;
	CoSwap *call_stack_ptr[CALL_STACK_MAX];
} CoThreadEnv;

typedef struct {
	ub magic_data;
	dave_bool be_in_use;

	coroutine_core_fun fun_core;
	void *fun_param;

	size_t ss_size;
	char *ss_sp;

	CoSwap swap;

	CoThreadEnv *env;
} CoCore;

static CoThreadEnv *_co_thread_env[ TID_MAX ] = { 0 };
static ub _coroutine_stack_size = 0;

static int
_coroutine_swap_function(CoCore *pCore, void *param)
{
	if(pCore == NULL)
	{
		THREADABNOR("pCore is NULL");
		return 0;
	}

	if(pCore->magic_data != COCORE_MAGIC_DATA)
	{
		THREADABNOR("pCore has invalid magic data:%lx", pCore->magic_data);
		return 0;
	}

	if(pCore->fun_core != NULL)
	{
		pCore->fun_core(pCore->fun_param);
	}

	pCore->be_in_use = dave_false;

	if(coroutine_yield(pCore) == dave_false)
	{
		THREADABNOR("yield failed!");
	}

	THREADLOG("It is impossible for the code to run here, and if it runs here, something must have gone wrong.");

	return 0;
}

static inline CoThreadEnv *
_coroutine_env_malloc(void)
{
	CoThreadEnv *pEnv = coroutine_malloc(sizeof(CoThreadEnv));
	ub stack_index;

	dave_memset(pEnv, 0x00, sizeof(CoThreadEnv));

	coroutine_swap_make(&(pEnv->base_swap), NULL, NULL, NULL, 0, NULL);

	pEnv->call_stack_index = 0;

	for(stack_index=0; stack_index<CALL_STACK_MAX; stack_index++)
	{
		pEnv->call_stack_ptr[stack_index] = NULL;
	}

	pEnv->call_stack_ptr[pEnv->call_stack_index ++] = &(pEnv->base_swap);

	return pEnv;
}

static inline void
_coroutine_env_free(CoThreadEnv *pEnv)
{
	if(pEnv != NULL)
	{	
		coroutine_free(pEnv);
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
_coroutine_create(coroutine_core_fun fun_core, void *fun_param, MSGBODY *msg)
{
	CoCore *pCore = (CoCore *)coroutine_malloc(sizeof(CoCore));

	pCore->magic_data = COCORE_MAGIC_DATA;
	pCore->be_in_use = dave_true;

	pCore->fun_core = fun_core;
	pCore->fun_param = fun_param;

	pCore->ss_size = _coroutine_stack_size;
	pCore->ss_sp = dave_malloc(pCore->ss_size);

	coroutine_swap_make(&(pCore->swap), _coroutine_swap_function, pCore, pCore->ss_sp, pCore->ss_size, msg);

	pCore->env = _coroutine_thread_env();

	return pCore;
}

static inline dave_bool
_coroutine_resume(void *co)
{
	CoCore *pCore = (CoCore *)co;
	CoThreadEnv *pEnv;
	CoSwap *pCurrentSwap, *pPendingSwap;

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

	if((pEnv->call_stack_index <= 0) || (pEnv->call_stack_index >= CALL_STACK_MAX))
	{
		THREADABNOR("invalid call_stack_index:%ld", pEnv->call_stack_index);
		return dave_false;
	}

	pCurrentSwap = pEnv->call_stack_ptr[pEnv->call_stack_index - 1];
	pPendingSwap = pEnv->call_stack_ptr[pEnv->call_stack_index ++] = &(pCore->swap);

	coroutine_swap_run(pCurrentSwap, pPendingSwap);

	return dave_true;
}

static inline dave_bool
_coroutine_yield(void *co)
{
	CoCore *pCore = (CoCore *)co;
	CoThreadEnv *pEnv;
	CoSwap *pCurrentSwap, *pPendingSwap;

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

	if((pEnv->call_stack_index <= 0) || (pEnv->call_stack_index >= CALL_STACK_MAX))
	{
		THREADABNOR("invalid call_stack_index:%ld", pEnv->call_stack_index);
		return dave_false;
	}

	pCurrentSwap = pEnv->call_stack_ptr[pEnv->call_stack_index - 1];
	pPendingSwap = pEnv->call_stack_ptr[pEnv->call_stack_index - 2];
	pEnv->call_stack_index --;

	coroutine_swap_run(pCurrentSwap, pPendingSwap);

	return dave_true;
}

static inline void
_coroutine_release(void *co)
{
	CoCore *pCore = (CoCore *)co;

	if(pCore != NULL)
	{
		dave_free(pCore->ss_sp);

		pCore->magic_data = 0x00;
		pCore->be_in_use = dave_false;

		pCore->ss_size = 0;
		pCore->ss_sp = NULL;

		pCore->fun_core = NULL;
		pCore->fun_param = NULL;

		pCore->env = NULL;

		coroutine_free(pCore);
	}
}

static inline void
_coroutine_load_stack_size(void)
{
	ub stack_size_backup = _coroutine_stack_size;

	_coroutine_stack_size = cfg_get_ub(CFG_COROUTINE_STACK_SIZE, COROUTINE_CORE_STACK_DEFAULT_SIZE);

	if(stack_size_backup != _coroutine_stack_size)
	{
		THREADLOG("%s change:%d->%d", CFG_COROUTINE_STACK_SIZE, stack_size_backup, _coroutine_stack_size)
	}
}

// =====================================================================

void
coroutine_core_init(void)
{
	coroutine_mem_init();

	_coroutine_core_init();
}

void
coroutine_core_exit(void)
{
	_coroutine_core_exit();

	coroutine_mem_exit();
}

void
coroutine_core_reload_cfg(CFGUpdate *pUpdate)
{
	if(dave_strcmp(pUpdate->cfg_name, CFG_COROUTINE_STACK_SIZE) == dave_true)
	{
		_coroutine_load_stack_size();
	}
}

void
coroutine_core_creat(void)
{
	_coroutine_load_stack_size();
}

void
coroutine_core_die(void)
{

}

void *
coroutine_create(coroutine_core_fun fun_core, void *fun_param, MSGBODY *msg)
{
	return _coroutine_create(fun_core, fun_param, msg);
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

dave_bool
coroutine_be_in_use(void *co)
{
	CoCore *pCore = (CoCore *)co;

	return pCore->be_in_use;
}

#endif

