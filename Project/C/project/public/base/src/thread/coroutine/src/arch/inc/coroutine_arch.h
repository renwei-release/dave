/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __COROUTINE_ARCH_H__
#define __COROUTINE_ARCH_H__

typedef struct {
	void *regs[14];

	ThreadId msg_src;
	ThreadId msg_dst;
	ub msg_id;
} CoSwap;

void coroutine_swap_make(CoSwap *pSwap, void *fun, const void *param, char *ss_sp, size_t ss_size, MSGBODY *msg);

void coroutine_swap_run(CoSwap *pCurrentSwap, CoSwap *pPendingSwap);

#endif

