/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_database.h"
#include "dave_os.h"
#include "uip_server.h"
#include "uip_client.h"
#include "uip_channel.h"
#include "uip_debug.h"

static ThreadId _uip_thread = INVALID_THREAD_ID;
static ub _uip_thread_number;

static void 
_uip_thread_remote_ready(ThreadRemoteReadyMsg *pMsg)
{
	if(dave_strcmp(pMsg->remote_thread_name, DATABASE_THREAD_NAME) == dave_true)
	{
		uip_channel_reset();
	}
}

static void
_uip_restart(RESTARTREQMSG *pRestart)
{
	uip_server_restart(pRestart);
}

static void
_uip_init(MSGBODY *pMsg)
{
	uip_channel_init();

	uip_server_init(pMsg);

	uip_client_init(pMsg);
}

static void
_uip_main(MSGBODY *pMsg)
{
	switch((sb)(pMsg->msg_id))
	{
		case MSGID_DEBUG_REQ:
				uip_debug(pMsg->msg_src, (DebugReq *)(pMsg->msg_body));
			break;
		case MSGID_RESTART_REQ:
				_uip_restart((RESTARTREQMSG *)(pMsg->msg_body));
			break;
		case MSGID_REMOTE_THREAD_READY:
				_uip_thread_remote_ready((ThreadRemoteReadyMsg *)(pMsg->msg_body));
			break;
		case UIP_DATA_SEND_REQ:
				uip_client_main(pMsg);
			break;
		default:
				uip_server_main(pMsg);
			break;
	}
}

static void
_uip_exit(MSGBODY *pMsg)
{
	uip_client_exit(pMsg);

	uip_server_exit(pMsg);

	uip_channel_exit();
}

// =====================================================================

void
dave_uip_init(void)
{
	_uip_thread_number = dave_os_cpu_process_number();

	_uip_thread = base_thread_creat(UIP_THREAD_NAME, _uip_thread_number, THREAD_THREAD_FLAG, _uip_init, _uip_main, _uip_exit);
	if(_uip_thread == INVALID_THREAD_ID)
		dave_restart(UIP_THREAD_NAME);
}

void
dave_uip_exit(void)
{
	if(_uip_thread != INVALID_THREAD_ID)
		base_thread_del(_uip_thread);
	_uip_thread = INVALID_THREAD_ID;
}

