/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_queue.h"
#if defined(QUEUE_STACK_CLIENT)
#include "dave_base.h"
#include "dave_tools.h"
#include "queue_client_message.h"
#include "queue_client_map.h"
#include "queue_log.h"

static ub
_queue_client_info(s8 *info_ptr, ub info_len)
{
	ub info_index = 0;

	info_index += queue_client_map_info(&info_ptr[info_index], info_len-info_index);

	return info_index;
}

// =====================================================================

void
queue_client_debug(ThreadId src, DebugReq *pReq)
{
	DebugRsp *pRsp = thread_msg(pRsp);

	switch(pReq->msg[0])
	{
		case 'i':
				_queue_client_info(pRsp->msg, sizeof(pRsp->msg));
			break;
		default:
				dave_strcpy(pRsp->msg, pReq->msg, sizeof(pRsp->msg));
			break;
	}
	pRsp->ptr = pReq->ptr;

	id_msg(src, MSGID_DEBUG_RSP, pRsp);
}

#endif

