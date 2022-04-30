/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_http.h"
#include "dave_os.h"
#include "dave_3rdparty.h"
#include "http_recv.h"
#include "http_distributor.h"
#include "http_test.h"
#include "http_log.h"

#define HTTP_THREAD_MAX 16

static ThreadId _http_thread = INVALID_THREAD_ID;

static ub
_http_thread_number(void)
{
	ub thread_number = dave_os_cpu_process_number();

	if(thread_number > HTTP_THREAD_MAX)
	{
		thread_number = HTTP_THREAD_MAX;
	}

	return thread_number;
}

static void
_http_restart(RESTARTREQMSG *pRestart)
{
	if(pRestart->times == 4)
	{
		http_recv_exit();
		http_distributor_exit();
	}
}

static void
_http_init(MSGBODY *msg)
{
	dave_nginx_init();
	http_recv_init();
	http_distributor_init();
}

static void
_http_main(MSGBODY *msg)
{
	switch((ub)msg->msg_id)
	{
		case MSGID_RESTART_REQ:
				_http_restart((RESTARTREQMSG *)(msg->msg_body));
			break;
		case MSGID_DEBUG_REQ:
				http_test(msg->msg_src, (DebugReq *)(msg->msg_body));
			break;
		case HTTPMSG_LISTEN_REQ:
				http_recv_listen(msg->msg_src, (HTTPListenReq *)(msg->msg_body));
			break;
		case HTTPMSG_CLOSE_REQ:
				http_recv_close(msg->msg_src, (HTTPCloseReq *)(msg->msg_body));
			break;
		default:
			break;
	}
}

static void
_http_exit(MSGBODY *msg)
{
	dave_nginx_exit();
}

// =====================================================================

void
dave_http_init(void)
{
	ub thread_number = _http_thread_number();

	_http_thread = base_thread_creat(HTTP_THREAD_NAME, thread_number, THREAD_THREAD_FLAG, _http_init, _http_main, _http_exit);
	if(_http_thread == INVALID_THREAD_ID)
		dave_restart(HTTP_THREAD_NAME);
}

void
dave_http_exit(void)
{
	if(_http_thread != INVALID_THREAD_ID)
		base_thread_del(_http_thread);
	_http_thread = INVALID_THREAD_ID;
}

