/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "thread_parameter.h"
#include "thread_mem.h"
#include "thread_struct.h"
#include "thread_thread.h"
#include "thread_tools.h"
#include "thread_quit.h"
#include "thread_statistics.h"
#include "thread_running.h"
#include "thread_call.h"
#include "thread_log.h"

static void
_thread_guardian_thread_option(ThreadStruct *pThread, ub thread_index, dave_bool initialization_flag)
{
	if(pThread->thread_flag & THREAD_THREAD_FLAG)
	{
		if(initialization_flag == dave_true)
		{
			thread_thread_creat((s8 *)(pThread->thread_name), thread_index, pThread->thread_id, pThread->level_number);
		}
		else
		{
			thread_thread_die(thread_index);
		}
	}
}

static void
_thread_guardian_run_user_function(MSGBODY *msg)
{
	RUNFUNCTIONMSG *pRun = (RUNFUNCTIONMSG *)(msg->msg_body);
	base_thread_fun user_initialization_function = (base_thread_fun)(pRun->thread_fun);
	ThreadStruct *pThread = thread_find_busy_thread(pRun->run_thread_id);
	WAKEUPMSG wakeup;

	THREADDEBUG("%s user_initialization_function:%lx",
		thread_name(self()), user_initialization_function);

	user_initialization_function(msg);

	pThread->has_initialization = dave_true;

	base_thread_id_msg(
		NULL, NULL, NULL, NULL,
		pThread->thread_id, pThread->thread_id,
		BaseMsgType_Unicast,
		MSGID_WAKEUP, sizeof(WAKEUPMSG), (u8 *)&wakeup,
		1,
		(s8 *)__func__, (ub)__LINE__);	
}

static void
_thread_guardian_run_user_initialization(ThreadStruct *pThread, RUNFUNCTIONMSG *run)
{
	RUNFUNCTIONMSG my_run;

	base_thread_msg_register(pThread->thread_id, MSGID_RUN_FUNCTION, _thread_guardian_run_user_function, NULL);
	
	my_run	= *run;
	
	base_thread_id_msg(
		NULL, NULL, NULL, NULL,
		pThread->thread_id, pThread->thread_id,
		BaseMsgType_pre_msg,
		MSGID_RUN_FUNCTION, sizeof(RUNFUNCTIONMSG), (u8 *)&my_run,
		1,
		(s8 *)__func__, (ub)__LINE__);
}

static void
_thread_guardian_run_guardian_initialization(ThreadStruct *pThread, RUNFUNCTIONMSG *run)
{
	MSGBODY msg;
	
	dave_memset(&msg, 0x00, sizeof(MSGBODY));
	
	msg.msg_src = get_self();
	msg.msg_dst = run->run_thread_id;
	msg.msg_id = MSGID_RUN_FUNCTION;

	thread_running(
		(ThreadStack **)(run->param),
		(base_thread_fun)(run->thread_fun),
		pThread,
		&msg, dave_true);

	pThread->has_initialization = dave_true;
}

static void
_thread_guardian_run_initialization(ThreadStruct *pThread, RUNFUNCTIONMSG *run)
{
	if(pThread->has_initialization == dave_true)
	{
		THREADABNOR("%s repeat the initialization function!", pThread->thread_name);
	}
	else
	{
		_thread_guardian_thread_option(pThread, pThread->thread_index, run->initialization_flag);

		if(dave_strcmp(pThread->thread_name, GUARDIAN_THREAD_NAME) == dave_true)
		{
			_thread_guardian_run_guardian_initialization(pThread, run);
		}
		else
		{
			_thread_guardian_run_user_initialization(pThread, run);
		}

		thread_local_ready_notify(pThread->thread_name);
	}
}

static void
_thread_guardian_run_end(ThreadStruct *pThread, RUNFUNCTIONMSG *run)
{
	MSGBODY msg;

	dave_memset(&msg, 0x00, sizeof(MSGBODY));

	msg.msg_src = get_self();
	msg.msg_dst = run->run_thread_id;
	msg.msg_id = MSGID_RUN_FUNCTION;

	if(pThread->has_initialization == dave_false)
	{
		THREADABNOR("%s repeat the exit function!", pThread->thread_name);
	}
	else
	{
		thread_running(
			(ThreadStack **)(run->param),
			(base_thread_fun)(run->thread_fun),
			pThread,
			&msg, dave_true);

		pThread->has_initialization = dave_false;
	
		_thread_guardian_thread_option(pThread, pThread->thread_index, run->initialization_flag);

		thread_local_remove_notify(pThread->thread_name);
	}
}

static void
_thread_guardian_run_function(RUNFUNCTIONMSG *run)
{
	ThreadStruct *pThread;

	pThread = thread_find_busy_thread(run->run_thread_id);
	if(pThread == NULL)
	{
		THREADDEBUG("invalid thread_fun:%lx last_fun:%lx run_thread_id:%lx/%s initialization_flag:%d",
			run->thread_fun, run->last_fun,
			run->run_thread_id, thread_name(run->run_thread_id),
			run->initialization_flag);
		return;
	}

	if(run->initialization_flag == dave_true)
	{
		_thread_guardian_run_initialization(pThread, run);
	}
	else
	{
		_thread_guardian_run_end(pThread, run);
	}

	if(run->last_fun != NULL)
	{
		((base_thread_fun)(run->last_fun))((MSGBODY *)pThread);
	}
}

// =====================================================================

void
thread_guardian_run_function(RUNFUNCTIONMSG *run)
{
	_thread_guardian_run_function(run);
}

#endif

