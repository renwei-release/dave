/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_os.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "base_tools.h"
#include "socket_tools.h"
#include "socket_parameters.h"
#include "socket_external.h"
#include "socket_info.h"
#include "socket_log.h"

#define SOCKET_THREAD_MAX 16

static ThreadId _socket_thread = INVALID_THREAD_ID;

static ub
_socket_thread_number(void)
{
	ub thread_number = dave_os_cpu_process_number();

	if(thread_number >= SOCKET_THREAD_MAX)
	{
		thread_number = SOCKET_THREAD_MAX;
	}

	return thread_number;
}

static void
_socket_restart(RESTARTREQMSG *pReq)
{
	if(pReq->times == 1)
	{
		socket_external_exit();
	}
}

static void
_socket_debug(ThreadId src, DebugReq *pReq, ub wakeup_index)
{
	DebugRsp *pRsp = thread_reset_msg(pRsp);
	ub info_index = 0;

	switch(pReq->msg[0])
	{
		case 'i':
				info_index += socket_info(pRsp->msg, sizeof(pRsp->msg), &(pReq->msg[1]));
			break;
		default:
				info_index += dave_snprintf(pRsp->msg, sizeof(pRsp->msg), "socket empty message!");
			break;
	}
	pRsp->ptr = pReq->ptr;

	id_msg(src, MSGID_DEBUG_RSP, pRsp);
}

static void
_socket_plugin(ThreadId dst, s32 father_socket, s32 socket, SocNetInfo *pNetInfo, void *user_ptr)
{
	SocketPlugIn *pPlugin = thread_msg(pPlugin);

	pPlugin->father_socket = father_socket;
	pPlugin->child_socket = socket;
	T_CopyNetInfo(&(pPlugin->NetInfo), pNetInfo);
	pPlugin->thread_id = _socket_thread;
	pPlugin->ptr = user_ptr;

	id_msg(dst, SOCKET_PLUGIN, pPlugin);
}

static void
_socket_bing_rsp(SocketCore *pCore, ThreadId src, SocketBindReq *pReq)
{
	SocketBindRsp *pRsp = thread_msg(pRsp);

	if(pCore != NULL)
	{
		pRsp->socket = pCore->socket_external_index;
	}
	else
	{
		pRsp->socket = INVALID_SOCKET_ID;
	}
	T_CopyNetInfo(&(pRsp->NetInfo), &(pReq->NetInfo));
	if(pCore != NULL)
	{
		pRsp->BindInfo = SOCKETINFO_BIND_OK;
	}
	else
	{
		pRsp->BindInfo = SOCKETINFO_BIND_FAIL;
	}
	pRsp->thread_id = _socket_thread;
	pRsp->ptr = pReq->ptr;

	id_msg(src, SOCKET_BIND_RSP, pRsp);

	if(pCore != NULL)
	{
		SAFECODEv2W(pCore->opt_pv, pCore->bind_or_connect_rsp_flag = dave_true; );
	}
}

static void
_socket_bind_req(ThreadId src, SocketBindReq *pReq)
{
	SocketCore *pCore;

	pCore = socket_external_creat_service(src, &(pReq->NetInfo), pReq->ptr);

	_socket_bing_rsp(pCore, src, pReq);

	if((pCore != NULL) && (pCore->NetInfo.type == TYPE_SOCK_DGRAM))
	{
		_socket_plugin(pCore->owner, pCore->socket_external_index, pCore->socket_external_index, &(pCore->NetInfo), pReq->ptr);

		SOCKETTRACE("socket:%d/%d/%d owner:%s",
			pCore->socket_external_index, pCore->socket_internal_index, pCore->os_socket,
			thread_name(pCore->owner));
	}
}

static void
_socket_connect_rsp(SocketCore *pCore, SOCKETINFO info, ThreadId src, SocketConnectReq *pReq)
{
	SocketConnectRsp *pRsp;

	pRsp = thread_msg(pRsp);
	if(pCore != NULL)
	{
		pRsp->socket = pCore->socket_external_index;
	}
	else
	{
		pRsp->socket = INVALID_SOCKET_ID;
	}
	pRsp->NetInfo = pReq->NetInfo;
	pRsp->ConnectInfo = info;
	pRsp->thread_id = _socket_thread;
	pRsp->ptr = pReq->ptr;

	id_msg(src, SOCKET_CONNECT_RSP, pRsp);

	if(pCore != NULL)
	{
		SAFECODEv2W(pCore->opt_pv, pCore->bind_or_connect_rsp_flag = dave_true; );
	}
}

static void
_socket_connect_req(ThreadId src, SocketConnectReq *pReq)
{
	SOCKETINFO info = SOCKETINFO_MAX;
	SocketCore *pCore;

	pCore = socket_external_connect_service(src, &(pReq->NetInfo), pReq->ptr, &info);

	_socket_connect_rsp(pCore, info, src, pReq);

	if((pCore != NULL) && (info == SOCKETINFO_CONNECT_OK))
	{
		_socket_plugin(pCore->owner, INVALID_SOCKET_ID, pCore->socket_external_index, &(pCore->NetInfo), pReq->ptr);
	}
}

static void
_socket_disconnect_rsp(ThreadId src, SocketDisconnectReq *pReq)
{
	SocketDisconnectRsp *pRsp = thread_msg(pRsp);

	pRsp->socket = pReq->socket;
	pRsp->result = SOCKETINFO_DISCONNECT_OK;
	pRsp->ptr = pReq->ptr;

	id_msg(src, SOCKET_DISCONNECT_RSP, pRsp);
}

static void
_socket_disconnect_req(ThreadId src, SocketDisconnectReq *pReq)
{
	socket_external_close(src, pReq->socket);

	_socket_disconnect_rsp(src, pReq);
}

static void
_socket_write(ThreadId src, SocketWrite *pWrite)
{
	if(pWrite == NULL)
	{
		SOCKETABNOR("pWrite is NULL!");
		return;
	}

	if(pWrite->data == NULL)
	{
		SOCKETABNOR("pWrite->data is NULL!");
		return;
	}

	if(socket_external_send(src, pWrite->socket, &(pWrite->IPInfo), pWrite->data, pWrite->close_flag) == dave_false)
	{
		dave_mfree(pWrite->data);
	}

	if(pWrite->close_flag == SOCKETINFO_WRITE_THEN_CLOSE)
	{
		socket_external_close(src, pWrite->socket);
	}
}

static void
_socket_notify(SocketNotify *pNotify)
{
	socket_external_notify(pNotify);
}

static void
_socket_event(SocketRawEvent *pEvent)
{
	socket_external_event(pEvent);
}

static void
_socket_init(MSGBODY *msg)
{
	socket_core_init();

	socket_external_init();
}

static void
_socket_main(MSGBODY *msg)
{
	switch((ub)msg->msg_id)
	{
		case MSGID_RESTART_REQ:
				_socket_restart((RESTARTREQMSG *)(msg->msg_body));
			break;
		case MSGID_DEBUG_REQ:
				_socket_debug(msg->msg_src, (DebugReq *)(msg->msg_body), msg->thread_wakeup_index);
			break;
		case SOCKET_BIND_REQ:
				_socket_bind_req(msg->msg_src, (SocketBindReq *)(msg->msg_body));
			break;
		case SOCKET_CONNECT_REQ:
				_socket_connect_req(msg->msg_src, (SocketConnectReq *)(msg->msg_body));
			break;
		case SOCKET_DISCONNECT_REQ:
				_socket_disconnect_req(msg->msg_src, (SocketDisconnectReq *)(msg->msg_body));
			break;
		case SOCKET_WRITE:
				_socket_write(msg->msg_src, (SocketWrite *)(msg->msg_body));
			break;
		case SOCKET_NOTIFY:
				_socket_notify((SocketNotify *)(msg->msg_body));
			break;
		case SOCKET_RAW_EVENT:
				_socket_event((SocketRawEvent *)(msg->msg_body));
			break;
		default:
			break;
	}
}

static void
_socket_exit(MSGBODY *msg)
{
	socket_core_exit();
}

// =====================================================================

void
base_socket_init(void)
{
	ub thread_number = _socket_thread_number();

	_socket_thread = base_thread_creat(SOCKET_THREAD_NAME, thread_number, THREAD_THREAD_FLAG|THREAD_PRIVATE_FLAG, _socket_init, _socket_main, _socket_exit);
	if(_socket_thread == INVALID_THREAD_ID)
		base_restart("socket");
}

void
base_socket_exit(void)
{
	if(_socket_thread != INVALID_THREAD_ID)
		base_thread_del(_socket_thread);
	_socket_thread = INVALID_THREAD_ID;
}

#endif

