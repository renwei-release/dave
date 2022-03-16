/*
 * ================================================================================
 * (c) Copyright 2017 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2017.08.02.
 * ================================================================================
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "dave_tools.h"
#include "thread_struct.h"
#include "thread_quit.h"
#include "thread_guardian.h"
#include "thread_log.h"

static ub _statistics_thread_run_time = 0;	// 1000 * 100;
static ub _statistics_thread_msg_time = 0;	// 1000 * 100;
static ub _statistics_msg_id = MSGID_RESERVED;

static void
_thread_statistics_msg_id(MSGBODY *msg)
{
	if(_statistics_msg_id != msg->msg_id)
	{
		return;
	}

	THREADLOG("msg run %s->%s %d wakeup:%d time:%ld serial:%ld",
		thread_name(msg->msg_src),
		thread_name(msg->msg_dst),
		msg->msg_id,
		msg->thread_wakeup_index,
		msg->msg_build_time,
		msg->msg_build_serial);
}

// =====================================================================

ub
thread_statistics_run_start(MSGBODY *pMsg)
{
	if(_statistics_thread_run_time > 0)
	{
		return dave_os_time_us();
	}
	else
	{
		return 0;
	}
}

void
thread_statistics_run_end(ub run_time, ThreadStruct *pThread, MSGBODY *msg)
{
	if((_statistics_thread_run_time > 0) && (run_time > 0))
	{
		run_time = dave_os_time_us() - run_time;
		if(run_time > _statistics_thread_run_time)
		{
			THREADLOG("msg run time:%dus name:%s %s->%s %d wakeup:%d",
				run_time,
				pThread->thread_name,
				thread_name(msg->msg_src),
				thread_name(msg->msg_dst),
				msg->msg_id,
				msg->thread_wakeup_index);
		}
	}

	_thread_statistics_msg_id(msg);
}

void
thread_statistics_write_msg_time(ThreadMsg *pMsg)
{
	if(pMsg != NULL)
	{
		if(_statistics_thread_msg_time > 0)
		{
			pMsg->msg_body.msg_build_time = dave_os_time_us();
		}
	}
}

void
thread_statistics_read_msg_time(ThreadMsg *pMsg)
{
	ub msg_wakeup_time;

	if((pMsg != NULL) && (_statistics_thread_msg_time > 0) && (pMsg->msg_body.msg_build_time > 0))
	{
		msg_wakeup_time = dave_os_time_us() - pMsg->msg_body.msg_build_time;

		if(msg_wakeup_time > _statistics_thread_msg_time)
		{
			THREADLOG("msg wakeup time:%dus %s->%s %d",
				msg_wakeup_time,
				thread_name(pMsg->msg_body.msg_src),
				thread_name(pMsg->msg_body.msg_dst),
				pMsg->msg_body.msg_id);
		}
	}
}

void
thread_statistics_setup_run_time(ub run_time)
{
	_statistics_thread_run_time = run_time;
}

void
thread_statistics_setup_msg_time(ub msg_time)
{
	_statistics_thread_msg_time = msg_time;
}

void
thread_statistics_setup_msg_id(ub msg_id)
{
	_statistics_msg_id = msg_id;
}

void
thread_statistics_setup_all_time(ub all_time)
{
	_statistics_thread_run_time = _statistics_thread_msg_time = all_time;
}

void
thread_statistics_load_all_time(ub *run_time, ub *msg_time)
{
	*run_time = _statistics_thread_run_time;
	*msg_time = _statistics_thread_msg_time;
}

#endif

