/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_echo.h"
#include "dave_uac.h"
#include "dave_osip.h"
#include "uac_server.h"
#include "uac_client.h"
#include "uac_call.h"
#include "uac_class.h"
#include "uac_rtp.h"
#include "uac_rtp_msg.h"
#include "uac_main.h"
#include "uac_dos.h"
#include "uac_state.h"
#include "uac_automatic.h"
#include "uac_global_lock.h"
#include "uac_log.h"

static ThreadId _uac_thread = INVALID_THREAD_ID;

static void
_uac_init(MSGBODY *pMsg)
{
	uac_global_lock_init();
	dave_osip_init();
	uac_server_init();
	uac_client_init();
	uac_state_init();
	uac_class_init();
	uac_rtp_init();
	uac_rtp_msg_init();
	uac_call_init();
	uac_dos_init();
	uac_automatic_init();
	uac_main_init();
}

static void
_uac_read(SocketRead *pRead)
{
	if(uac_rtp_recv(pRead) == dave_false)
	{
		uac_class_recv(pRead);
	}

	dave_mfree(pRead->data);
}

static void
_uac_main(MSGBODY *pMsg)
{
	switch((sb)(pMsg->msg_id))
	{
		case MSGID_ECHO_REQ:
		case MSGID_ECHO_RSP:
				dave_echo(pMsg->msg_src, pMsg->msg_dst, pMsg->msg_id, pMsg->msg_body);
			break;
		case MSGID_RESTART_REQ:
		case MSGID_LOCAL_THREAD_READY:
		case MSGID_LOCAL_THREAD_REMOVE:
		case MSGID_REMOTE_THREAD_READY:
		case MSGID_REMOTE_THREAD_REMOVE:
		case MSGID_REMOTE_THREAD_ID_READY:
		case MSGID_REMOTE_THREAD_ID_REMOVE:
		case SOCKET_DISCONNECT_RSP:
		case SOCKET_PLUGIN:
		case SOCKET_PLUGOUT:
		case MSGID_CFG_UPDATE:
			break;
		case SOCKET_READ:
				_uac_read((SocketRead *)(pMsg->msg_body));
			break;
		default:
				UACLOG("unprocess msg-id:%s", msgstr(pMsg->msg_id));
			break;
	}
}

static void
_uac_exit(MSGBODY *pMsg)
{
	uac_main_exit();
	uac_automatic_exit();
	uac_dos_exit();
	uac_call_exit();
	uac_rtp_msg_exit();
	uac_rtp_exit();
	uac_class_exit();
	uac_state_exit();
	uac_server_exit();
	uac_client_exit();
	dave_osip_exit();
	uac_global_lock_exit();
}

// =====================================================================

void
dave_uac_init(void)
{
	ub thread_number = dave_os_cpu_process_number();

	_uac_thread = base_thread_creat(
		UAC_THREAD_NAME, thread_number, THREAD_THREAD_FLAG,
		_uac_init, _uac_main, _uac_exit);

	if(_uac_thread == INVALID_THREAD_ID)
		base_restart(UAC_THREAD_NAME);
}

void
dave_uac_exit(void)
{
	if(_uac_thread != INVALID_THREAD_ID)
		base_thread_del(_uac_thread);
	_uac_thread = INVALID_THREAD_ID;
}
