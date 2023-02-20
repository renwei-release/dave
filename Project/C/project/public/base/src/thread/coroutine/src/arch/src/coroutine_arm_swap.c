/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_parameter.h"
#if defined(__DAVE_BASE__) && defined(ENABLE_THREAD_COROUTINE) && defined(__aarch64__)
#include "dave_base.h"
#include "dave_tools.h"
#include "coroutine_arm_swap.h"

extern void coroutine_swap(CoSwap *, CoSwap *) asm("coroutine_swap");

const size_t COROUTINE_REGISTERS = 0xb0 / 8;

// =====================================================================

void
coroutine_swap_make(CoSwap *pSwap, void *fun, const void *param)
{
	/* Force 16-byte alignment */
	pSwap->stack_pointer = (void **)((ub)(pSwap->ss_sp) & ~0xF);

	pSwap->argument = param;
	pSwap->stack_pointer -= COROUTINE_REGISTERS;
	dave_memset(pSwap->stack_pointer, 0, sizeof(void*) * COROUTINE_REGISTERS);

	pSwap->stack_pointer[0xa0 / 8] = (void*)fun;
}

void
coroutine_swap_run(CoSwap *pCurrentSwap, CoSwap *pPendingSwap)
{
	coroutine_swap(pCurrentSwap, pPendingSwap);
}

#endif

