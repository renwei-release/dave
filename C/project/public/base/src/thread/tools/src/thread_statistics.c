/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "thread_struct.h"
#include "thread_quit.h"
#include "thread_guardian.h"
#include "thread_log.h"

static ub _statistics_thread_msg_id = 0;
static ub _statistics_thread_run_time = 0;
static ub _statistics_thread_wakeup_time = 0;

// =====================================================================

ub
thread_statistics_start_msg(MSGBODY *pMsg)
{
	if((_statistics_thread_run_time > 0) && (_statistics_thread_msg_id == pMsg->msg_id))
	{
		return dave_os_time_us();
	}
	else
	{
		return 0;
	}
}

void
thread_statistics_end_msg(ub run_time, ThreadStruct *pThread, MSGBODY *msg)
{
	if((_statistics_thread_run_time > 0) && (run_time > 0))
	{
		run_time = dave_os_time_us() - run_time;
		if(run_time > _statistics_thread_run_time)
		{
			THREADLOG("msg run time:%dus name:%s %s->%s:%s wakeup:%d",
				run_time,
				pThread->thread_name,
				thread_name(msg->msg_src),
				thread_name(msg->msg_dst),
				msgstr(msg->msg_id),
				msg->thread_wakeup_index);
		}
	}
}

void
thread_statistics_write_msg(ThreadMsg *pMsg)
{
	if(pMsg != NULL)
	{
		if((_statistics_thread_wakeup_time > 0) && (_statistics_thread_msg_id == pMsg->msg_body.msg_id))
		{
			pMsg->msg_body.msg_build_time = dave_os_time_us();
		}
	}
}

void
thread_statistics_read_msg(ThreadMsg *pMsg)
{
	ub msg_wakeup_time;

	if((pMsg != NULL) && (_statistics_thread_wakeup_time > 0) && (pMsg->msg_body.msg_build_time > 0))
	{
		msg_wakeup_time = dave_os_time_us() - pMsg->msg_body.msg_build_time;

		if(msg_wakeup_time > _statistics_thread_wakeup_time)
		{
			THREADLOG("msg wakeup time:%dus %s->%s:%s",
				msg_wakeup_time,
				thread_name(pMsg->msg_body.msg_src),
				thread_name(pMsg->msg_body.msg_dst),
				msgstr(pMsg->msg_body.msg_id));
		}
	}
}

void
thread_statistics_setup_all(ub msg_id, ub run_time, ub wakeup_time)
{
	_statistics_thread_msg_id = msg_id;
	_statistics_thread_run_time = run_time;
	_statistics_thread_wakeup_time = wakeup_time;
}

void
thread_statistics_load_all(ub *msg_id, ub *run_time, ub *wakeup_time)
{
	*msg_id = _statistics_thread_msg_id;
	*run_time = _statistics_thread_run_time;
	*wakeup_time = _statistics_thread_wakeup_time;
}

#endif

