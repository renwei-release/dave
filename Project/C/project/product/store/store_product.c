/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_PRODUCT_STORE__
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_3rdparty.h"
#include "dave_verno.h"
#include "dave_base.h"
#include "dave_echo.h"
#include "store_msg.h"
#include "store_mysql.h"

/*
 * Some databases cannot accept more connections, 
 * and the current design is to have as many connections as there are threads, 
 * so the number of threads needs to be limited.
 */
#define STORE_THREAD_MAX (16)

extern void store_debug(ThreadId src, DebugReq *pReq);

static ThreadId _store_thread = INVALID_THREAD_ID;

static ub
_store_thread_number(void)
{
	ub thread_number;

	thread_number = dave_os_cpu_process_number();
	if(thread_number > STORE_THREAD_MAX)
	{
		thread_number = STORE_THREAD_MAX;
	}

	return thread_number;
}

static void
_store_init(MSGBODY *msg)
{
	store_mysql_init(_store_thread_number());
}

static void
_store_main(MSGBODY *msg)
{
	switch(msg->msg_id)
	{
		case MSGID_DEBUG_REQ:
				store_debug(msg->msg_src, (DebugReq *)(msg->msg_body));
			break;
		case MSGID_ECHO_REQ:
		case MSGID_ECHO_RSP:
				dave_echo(msg->msg_src, msg->msg_dst, msg->msg_id, msg->msg_body);
			break;
		case STORE_MYSQL_REQ:
				store_mysql_sql(msg->msg_src, msg->thread_wakeup_index, (StoreMysqlReq *)(msg->msg_body));
			break;
		default:
			break;
	}
}

static void
_store_exit(MSGBODY *msg)
{
	store_mysql_exit();
}

// =====================================================================

void
dave_product_init(void)
{
	ub thread_number = _store_thread_number();

	_store_thread = base_thread_creat(STORE_THREAD_NAME, thread_number, THREAD_THREAD_FLAG, _store_init, _store_main, _store_exit);
	if(_store_thread == INVALID_THREAD_ID)
		base_restart(STORE_THREAD_NAME);
}

void
dave_product_exit(void)
{
	if(_store_thread != INVALID_THREAD_ID)
		base_thread_del(_store_thread);
	_store_thread = INVALID_THREAD_ID;
}

#endif

