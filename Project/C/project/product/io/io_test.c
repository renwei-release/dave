/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_PRODUCT_IO__
#include "dave_base.h"
#include "dave_os.h"
#include "dave_tools.h"

// =====================================================================

void
io_debug(ThreadId src, DebugReq *pReq)
{
	DebugRsp *pRsp = thread_msg(pRsp);

	dave_strcpy(pRsp->msg, pReq->msg, sizeof(pRsp->msg));
	pRsp->ptr = pReq->ptr;

	id_msg(src, MSGID_DEBUG_RSP, pRsp);
}

#endif

