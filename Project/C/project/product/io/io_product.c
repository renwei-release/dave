/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_PRODUCT_IO__
#include "dave_base.h"
#include "dave_os.h"
#include "dave_tools.h"
#include "dave_http.h"
#include "dave_uip.h"
#include "dave_verno.h"
#include "dave_echo.h"
#include "io_test.h"

static ThreadId _io_thread = INVALID_THREAD_ID;

static void
_io_thread_init(MSGBODY *msg)
{
	dave_http_init();
	dave_uip_init();
}

static void
_io_thread_main(MSGBODY *msg)
{
	switch((sb)(msg->msg_id))
	{
		case MSGID_DEBUG_REQ:
				io_debug(msg->msg_src, (DebugReq *)(msg->msg_body));
			break;
		case MSGID_ECHO:
				dave_echo(msg->msg_src, msg->msg_dst, (MsgIdEcho *)(msg->msg_body));
			break;
		default:
			break;
	}
}

static void
_io_thread_exit(MSGBODY *msg)
{
	dave_uip_exit();
	dave_http_exit();
}

// =====================================================================

void
dave_product_init(void)
{
	ub thread_number = dave_os_cpu_process_number();

	_io_thread = base_thread_creat(t_gp_product_name(), thread_number, THREAD_THREAD_FLAG, _io_thread_init, _io_thread_main, _io_thread_exit);
	if(_io_thread == INVALID_THREAD_ID)
		base_restart(t_gp_product_name());
}

void
dave_product_exit(void)
{
	if(_io_thread != INVALID_THREAD_ID)
		base_thread_del(_io_thread);
	_io_thread = INVALID_THREAD_ID;
}

#endif

