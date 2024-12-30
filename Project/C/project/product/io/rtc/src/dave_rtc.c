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
#include "dave_rtc.h"
#include "rtc_param.h"
#include "rtc_server.h"
#include "rtc_token.h"
#include "rtc_log.h"

static ThreadId _rtc_thread = INVALID_THREAD_ID;

static void
_rtc_init(MSGBODY *pMsg)
{
	rtc_token_init();
	rtc_server_init();
}

static void
_rtc_main(MSGBODY *pMsg)
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
		case MSGID_CFG_UPDATE:
			break;
		default:
				RTCLOG("unprocess msg-id:%s", msgstr(pMsg->msg_id));
			break;
	}
}

static void
_rtc_exit(MSGBODY *pMsg)
{
	rtc_server_exit();
	rtc_token_exit();
}

// =====================================================================

void
dave_rtc_init(void)
{
	ub thread_number = dave_os_cpu_process_number();

	_rtc_thread = base_thread_creat(
		RTC_THREAD_NAME, thread_number, THREAD_THREAD_FLAG|THREAD_dCOROUTINE_FLAG,
		_rtc_init, _rtc_main, _rtc_exit);

	if(_rtc_thread == INVALID_THREAD_ID)
		base_restart(RTC_THREAD_NAME);
}

void
dave_rtc_exit(void)
{
	if(_rtc_thread != INVALID_THREAD_ID)
		base_thread_del(_rtc_thread);
	_rtc_thread = INVALID_THREAD_ID;
}

