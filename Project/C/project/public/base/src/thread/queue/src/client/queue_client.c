/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_queue.h"
#if defined(QUEUE_STACK_CLIENT)
#include "dave_tools.h"
#include "dave_verno.h"
#include "dave_base.h"
#include "dave_os.h"
#include "base_rxtx.h"

static ThreadId _queue_client_thread;

static void
_queue_client_init(MSGBODY *msg)
{

}

static void
_queue_client_main(MSGBODY *msg)
{
	switch((ub)(msg->msg_id))
	{
		default:
			break;
	}
}

static void
_queue_client_exit(MSGBODY *msg)
{

}

// =====================================================================

void
queue_client_init(void)
{
	ub thread_number = 1;

	_queue_client_thread = base_thread_creat(QUEUE_CLIENT_THREAD_NAME, thread_number, THREAD_THREAD_FLAG|THREAD_CORE_FLAG, _queue_client_init, _queue_client_main, _queue_client_exit);
	if(_queue_client_thread == INVALID_THREAD_ID)
		base_restart(QUEUE_CLIENT_THREAD_NAME);
}

void
queue_client_exit(void)
{
	if(_queue_client_thread != INVALID_THREAD_ID)
		base_thread_del(_queue_client_thread);
	_queue_client_thread = INVALID_THREAD_ID;
}

#endif

