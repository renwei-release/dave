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
#include "dave_verno.h"
#include "base_test.h"

static ThreadId _base_thread = INVALID_THREAD_ID;

static void
_base_thread_remote_id_ready(ThreadRemoteIDReadyMsg *pReady)
{
	RPCDebugMsg *pDebug = thread_msg(pDebug);

	DAVELOG("%lx/%s/%s/%s\n",
		pReady->remote_thread_id, thread_name(pReady->remote_thread_id),
		pReady->remote_thread_name, pReady->globally_identifier);

	pDebug->u32_debug = 1234567890;

	id_msg(pReady->remote_thread_id, MSGID_RPC_DEBUG_MSG, pDebug);
}

static void
_base_thread_remote_id_remove(ThreadRemoteIDRemoveMsg *pReady)
{
	DAVELOG("%lx/%s/%s\n",
		pReady->remote_thread_id,
		pReady->remote_thread_name, pReady->globally_identifier);
}

static void
_base_thread_rpc_debug(ThreadId src, RPCDebugMsg *pDebug)
{
	DAVELOG("%lx/%s u32_debug:%d\n", src, thread_name(src), pDebug->u32_debug);
}

static void
_base_thread_init(MSGBODY *msg)
{

}

static void
_base_thread_main(MSGBODY *msg)
{
	switch((sb)(msg->msg_id))
	{
		case MSGID_DEBUG_REQ:
				base_debug(msg->msg_src, (DebugReq *)(msg->msg_body));
			break;
		case MSGID_ECHO:
				base_echo(msg->msg_src, (MsgIdEcho *)(msg->msg_body));
			break;
		case MSGID_REMOTE_THREAD_ID_READY:
				_base_thread_remote_id_ready((ThreadRemoteIDReadyMsg *)(msg->msg_body));
			break;
		case MSGID_REMOTE_THREAD_ID_REMOVE:
				_base_thread_remote_id_remove((ThreadRemoteIDRemoveMsg *)(msg->msg_body));
			break;
		case MSGID_RPC_DEBUG_MSG:
				_base_thread_rpc_debug(msg->msg_src, (RPCDebugMsg *)(msg->msg_body));
			break;
		default:
			break;
	}
}

static void
_base_thread_exit(MSGBODY *msg)
{

}

// =====================================================================

void
dave_product_init(void)
{
	_base_thread = base_thread_creat(dave_verno_my_product(), 1, THREAD_THREAD_FLAG|THREAD_COROUTINE_FLAG, _base_thread_init, _base_thread_main, _base_thread_exit);
	if(_base_thread == INVALID_THREAD_ID)
		base_restart(dave_verno_my_product());
}

void
dave_product_exit(void)
{
	if(_base_thread != INVALID_THREAD_ID)
		base_thread_del(_base_thread);
	_base_thread = INVALID_THREAD_ID;
}

#endif

