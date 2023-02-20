/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __COROUTINE_ARM_SWAP_H__
#define __COROUTINE_ARM_SWAP_H__

#if defined(__aarch64__)

typedef struct {
	void **stack_pointer;
	void *argument;

	size_t ss_size;
	char *ss_sp;
} CoSwap;

typedef struct {
	CoSwap base_swap;
	CoSwap *co_swap;
} CoThreadEnv;

void coroutine_swap_make(CoSwap *pSwap, void *fun, const void *param);

void coroutine_swap_run(CoSwap *pCurrentSwap, CoSwap *pPendingSwap);

#endif
#endif


