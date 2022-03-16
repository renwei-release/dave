/*
 * ================================================================================
 * (c) Copyright 2018 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * The main purpose of this module is to prevent the network interface from reading
 * data out of order. 
 * In a multi-threaded environment, messages that read data from the network enter
 * the queue in order, but threads that do not necessarily get the message first
 * are executed first, which causes subsequent messages to be read first.
 * 2018.07.22.
 * ================================================================================
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "dave_tools.h"
#include "thread_struct.h"
#include "thread_tools.h"
#include "thread_log.h"

// =====================================================================

sb
thread_is_seq_msg(ThreadMsg *pMsg)
{
	sb index;

	switch((ub)(pMsg->msg_body.msg_id))
	{
		case MSGID_TIMER:
		case MSGID_WAKEUP:
				index = 0;
			break;
		case SOCKET_BIND_RSP:
				index = (sb)(((SocketBindRsp *)(pMsg->msg_body.msg_body))->socket);
			break;
		case SOCKET_CONNECT_RSP:
				index = (sb)(((SocketConnectRsp *)(pMsg->msg_body.msg_body))->socket);
			break;
		case SOCKET_DISCONNECT_RSP:
				index = (sb)(((SocketDisconnectRsp *)(pMsg->msg_body.msg_body))->socket);
			break;
		case SOCKET_PLUGIN:
				index = (sb)(((SocketPlugIn *)(pMsg->msg_body.msg_body))->child_socket);
			break;
		case SOCKET_PLUGOUT:
				index = (sb)(((SocketPlugOut *)(pMsg->msg_body.msg_body))->socket);
			break;
		case SOCKET_READ:
				index = (sb)(((SocketRead *)(pMsg->msg_body.msg_body))->socket);
			break;
		case SOCKET_RAW_EVENT:
				index = (sb)(((SocketRawEvent *)(pMsg->msg_body.msg_body))->socket);
			break;
		default:
				index = -1;
			break;
	}

	return index;
}

#endif

