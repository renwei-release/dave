/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_PRODUCT_TEST__
#include "dave_base.h"
#include "dave_os.h"
#include "dave_tools.h"
#include "dave_verno.h"
#include "service_test.h"
#include "test_log.h"

static ThreadId _test_thread = INVALID_THREAD_ID;
static TIMERID _test_timer = INVALID_TIMER_ID;

static void
_test_service_test_exit(TIMERID timer_id, ub thread_index)
{
	base_restart("test");
}

static void
_test_stop_timer(void)
{
	if(_test_timer != INVALID_TIMER_ID)
	{
		base_timer_die(_test_timer);
		_test_timer = INVALID_TIMER_ID;
	}
}

static void
_test_start_timer(ub alarm_s)
{
	_test_stop_timer();

	_test_timer = base_timer_creat((char *)"test", _test_service_test_exit, alarm_s*1000);
}

static void
_test_thread_id_ready(ThreadRemoteIDReadyMsg *pReady)
{
	_test_stop_timer();

	service_test(pReady->globally_identifier, pReady->remote_thread_name, pReady->remote_thread_id);

	_test_start_timer(3);
}

static void
_test_thread_init(MSGBODY *msg)
{
	_test_start_timer(360);
}

static void
_test_thread_main(MSGBODY *msg)
{
	switch((sb)(msg->msg_id))
	{
		case MSGID_REMOTE_THREAD_ID_READY:
				_test_thread_id_ready((ThreadRemoteIDReadyMsg *)(msg->msg_body));
			break;
		default:
			break;
	}
}

static void
_test_thread_exit(MSGBODY *msg)
{
	_test_stop_timer();
}

// =====================================================================

extern "C" void
dave_product_init(void)
{
	ub thread_number = dave_os_cpu_process_number();

	_test_thread = base_thread_creat(t_gp_product_name(), thread_number, THREAD_THREAD_FLAG|THREAD_COROUTINE_FLAG, _test_thread_init, _test_thread_main, _test_thread_exit);
	if(_test_thread == INVALID_THREAD_ID)
		base_restart(t_gp_product_name());
}

extern "C" void
dave_product_exit(void)
{
	if(_test_thread != INVALID_THREAD_ID)
		base_thread_del(_test_thread);
	_test_thread = INVALID_THREAD_ID;
}

#endif

