/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_parameter.h"
#if defined(__DAVE_BASE__) && defined(ENABLE_THREAD_COROUTINE) && defined(__x86_64__)
#include "dave_base.h"
#include "dave_tools.h"
#include "coroutine_x86_swap.h"

extern void coroutine_swap(CoSwap *, CoSwap *) asm("coroutine_swap");

// =====================================================================

void
coroutine_swap_make(CoSwap *pSwap, void *fun, const void *param)
{
	char *sp = pSwap->ss_sp + pSwap->ss_size;

	sp = (char*) ((unsigned long)sp & -16LL);

	dave_memset(pSwap->regs, 0x00, sizeof(pSwap->regs));

	pSwap->regs[ kRSP ] = sp - 8;

	pSwap->regs[ kRETAddr] = (char *)fun;

	pSwap->regs[ kRDI ] = (char*)param;
	pSwap->regs[ kRSI ] = (char*)NULL;
}

void
coroutine_swap_run(CoSwap *pCurrentSwap, CoSwap *pPendingSwap)
{
	coroutine_swap(pCurrentSwap, pPendingSwap);
}

#endif

