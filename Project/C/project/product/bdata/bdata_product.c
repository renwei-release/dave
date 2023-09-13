/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_PRODUCT_BDATA__
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_3rdparty.h"
#include "dave_verno.h"
#include "dave_base.h"
#include "dave_echo.h"
#include "bdata_msg.h"
#include "recipient_log.h"
#include "recorder_file.h"

static ThreadId _bdata_thread = INVALID_THREAD_ID;

static void
_bdata_init(MSGBODY *msg)
{
	recorder_file_init();
}

static void
_bdata_main(MSGBODY *msg)
{
	switch(msg->msg_id)
	{
		case MSGID_ECHO_REQ:
		case MSGID_ECHO_RSP:
				dave_echo(msg->msg_src, msg->msg_dst, msg->msg_id, msg->msg_body);
			break;
		case BDATA_LOG_REQ:
				recipient_log(msg->msg_src, (BDataLogReq *)(msg->msg_body));
			break;
		default:
			break;
	}
}

static void
_bdata_exit(MSGBODY *msg)
{
	recorder_file_exit();
}

// =====================================================================

void
dave_product_init(void)
{
	ub thread_number = dave_os_cpu_process_number();

	_bdata_thread = base_thread_creat(BDATA_THREAD_NAME, thread_number, THREAD_THREAD_FLAG, _bdata_init, _bdata_main, _bdata_exit);
	if(_bdata_thread == INVALID_THREAD_ID)
		base_restart(BDATA_THREAD_NAME);
}

void
dave_product_exit(void)
{
	if(_bdata_thread != INVALID_THREAD_ID)
		base_thread_del(_bdata_thread);
	_bdata_thread = INVALID_THREAD_ID;
}

#endif

