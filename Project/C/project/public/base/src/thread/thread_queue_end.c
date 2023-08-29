/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "thread_parameter.h"
#include "thread_struct.h"
#include "thread_lock.h"
#include "thread_mem.h"
#include "thread_tools.h"
#include "thread_coroutine.h"
#include "thread_log.h"

static ThreadId _queue_client_thread = INVALID_THREAD_ID;

// =====================================================================

void
thread_queue_end(ThreadStruct *pThread, BaseMsgType msg_type)
{
	if((msg_type == BaseMsgType_Unicast_queue)
		&& (pThread->thread_flag & THREAD_THREAD_FLAG)
		&& ((pThread->thread_flag & THREAD_REMOTE_FLAG) == 0x00)
		&& ((pThread->thread_flag & THREAD_CORE_FLAG) == 0x00))
	{
		QueueRunMsgRsp *pRsp = thread_msg(pRsp);

		pRsp->ret = RetCode_OK;
		dave_strcpy(pRsp->name, pThread->thread_name, sizeof(pRsp->name));
		pRsp->msg_number = thread_num_msg(pThread, MSGID_RESERVED);
		pRsp->thread_number = pThread->level_number;
		pRsp->ptr = NULL;

		if(_queue_client_thread == INVALID_THREAD_ID)
		{
			_queue_client_thread = thread_id(QUEUE_CLIENT_THREAD_NAME);
		}

		id_msg(_queue_client_thread, MSGID_QUEUE_RUN_MESSAGE_RSP, pRsp);
	}
}

#endif

