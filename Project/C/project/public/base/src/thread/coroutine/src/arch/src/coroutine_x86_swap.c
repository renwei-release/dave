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
#include "thread_tools.h"
#include "coroutine_arch.h"
#include "thread_log.h"

extern void coroutine_swap(CoSwap *, CoSwap *) asm("coroutine_swap");

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

#define kRDI 7
#define kRSI 8
#define kRETAddr 9
#define kRSP 13

// =====================================================================

void
coroutine_swap_make(CoSwap *pSwap, void *fun, const void *param, char *ss_sp, size_t ss_size, MSGBODY *msg)
{
	char *sp;

	dave_memset(pSwap, 0x00, sizeof(CoSwap));

	if(ss_sp != NULL)
	{
		sp = ss_sp + ss_size;
		sp = (char*) ((unsigned long)sp & -16LL);
		pSwap->regs[ kRSP ] = sp - 8;
	}

	pSwap->regs[ kRETAddr] = fun;
	pSwap->regs[ kRDI ] = (void *)param;
	pSwap->regs[ kRSI ] = NULL;

	if(msg != NULL)
	{
		pSwap->msg_src = msg->msg_src;
		pSwap->msg_dst = msg->msg_dst;
		pSwap->msg_id = msg->msg_id;
	}
}

void
coroutine_swap_run(CoSwap *pCurrentSwap, CoSwap *pPendingSwap)
{
	THREADDEBUG("pCurrentSwap:%s->%s:%s pPendingSwap:%s->%s:%s",
		thread_id_to_name(pCurrentSwap->msg_src), thread_id_to_name(pCurrentSwap->msg_dst), msgstr(pCurrentSwap->msg_id),
		thread_id_to_name(pPendingSwap->msg_src), thread_id_to_name(pPendingSwap->msg_dst), msgstr(pPendingSwap->msg_id));

	coroutine_swap(pCurrentSwap, pPendingSwap);
}

#endif

