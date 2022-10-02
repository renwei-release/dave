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
#include "dave_3rdparty.h"
#include "dave_os.h"
#include "thread_struct.h"
#include "thread_mem.h"
#include "thread_thread.h"
#include "thread_tools.h"
#include "thread_coroutine.h"
#include "coroutine_core.h"
#include "thread_log.h"

 #define ENABLE_THREAD_COROUTINE_CORE

#ifdef ENABLE_THREAD_COROUTINE_CORE

#define COROUTINE_CORE_STACK_DEFAULT_SIZE 128 * 1024
#define TID_MAX DAVE_SYS_THREAD_ID_MAX
#define CFG_COROUTINE_STACK_SIZE "CoroutineStackSize"

typedef void* (* co_swap_callback_fun)(void *param_1, void *param_2);

/*
// 64 bit
// low | regs[00]: r15 |
//     | regs[01]: r14 |
//     | regs[02]: r13 |
//     | regs[03]: r12 |
//     | regs[04]: r9  |
//     | regs[05]: r8  | 
//     | regs[06]: rbp |
//     | regs[07]: rdi |
//     | regs[08]: rsi |
//     | regs[09]: ret |
//     | regs[10]: rdx |
//     | regs[11]: rcx | 
//     | regs[12]: rbx |
// hig | regs[13]: rsp |
*/

typedef enum {
	kRDI = 7,
	kRSI = 8,
	kRETAddr = 9,
	kRSP = 13,
} regs_map;

typedef struct {
	void *regs[14];
	size_t ss_size;
	char *ss_sp;
} CoSwap;

typedef struct {
	CoSwap base_swap;
	CoSwap *co_swap;
} CoThreadEnv;

typedef struct {
	coroutine_core_fun fun_ptr;
	void *fun_param;

	CoSwap swap;

	CoThreadEnv *env;
} CoCore;

extern void coroutine_swap(CoSwap *, CoSwap *) asm("coroutine_swap");

static CoThreadEnv *_co_thread_env[ TID_MAX ] = { 0 };
static ub _coroutine_stack_size = 0;

static void *
_coroutine_swap_function(void *param_1, void *param_2)
{
	CoCore *pCore = (CoCore *)param_1;

	if(pCore->fun_ptr != NULL)
	{
		pCore->fun_ptr(pCore->fun_param);
	}

	coroutine_yield(pCore);

	return NULL;
}

static inline void
_coroutine_swap_clean(CoSwap *pSwap)
{
	dave_memset(pSwap, 0x00, sizeof(CoSwap));
}

static inline void
_coroutine_swap_make(CoSwap *pSwap, co_swap_callback_fun fun, const void *param_1, const void *param_2)
{
	char *sp = pSwap->ss_sp + pSwap->ss_size;

	sp = (char*) ((unsigned long)sp & -16LL);

	dave_memset(pSwap->regs, 0x00, sizeof(pSwap->regs));

	pSwap->regs[ kRSP ] = sp - 8;

	pSwap->regs[ kRETAddr] = (char *)fun;

	pSwap->regs[ kRDI ] = (char*)param_1;
	pSwap->regs[ kRSI ] = (char*)param_2;
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

		_coroutine_swap_make(&(_co_thread_env[tid_index]->base_swap), NULL, NULL, NULL);

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

	_coroutine_stack_size = cfg_get_ub(CFG_COROUTINE_STACK_SIZE, COROUTINE_CORE_STACK_DEFAULT_SIZE);
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

static inline void
_coroutine_swap_run(CoSwap *pCurrentSwap, CoSwap *pPendingSwap)
{
	coroutine_swap(pCurrentSwap, pPendingSwap);
}

static inline void *
_coroutine_create(coroutine_core_fun fun_ptr, void *fun_param)
{
	CoCore *pCore = dave_malloc(sizeof(CoCore));

	pCore->fun_ptr = fun_ptr;
	pCore->fun_param = fun_param;

	pCore->swap.ss_size = _coroutine_stack_size;
	pCore->swap.ss_sp = dave_malloc(pCore->swap.ss_size);

	_coroutine_swap_make(&(pCore->swap), _coroutine_swap_function, pCore, NULL);

	pCore->env = _coroutine_thread_env();

	return pCore;
}

static inline void
_coroutine_resume(void *co)
{
	CoCore *pCore = (CoCore *)co;
	CoThreadEnv *pEnv;

	if(pCore == NULL)
	{
		THREADABNOR("Arithmetic error! pCore is NULL");
		return;
	}

	pEnv = pCore->env;
	if(pEnv == NULL)
	{
		THREADABNOR("Arithmetic error! pEnv is NULL");
		return;
	}

	if(pEnv->co_swap != NULL)
	{
		THREADABNOR("Arithmetic error! co_swap:%x", pEnv->co_swap);
		return;
	}

	pEnv->co_swap = &(pCore->swap);

	_coroutine_swap_run(&(pEnv->base_swap), pEnv->co_swap);
}

static inline void
_coroutine_yield(void *co)
{
	CoCore *pCore = (CoCore *)co;
	CoThreadEnv *pEnv;

	if(pCore == NULL)
	{
		THREADABNOR("Arithmetic error! pCore is NULL");
		return;
	}	

	pEnv = pCore->env;
	if(pEnv == NULL)
	{
		THREADABNOR("Arithmetic error! pEnv is NULL");
		return;
	}

	if(pEnv->co_swap == NULL)
	{
		THREADABNOR("Arithmetic error! pEnv->co_swap is NULL");
		return;
	}

	if(pEnv->co_swap != &(pCore->swap))
	{
		THREADABNOR("Arithmetic error! swap mismatch:%x/%x", pEnv->co_swap, &(pCore->swap));
		return;
	}

	pEnv->co_swap = NULL;

	_coroutine_swap_run(&(pCore->swap), &(pEnv->base_swap));
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

#endif

// =====================================================================

void
coroutine_core_init(void)
{
#ifdef ENABLE_THREAD_COROUTINE_CORE
	_coroutine_core_init();
#endif
}

void
coroutine_core_exit(void)
{
#ifdef ENABLE_THREAD_COROUTINE_CORE
	_coroutine_core_exit();
#endif
}

void *
coroutine_create(coroutine_core_fun fun_ptr, void *fun_param)
{
#ifdef ENABLE_THREAD_COROUTINE_CORE
	return _coroutine_create(fun_ptr, fun_param);
#else
	return dave_co_create(fun_ptr, fun_param);
#endif
}

void
coroutine_resume(void *co)
{
#ifdef ENABLE_THREAD_COROUTINE_CORE
	_coroutine_resume(co);
#else
	dave_co_resume(co);
#endif
}

void
coroutine_yield(void *co)
{
#ifdef ENABLE_THREAD_COROUTINE_CORE
	_coroutine_yield(co);
#else
	dave_co_yield(co);
#endif
}

void
coroutine_release(void *co)
{
#ifdef ENABLE_THREAD_COROUTINE_CORE
	_coroutine_release(co);
#else
	dave_co_release(co);
#endif
}

#endif

