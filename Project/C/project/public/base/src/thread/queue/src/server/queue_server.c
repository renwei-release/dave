/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_queue.h"
#if defined(QUEUE_STACK_SERVER)
#include "dave_tools.h"
#include "dave_verno.h"
#include "dave_base.h"
#include "dave_os.h"
#include "queue_server_message.h"
#include "base_rxtx.h"

static ThreadId _queue_server_thread;

static void
_queue_server_debug(ThreadId src, DebugReq *pReq)
{
	DebugRsp *pRsp = thread_msg(pRsp);

	switch(pReq->msg[0])
	{
		case 'i':
				queue_server_message_info(pRsp->msg, sizeof(pRsp->msg));
			break;
		default:
				dave_strcpy(pRsp->msg, pReq->msg, sizeof(pRsp->msg));
			break;
	}
	pRsp->ptr = pReq->ptr;

	id_msg(src, MSGID_DEBUG_RSP, pRsp);
}

static void
_queue_server_init(MSGBODY *msg)
{
	queue_server_message_init();
}

static void
_queue_server_main(MSGBODY *msg)
{
	switch((ub)(msg->msg_id))
	{
		case MSGID_DEBUG_REQ:
				_queue_server_debug(msg->msg_src, (DebugReq *)(msg->msg_body));
			break;
		case MSGID_QUEUE_UPLOAD_MESSAGE_REQ:
				queue_server_message_upload(msg->msg_src, (QueueUploadMsgReq *)(msg->msg_body));
			break;
		case MSGID_QUEUE_DOWNLOAD_MESSAGE_REQ:
				queue_server_message_download(msg->msg_src, (QueueDownloadMsgReq *)(msg->msg_body));
			break;
		default:
			break;
	}
}

static void
_queue_server_exit(MSGBODY *msg)
{
	queue_server_message_exit();
}

// =====================================================================

void
queue_server_init(void)
{
	ub thread_number = dave_os_cpu_process_number();

	_queue_server_thread = base_thread_creat(QUEUE_SERVER_THREAD_NAME, thread_number, THREAD_THREAD_FLAG|THREAD_CORE_FLAG, _queue_server_init, _queue_server_main, _queue_server_exit);
	if(_queue_server_thread == INVALID_THREAD_ID)
		base_restart(QUEUE_SERVER_THREAD_NAME);
}

void
queue_server_exit(void)
{
	if(_queue_server_thread != INVALID_THREAD_ID)
		base_thread_del(_queue_server_thread);
	_queue_server_thread = INVALID_THREAD_ID;
}

#endif

