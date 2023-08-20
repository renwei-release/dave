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
#include "dave_base.h"
#include "dave_os.h"
#include "queue_server_message.h"
#include "queue_server_map.h"
#include "queue_server_debug.h"

static ThreadId _queue_server_thread;

static void
_queue_server_remote_ready(ThreadRemoteIDReadyMsg *pReady)
{
	queue_server_map_add(pReady->remote_thread_name, pReady->globally_identifier);
}

static void
_queue_server_remote_remove(ThreadRemoteIDRemoveMsg *pRemove)
{
	queue_server_map_del(pRemove->remote_thread_name, pRemove->globally_identifier);
}

static void
_queue_server_init(MSGBODY *msg)
{
	queue_server_message_init();
	queue_server_map_init();
}

static void
_queue_server_main(MSGBODY *msg)
{
	switch((ub)(msg->msg_id))
	{
		case MSGID_DEBUG_REQ:
				queue_server_debug(msg->msg_src, (DebugReq *)(msg->msg_body));
			break;
		case MSGID_REMOTE_THREAD_ID_READY:
				_queue_server_remote_ready((ThreadRemoteIDReadyMsg *)(msg->msg_body));
			break;
		case MSGID_REMOTE_THREAD_ID_REMOVE:
				_queue_server_remote_remove((ThreadRemoteIDRemoveMsg *)(msg->msg_body));
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
	queue_server_map_exit();
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

