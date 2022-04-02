/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "product_macro.h"
#ifdef __DAVE_PRODUCT_IO__
#include "dave_base.h"
#include "dave_os.h"
#include "dave_tools.h"

#define IOTESTLOG(a, ...) { DAVELOG("[BASE]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); }

static void
__concurrency_echo(s8 *dst, MsgIdEcho *pNewEcho, dave_bool concurrency_flag)
{
	MsgIdEcho *pConcurrencyEcho = thread_msg(pConcurrencyEcho);

	*pConcurrencyEcho = *pNewEcho;

	pConcurrencyEcho->concurrency_flag = concurrency_flag;

	remote_msg(dst, MSGID_ECHO, pConcurrencyEcho);
}

static void
_concurrency_echo(s8 *dst, MsgIdEcho *pNewEcho)
{
	sb concurrency_number = (sb)(dave_rand() % 4096);
	dave_bool send_normal_echo = dave_false;

	while((concurrency_number --) > 0)
	{
		if((concurrency_number == 512) && (send_normal_echo == dave_false))
		{
			send_normal_echo = dave_true;

			__concurrency_echo(dst, pNewEcho, dave_false);
		}
		else
		{
			__concurrency_echo(dst, pNewEcho, dave_true);
		}
	}

	if(send_normal_echo == dave_false)
	{
		remote_msg(dst, MSGID_ECHO, pNewEcho);
	}
	else
	{
		thread_msg_release(pNewEcho);
	}
}

// =====================================================================

void
io_echo(ThreadId src, MsgIdEcho *pEcho)
{
	ub time_consuming = dave_os_time_us() - pEcho->echo_time;
	ub echo_show_interval = (pEcho->echo_multiple == dave_true) ? 150 : 10000;
	MsgIdEcho *pNewEcho = thread_msg(pNewEcho);

	pNewEcho->echo_counter = pEcho->echo_counter + 1;
	pNewEcho->echo_time = dave_os_time_us();
	pNewEcho->echo_multiple = pEcho->echo_multiple;
	pNewEcho->concurrency_flag = dave_false;
	dave_snprintf(pNewEcho->msg, sizeof(pNewEcho->msg),
		"counter:%ld consuming:%ldus",
		pNewEcho->echo_counter, time_consuming);

	if(pEcho->echo_counter == 0)
	{
		broadcast_remote(MSGID_ECHO, pNewEcho);
	}
	else
	{
		if((pEcho->echo_counter % 2) == 0)
		{
			if(pEcho->concurrency_flag == dave_false)
			{
				if((pEcho->echo_counter % echo_show_interval) == 0)
				{
					IOTESTLOG("from:%s echo:(%s) %ld/%ld", thread_name(src), pEcho->msg, pEcho->echo_counter, echo_show_interval);
				}

				remote_msg(thread_name(src), MSGID_ECHO, pNewEcho);
			}
			else
			{
				thread_msg_release(pNewEcho);
			}
		}
		else
		{
			if(pNewEcho->echo_multiple == dave_false)
			{
				remote_msg(thread_name(src), MSGID_ECHO, pNewEcho);
			}
			else
			{
				_concurrency_echo(thread_name(src), pNewEcho);
			}
		}
	}
}

void
io_debug(ThreadId src, DebugReq *pReq)
{
	DebugRsp *pRsp = thread_msg(pRsp);

	dave_strcpy(pRsp->msg, pReq->msg, sizeof(pRsp->msg));
	pRsp->ptr = pReq->ptr;

	write_msg(src, MSGID_DEBUG_RSP, pRsp);
}

#endif

