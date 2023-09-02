/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_PRODUCT_BASE__
#include "dave_base.h"
#include "dave_os.h"
#include "dave_tools.h"
#include "base_log.h"

static void
_memory_debug(void)
{
	DebugRsp *pRsp = thread_reset_msg(pRsp);

	BASELOG("memory overflow debug");

	t_stdio_print_hex("1", &(((u8 *)(pRsp))[sizeof(pRsp->msg)]), 10);
	dave_memset(&(((u8 *)(pRsp))[sizeof(pRsp->msg)]), 0x00, 10);
	t_stdio_print_hex("2", &(((u8 *)(pRsp))[sizeof(pRsp->msg)]), 10);

	thread_msg_release(pRsp);
}

// =====================================================================

void
base_debug(ThreadId src, DebugReq *pReq)
{
	DebugRsp *pRsp = thread_reset_msg(pRsp);

	switch(pReq->msg[0])
	{
		case 'm':
				_memory_debug();
			break;
		default:
			break;
	}
	if(pRsp->msg[0] == '\0')
		dave_strcpy(pRsp->msg, pReq->msg, sizeof(pRsp->msg));
	pRsp->ptr = pReq->ptr;

	id_msg(src, MSGID_DEBUG_RSP, pRsp);
}

#endif

