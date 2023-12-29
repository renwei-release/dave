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
#include "thread_queue.h"
#include "thread_remote_id_table.h"
#include "thread_gid_table.h"
#include "thread_wakeup_the_sleep.h"
#include "thread_run_function.h"
#include "thread_running.h"
#include "thread_orchestration.h"
#include "thread_coroutine.h"
#include "thread_cfg.h"
#include "thread_log.h"

static void _thread_guardian_exit(MSGBODY *msg);

static ThreadId _guardian_thread = INVALID_THREAD_ID;
static ThreadStruct *_thread;
static s8 _sync_domain[256] = { '\0' };
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
_thread_guardian_debug(ThreadId src, DebugReq *pReq)
{
	DebugRsp *pRsp = thread_reset_msg(pRsp);
	ub msg_len;

	if(pReq->msg[0] == 'w')
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
	else if(pReq->msg[0] == 'o')
	{
		thread_orchestration_info(pRsp->msg, sizeof(pRsp->msg));
	}
	else if(dave_strcmp(pReq->msg, "se") == dave_true)
	{
		thread_statistics_enable(pRsp->msg, sizeof(pRsp->msg));
	}
	else if(dave_strcmp(pReq->msg, "sd") == dave_true)
	{
		thread_statistics_disable(pRsp->msg, sizeof(pRsp->msg));
	}
	else if(dave_strcmp(pReq->msg, "si") == dave_true)
	{
		thread_statistics_info(pRsp->msg, sizeof(pRsp->msg));
	}
	else if(dave_strcmp(pReq->msg, "co") == dave_true)
	{
		thread_coroutine_info(pRsp->msg, sizeof(pRsp->msg));
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
	if(pReq->times == 1)
	{
		thread_busy_idle_exit();
	}
	else if(pReq->times == 4)
	{
		thread_statistics_exit();
		thread_orchestration_exit();
		thread_sync_exit();
		thread_queue_exit();
		thread_gid_table_exit();
		thread_remote_id_table_exit();
		thread_msg_buffer_exit();
		thread_cfg_exit();
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
_thread_guardian_cfg_update(CFGUpdate *pUpdate)
{
	thread_chain_reload_cfg(pUpdate);
	thread_coroutine_reload_cfg(pUpdate);
}

static void
_thread_guardian_remote_thread_ready(ThreadRemoteReadyMsg *pReady)
{
	THREADDEBUG("%s", pReady->remote_thread_name);

	thread_msg_buffer_thread_pop(pReady->remote_thread_name);
}

static void
_thread_guardian_remote_thread_id_ready(ThreadRemoteIDReadyMsg *pReady)
{
	THREADDEBUG("%s", pReady->remote_thread_name);

	thread_msg_buffer_thread_pop(pReady->remote_thread_name);
}

static void
_thread_guardian_local_thread_ready(ThreadLocalReadyMsg *pReady)
{
	THREADDEBUG("%s", pReady->local_thread_name);

	thread_msg_buffer_thread_pop(pReady->local_thread_name);
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
				thread_guardian_run_function((RUNFUNCTIONMSG *)(msg->msg_body));
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
				_thread_guardian_cfg_update((CFGUpdate *)(msg->msg_body));
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
		case MSGID_APPLICATION_BUSY:
				thread_busy_idle_app_busy();
			break;
		case MSGID_APPLICATION_IDLE:
				thread_busy_idle_app_idle();
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
	thread_sync_init(_sync_domain);
	thread_orchestration_init();
	thread_statistics_init();
	thread_cfg_init();
	thread_queue_init();
	thread_busy_idle_init(_thread);
}

static void
_thread_guardian_exit(MSGBODY *msg)
{

}

// =====================================================================

ThreadId
thread_guardian_init(ThreadStruct *thread_struct, s8 *sync_domain)
{
	ub thread_flag = THREAD_TICK_WAKEUP|THREAD_PRIVATE_FLAG|THREAD_CORE_FLAG;

	_thread = thread_struct;
	if(sync_domain != NULL)
		dave_strcpy(_sync_domain, sync_domain, sizeof(_sync_domain));
	else
		dave_memset(_sync_domain, 0x00, sizeof(_sync_domain));

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

