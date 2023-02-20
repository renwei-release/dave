/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __COROUTINE_X86_SWAP_H__
#define __COROUTINE_X86_SWAP_H__

#if defined(__x86_64__)

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

void coroutine_swap_make(CoSwap *pSwap, void *fun, const void *param);

void coroutine_swap_run(CoSwap *pCurrentSwap, CoSwap *pPendingSwap);

#endif
#endif

