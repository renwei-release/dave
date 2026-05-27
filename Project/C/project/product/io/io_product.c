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
#include "dave_email.h"
#include "dave_rtc.h"
#include "dave_sip.h"
#include "dave_verno.h"
#include "dave_echo.h"
#include "io_test.h"

static ThreadId _io_thread = INVALID_THREAD_ID;

static void
_io_thread_init(MSGBODY *msg)
{
	if(cfg_get_bool("IOHttpEnable", dave_true) == dave_true)
		dave_http_init();
	if(cfg_get_bool("IOUIPEnable", dave_true) == dave_true)
		dave_uip_init();
	if(cfg_get_bool("IOEmailEnable", dave_true) == dave_true)
		dave_email_init();
	if(cfg_get_bool("IORTCEnable", dave_true) == dave_true)
		dave_rtc_init();
	if(cfg_get_bool("IOSIPEEnable", dave_true) == dave_true)
		dave_sip_init();
}

static void
_io_thread_main(MSGBODY *msg)
{
	switch((sb)(msg->msg_id))
	{
		case MSGID_DEBUG_REQ:
				io_debug(msg->msg_src, (DebugReq *)(msg->msg_body));
			break;
		case MSGID_ECHO_REQ:
		case MSGID_ECHO_RSP:
				dave_echo(msg->msg_src, msg->msg_dst, msg->msg_id, msg->msg_body);
			break;
		default:
			break;
	}
}

static void
_io_thread_exit(MSGBODY *msg)
{
	dave_sip_exit();
	dave_rtc_exit();
	dave_email_exit();
	dave_uip_exit();
	dave_http_exit();
}

// =====================================================================

void
dave_product_init(void)
{
	_io_thread = base_thread_creat(t_gp_product_name(), 1, THREAD_THREAD_FLAG, _io_thread_init, _io_thread_main, _io_thread_exit);
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

