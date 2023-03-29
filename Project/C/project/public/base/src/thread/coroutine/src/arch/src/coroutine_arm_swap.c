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
#include "thread_tools.h"
#include "coroutine_arch.h"
#include "thread_log.h"

extern void coroutine_swap(CoSwap *, CoSwap *) asm("coroutine_swap");

//-------------
// 64 bit arm
//low | regs[0]: x19 |
//    | regs[1]: x20 |
//    | regs[2]: x21 |
//    | regs[3]: x22 |
//    | regs[4]: x23 |
//    | regs[5]: x24 |
//    | regs[6]: x25 |
//    | regs[7]: x26 |
//    | regs[8]: x27 |
//    | regs[9]: x28 |  
//    | regs[10]: x29|
//    | regs[11]: x30|//ret func addr
//    | regs[12]: sp |
//hig | regs[13]: x0 |

#define kARG1 13
#define kRETAddr_ARM64 11
#define kSP_ARM64 12

// =====================================================================

void
coroutine_swap_make(CoSwap *pSwap, void *fun, const void *param, char *ss_sp, size_t ss_size, MSGBODY *msg)
{
	char *sp;

	dave_memset(pSwap, 0x00, sizeof(CoSwap));

	if(ss_sp != NULL)
	{
		sp = ss_sp + ss_size - sizeof(void*);
		sp = (char*)((unsigned long)sp & -16LL);
		pSwap->regs[kSP_ARM64] = sp;
	}

	pSwap->regs[kRETAddr_ARM64] = fun;
	pSwap->regs[kARG1] = (void *)param;
	pSwap->ss_size = ss_size;
	pSwap->ss_sp = ss_sp;

	pSwap->fun = fun;
	pSwap->param = param;

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

