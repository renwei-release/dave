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
#include "thread_parameter.h"
#include "thread_mem.h"
#include "thread_struct.h"
#include "thread_thread.h"
#include "thread_tools.h"
#include "thread_quit.h"
#include "thread_statistics.h"
#include "thread_msg_buffer.h"
#include "thread_call.h"
#include "thread_busy_idle.h"
#include "thread_wait_msg_show.h"
#include "thread_sync.h"
#include "thread_remote_id_table.h"
#include "thread_gid_table.h"
#include "thread_wakeup_the_sleep.h"
#include "thread_running.h"
#include "thread_log.h"

static void _thread_guardian_exit(MSGBODY *msg);

static ThreadId _guardian_thread = INVALID_THREAD_ID;
static ThreadStruct *_thread;
static DateStruct _system_work_start_date;

static void
_thread_guardian_test(MSGBODY *msg)
{
	ub msg_index;
	ThreadId task2_id;

	if(msg->msg_len != 33)
	{
		THREADDEBUG("GUARDIAN GET MSG FAIL msg_len=%d", msg->msg_len);
	}
	for(msg_index=0; msg_index<msg->msg_len; msg_index++)
	{
		if(((u8 *)(msg->msg_body))[msg_index] != 0xaa)
			break;
	}
	if(msg_index < msg->msg_len)
	{
		t_stdio_print_hex("GUARDIAN GET DATA FAIL", (u8 *)(msg->msg_body), (s32)(msg->msg_len));
	}

	task2_id = get_thread_id("test2");
	if(snd_msg(task2_id, MSGID_TEST, msg->msg_len, msg->msg_body) == dave_false)
	{
		THREADDEBUG("GUARDIAN SND MSG FAIL!");
	}
}

static void
_thread_guardian_setup_statistics(s8 *msg, s8 *rsp_msg, ub rsp_len)
{
	s8 msg_str[64], run_str[64], wakeup_str[64];
	ub msg_id, run_time, wakeup_time;

	msg = dave_strfind(msg, ' ', msg_str, sizeof(msg_str));
	msg = dave_strfind(msg, ' ', run_str, sizeof(run_str));
	msg = dave_strfind(msg, ' ', wakeup_str, sizeof(wakeup_str));

	msg_id = stringdigital(msg_str);
	run_time = stringdigital(run_str);
	wakeup_time = stringdigital(wakeup_str);

	dave_snprintf(rsp_msg, rsp_len, "setup statistics msg-id:%s run-time:%d wakup-time:%d",
		msgstr(msg_id), run_time, wakeup_time);

	thread_statistics_setup_all(msg_id, run_time, wakeup_time);
}

static void
_thread_guardian_load_statistics(s8 *msg, s8 *rsp_msg, ub rsp_len)
{
	ub msg_id, run_time, wakeup_time;

	thread_statistics_load_all(&msg_id, &run_time, &wakeup_time);

	dave_snprintf(rsp_msg, rsp_len, "load statistics msg-id:%s run-time:%d wakup-time:%d",
		msgstr(msg_id), run_time, wakeup_time);
}

static void
_thread_guardian_test_memory(void)
{
	void *ptr;

	ptr = dave_malloc(1024 * 1024 * 2);

	dave_free(ptr);

	dave_free(ptr);

	dave_free(ptr + 2);
}

static void
_thread_guardian_debug(ThreadId src, DebugReq *pReq)
{
	DebugRsp *pRsp = thread_reset_msg(pRsp);
	ub msg_len;

	if(pReq->msg[0] == 's')
	{
		_thread_guardian_setup_statistics(&(pReq->msg[1]), pRsp->msg, sizeof(pRsp->msg));
	}
	else if(pReq->msg[0] == 'l')
	{
		_thread_guardian_load_statistics(&(pReq->msg[1]), pRsp->msg, sizeof(pRsp->msg));
	}
	else if(pReq->msg[0] == 'b')
	{
		_thread_guardian_test_memory();
	}
	else if(pReq->msg[0] == 'w')
	{
		msg_len = thread_wait_msg_show(_thread, &pReq->msg[1], pRsp->msg, sizeof(pRsp->msg));
		if(msg_len == 0)
		{
			dave_snprintf(pRsp->msg, msg_len,
				"the thread_name:%s has empty message (debug GUARDIAN w[thread_name])!",
				&pReq->msg[1]);
		}
	}
	else if(pReq->msg[0] == 'd')
	{
		thread_show_all_info(_thread, &_system_work_start_date, pRsp->msg, sizeof(pRsp->msg), dave_false);
	}
	else
	{
		thread_show_all_info(_thread, &_system_work_start_date, pRsp->msg, sizeof(pRsp->msg), dave_true);
	}

	id_msg(src, MSGID_DEBUG_RSP, pRsp);
}

static void
_thread_guardian_restart(RESTARTREQMSG *pReq)
{
	if(pReq->times == 4)
	{
		_thread_guardian_exit(NULL);
	}
}

static void
_thread_guardian_system_check(void)
{
	thread_call_sync_check();

	thread_busy_idle_check();

	thread_wakeup_the_sleep(_thread);
}

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
_thread_guardian_run_function(RUNFUNCTIONMSG *run)
{
	ub thread_index;
	ThreadStruct *pThread;
	MSGBODY msg;

	thread_index = thread_find_busy_index(run->thread_dst);
	if(thread_index >= THREAD_MAX)
	{
		THREADDEBUG("invalid thread_index:%d thread_fun:%lx last_fun:%lx thread_dst:%lx/%s initialization_flag:%d",
			thread_index,
			run->thread_fun, run->last_fun,
			run->thread_dst, thread_name(run->thread_dst),
			run->initialization_flag);
		return;
	}

	pThread = &_thread[thread_index];

	msg.msg_src = get_self();
	msg.msg_dst = run->thread_dst;
	msg.msg_id = MSGID_RUN_FUNCTION;
	msg.msg_len = 0;
	msg.msg_body = NULL;
	msg.msg_chain = NULL;

	if(run->initialization_flag == dave_true)
	{
		if((pThread->has_initialization == dave_true)
			&& (dave_strcmp(pThread->thread_name, GUARDIAN_THREAD_NAME) == dave_false))
		{
			THREADABNOR("%s repeat the initialization function!", pThread->thread_name);
		}
		else
		{
			_thread_guardian_thread_option(pThread, thread_index, run->initialization_flag);

			thread_running(
				(ThreadStack **)(run->param),
				(base_thread_fun)(run->thread_fun),
				pThread,
				&msg, dave_true);

			pThread->has_initialization = dave_true;
		}
	}
	else
	{
		if(pThread->has_initialization == dave_false)
		{
			THREADABNOR("%s repeat the exit function!", pThread->thread_name);
		}
		else
		{
			pThread->has_initialization = dave_false;

			thread_running(
				(ThreadStack **)(run->param),
				(base_thread_fun)(run->thread_fun),
				pThread,
				&msg, dave_true);

			_thread_guardian_thread_option(pThread, thread_index, run->initialization_flag);
		}
	}

	if(run->last_fun != NULL)
	{
		((base_thread_fun)(run->last_fun))((MSGBODY *)pThread);
	}
}

static void
_thread_guardian_trace_switch(TraceSwitchMsg *pSwitch)
{
	ub thread_index;

	thread_index = thread_find_busy_index(pSwitch->thread_id);
	if(thread_index >= THREAD_MAX)
	{
		THREADDEBUG("");
	}
	else
	{
		_thread[thread_index].trace_on = pSwitch->trace_on;
	}
}

static void
_thread_guardian_remote_thread_ready(ThreadRemoteReadyMsg *pReady)
{
	THREADDEBUG("%s", pReady->remote_thread_name);

	thread_msg_buffer_pop(pReady->remote_thread_name);
}

static void
_thread_guardian_remote_thread_id_ready(ThreadRemoteIDReadyMsg *pReady)
{
	THREADDEBUG("%s", pReady->remote_thread_name);

	thread_msg_buffer_pop(pReady->remote_thread_name);
}

static void
_thread_guardian_local_thread_ready(ThreadLocalReadyMsg *pReady)
{
	THREADDEBUG("%s", pReady->local_thread_name);

	thread_msg_buffer_pop(pReady->local_thread_name);
}

static void
_thread_guardian_main(MSGBODY *msg)
{
	switch((ub)(msg->msg_id))
	{
		case MSGID_TEST:
				_thread_guardian_test(msg);
			break;
		case MSGID_DEBUG_REQ:
				_thread_guardian_debug(msg->msg_src, (DebugReq *)(msg->msg_body));
			break;
		case MSGID_WAKEUP:
				_thread_guardian_system_check();
			break;
		case MSGID_RUN_FUNCTION:
				_thread_guardian_run_function((RUNFUNCTIONMSG *)(msg->msg_body));
			break;
		case MSGID_RESTART_REQ:
				_thread_guardian_restart((RESTARTREQMSG *)(msg->msg_body));
			break;
		case MSGID_POWER_OFF:
				thread_quit(QUIT_TYPE_RESTART, ((POWEROFFMSG *)(msg->msg_body))->reason, _thread, THREAD_MAX);
			break;
		case MSGID_TRACE_SWITCH:
				_thread_guardian_trace_switch((TraceSwitchMsg *)(msg->msg_body));
			break;
		case MSGID_CFG_UPDATE:
				thread_busy_idle_cfg_update((CFGUpdate *)(msg->msg_body));
			break;
		case MSGID_REMOTE_THREAD_READY:
				_thread_guardian_remote_thread_ready((ThreadRemoteReadyMsg *)(msg->msg_body));
			break;
		case MSGID_REMOTE_THREAD_ID_READY:
				_thread_guardian_remote_thread_id_ready((ThreadRemoteIDReadyMsg *)(msg->msg_body));
			break;
		case MSGID_LOCAL_THREAD_READY:
				_thread_guardian_local_thread_ready((ThreadLocalReadyMsg *)(msg->msg_body));
			break;
		default:
			break;
	}
}

static void
_thread_guardian_init(MSGBODY *msg)
{
	thread_msg_buffer_init();
	thread_remote_id_table_init();
	thread_gid_table_init();
	thread_busy_idle_init(_thread);
	thread_sync_init();
}

static void
_thread_guardian_exit(MSGBODY *msg)
{
	thread_sync_exit();
	thread_busy_idle_exit();
	thread_gid_table_exit();
	thread_remote_id_table_exit();
	thread_msg_buffer_exit();
}

// =====================================================================

ThreadId
thread_guardian_init(ThreadStruct *thread_struct)
{
	ub thread_flag = THREAD_TICK_WAKEUP|THREAD_PRIVATE_FLAG;

	_thread = thread_struct;

	t_time_get_date(&_system_work_start_date);

	_guardian_thread = base_thread_creat(GUARDIAN_THREAD_NAME, 1, thread_flag, _thread_guardian_init, _thread_guardian_main, _thread_guardian_exit);

	return _guardian_thread;
}

void
thread_guardian_exit(void)
{
	if(_guardian_thread != INVALID_THREAD_ID)
	{
		base_thread_del(_guardian_thread);
		_guardian_thread = INVALID_THREAD_ID;
	}
}

#endif

