/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "product_macro.h"
#ifdef __DAVE_PRODUCT_BASE__
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_verno.h"
#include "base_test.h"

static ThreadId _base_thread = INVALID_THREAD_ID;

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
	ub thread_number = dave_os_cpu_process_number();

	_base_thread = dave_thread_creat(dave_verno_my_product(), thread_number, THREAD_THREAD_FLAG, _base_thread_init, _base_thread_main, _base_thread_exit);
	if(_base_thread == INVALID_THREAD_ID)
		dave_restart(dave_verno_my_product());
}

void
dave_product_exit(void)
{
	if(_base_thread != INVALID_THREAD_ID)
		dave_thread_del(_base_thread);
	_base_thread = INVALID_THREAD_ID;
}

#endif

