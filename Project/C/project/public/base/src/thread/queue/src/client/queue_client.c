/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_queue.h"
#if defined(QUEUE_STACK_CLIENT)
#include "dave_tools.h"
#include "dave_base.h"
#include "dave_os.h"
#include "queue_client_map.h"
#include "queue_client_message.h"
#include "queue_client_debug.h"
#include "queue_log.h"

static ThreadId _queue_client_thread;

static void
_queue_client_remote_ready(ThreadRemoteReadyMsg *pReady)
{
	queue_client_map_add(pReady->remote_thread_name);
}

static void
_queue_client_remote_remove(ThreadRemoteRemoveMsg *pRemove)
{
	queue_client_map_del(pRemove->remote_thread_name);
}

static void
_queue_client_remote_id_ready(ThreadRemoteIDReadyMsg *pReady)
{
	queue_client_map_add(pReady->remote_thread_name);

	queue_client_gid_map_add(pReady->remote_thread_name, pReady->globally_identifier);
}

static void
_queue_client_remote_id_remove(ThreadRemoteIDRemoveMsg *pRemove)
{
	if(dave_strcmp(pRemove->remote_thread_name, QUEUE_SERVER_THREAD_NAME) == dave_true)
	{
		queue_client_map_queue_del_all(pRemove->globally_identifier);
	}

	queue_client_gid_map_del(pRemove->remote_thread_name);
}

static void
_queue_client_local_ready(ThreadLocalReadyMsg *pReady)
{
	if((pReady->thread_flag & THREAD_PRIVATE_FLAG) == 0x00)
	{
		queue_client_map_add(pReady->local_thread_name);
	}
}

static void
_queue_client_local_remove(ThreadLocalRemoveMsg *pRemove)
{
	queue_client_map_del(pRemove->local_thread_name);
}

static void
_queue_client_init(MSGBODY *msg)
{
	queue_client_map_init();
	queue_client_message_init();
}

static void
_queue_client_main(MSGBODY *msg)
{
	switch((ub)(msg->msg_id))
	{
		case MSGID_DEBUG_REQ:
				queue_client_debug(msg->msg_src, (DebugReq *)(msg->msg_body));
			break;
		case MSGID_REMOTE_THREAD_READY:
				_queue_client_remote_ready((ThreadRemoteReadyMsg *)(msg->msg_body));
			break;
		case MSGID_REMOTE_THREAD_REMOVE:
				_queue_client_remote_remove((ThreadRemoteRemoveMsg *)(msg->msg_body));
			break;
		case MSGID_REMOTE_THREAD_ID_READY:
				_queue_client_remote_id_ready((ThreadRemoteIDReadyMsg *)(msg->msg_body));
			break;
		case MSGID_REMOTE_THREAD_ID_REMOVE:
				_queue_client_remote_id_remove((ThreadRemoteIDRemoveMsg *)(msg->msg_body));
			break;
		case MSGID_LOCAL_THREAD_READY:
				_queue_client_local_ready((ThreadLocalReadyMsg *)(msg->msg_body));
			break;
		case MSGID_LOCAL_THREAD_REMOVE:
				_queue_client_local_remove((ThreadLocalRemoveMsg *)(msg->msg_body));
			break;
		case MSGID_QUEUE_UPDATE_STATE_REQ:
				queue_client_message_update(msg->msg_src, (QueueUpdateStateReq *)(msg->msg_body));
			break;
		case MSGID_QUEUE_RUN_MESSAGE_RSP:
				queue_client_message_run_rsp((QueueRunMsgRsp *)(msg->msg_body));
			break;
		default:
			break;
	}
}

static void
_queue_client_exit(MSGBODY *msg)
{
	queue_client_map_exit();
	queue_client_message_exit();
}

// =====================================================================

void
queue_client_init(void)
{
	ub thread_number = dave_os_cpu_process_number();

	_queue_client_thread = base_thread_creat(QUEUE_CLIENT_THREAD_NAME, thread_number, THREAD_THREAD_FLAG, _queue_client_init, _queue_client_main, _queue_client_exit);
	if(_queue_client_thread == INVALID_THREAD_ID)
		base_restart(QUEUE_CLIENT_THREAD_NAME);
}

void
queue_client_exit(void)
{
	if(_queue_client_thread != INVALID_THREAD_ID)
		base_thread_del(_queue_client_thread);
	_queue_client_thread = INVALID_THREAD_ID;
}

#endif

