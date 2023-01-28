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
#include "store_msg.h"
#include "store_mysql.h"

static ThreadId _store_thread = INVALID_THREAD_ID;

static void
_store_init(MSGBODY *msg)
{
	store_mysql_init();
}

static void
_store_main(MSGBODY *msg)
{
	switch(msg->msg_id)
	{
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
	ub thread_number = dave_os_cpu_process_number();

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

