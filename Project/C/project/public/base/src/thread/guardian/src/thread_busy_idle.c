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
#include "dave_verno.h"
#include "base_tools.h"
#include "thread_parameter.h"
#include "thread_struct.h"
#include "thread_tools.h"
#include "thread_cfg.h"
#include "thread_log.h"

#define HOW_LONG_WILL_IT_TAKE_TO_CONTINUE_DETECTED 6
#define MESSAGE_QUEUING_MULTIPLE 3

static ThreadStruct *_thread_struct = NULL;
static ub _notify_last_time = 0;
static dave_bool _notify_busy_flag = dave_false;
static dave_bool _app_busy_flag = dave_false;

static inline void
_thread_busy_idle_system_notify(dave_bool notify_busy_flag)
{
	if(thread_id(SYNC_CLIENT_THREAD_NAME) == INVALID_THREAD_ID)
	{
		THREADLOG("not has %s", SYNC_CLIENT_THREAD_NAME);
		return;
	}

	THREADLOG("****** system on %s ******",
		notify_busy_flag == dave_true? "busy" : "idle");

	if(notify_busy_flag == dave_true)
	{
		SystemBusy *pBusy = thread_msg(pBusy);

		dave_strcpy(pBusy->gid, globally_identifier(), sizeof(pBusy->gid));
		dave_strcpy(pBusy->verno, dave_verno(), sizeof(pBusy->verno));

		name_msg(SYNC_CLIENT_THREAD_NAME, MSGID_SYSTEM_BUSY, pBusy);
	}
	else
	{
		SystemIdle *pIdle = thread_msg(pIdle);

		dave_strcpy(pIdle->gid, globally_identifier(), sizeof(pIdle->gid));
		dave_strcpy(pIdle->verno, dave_verno(), sizeof(pIdle->verno));

		name_msg(SYNC_CLIENT_THREAD_NAME, MSGID_SYSTEM_IDLE, pIdle);
	}
}

static inline ub
_thread_busy_idle_total_number(ThreadStruct *pThread)
{
	ub total_msg_number, coroutines_site_creat_counter, coroutines_site_release_counter;

	total_msg_number = thread_total_number(pThread);

	coroutines_site_creat_counter = pThread->coroutines_site_creat_counter;
	coroutines_site_release_counter = pThread->coroutines_site_release_counter;
	if(coroutines_site_creat_counter > coroutines_site_release_counter)
	{
		total_msg_number += (coroutines_site_creat_counter - coroutines_site_release_counter);
	}

	return total_msg_number;
}

static inline dave_bool
_thread_busy_idle_detected_busy(ThreadStruct *pThread)
{
	ub total_msg_number;

	total_msg_number = _thread_busy_idle_total_number(pThread);

	if(total_msg_number >= (MESSAGE_QUEUING_MULTIPLE * pThread->level_number))
	{
		return dave_true;
	}

	return dave_false;
}

static inline dave_bool
_thread_busy_idle_detected_idle(ThreadStruct *pThread)
{
	ub total_msg_number;

	total_msg_number = _thread_busy_idle_total_number(pThread);

	if(total_msg_number == 0)
	{
		return dave_true;
	}

	return dave_false;
}

static void
_thread_busy_idle_check(void)
{
	ub thread_index;
	ThreadStruct *pThread;
	dave_bool notify_busy, notify_idle;

	notify_busy = notify_idle = dave_false;

	for(thread_index=0; thread_index<THREAD_MAX; thread_index++)
	{
		pThread = &_thread_struct[thread_index];
	
		if((pThread->thread_id != INVALID_THREAD_ID)
			&& (pThread->has_initialization == dave_true)
			&& (pThread->thread_flag & THREAD_THREAD_FLAG)
			&& ((pThread->thread_flag & THREAD_REMOTE_FLAG) == 0x00)
			&& ((pThread->thread_flag & THREAD_PRIVATE_FLAG) == 0x00)
			&& ((pThread->thread_flag & THREAD_CORE_FLAG) == 0x00))
		{
			if(_notify_busy_flag == dave_false)
			{
				if(_thread_busy_idle_detected_busy(pThread) == dave_true)
				{
					notify_busy = dave_true;
					break;
				}
			}
			else
			{
				if(_thread_busy_idle_detected_idle(pThread) == dave_false)
				{
					break;
				}
			}
		}
	}
	if((_notify_busy_flag == dave_true) && (thread_index >= THREAD_MAX))
	{
		notify_idle = dave_true;
	}

	if(notify_busy == dave_true)
	{
		_notify_busy_flag = dave_true;
		_thread_busy_idle_system_notify(_notify_busy_flag);
	}
	else if(notify_idle == dave_true)
	{
		_notify_busy_flag = dave_false;
		_thread_busy_idle_system_notify(_notify_busy_flag);
	}
}

static void
_thread_busy_idle_app_busy(void)
{
	THREADLOG("application is %s, set busy now!", _app_busy_flag==dave_true?"busy":"idle");
	_app_busy_flag = _notify_busy_flag = dave_true;
	_thread_busy_idle_system_notify(_app_busy_flag);

	thread_cfg_system_startup_flag_set(dave_false);
}

static void
_thread_busy_idle_app_idle(void)
{
	THREADLOG("application is %s, wait _notify_busy_flag change to idle!", _app_busy_flag==dave_true?"busy":"idle");
	_app_busy_flag = dave_false;

	thread_cfg_system_startup_flag_set(dave_true);
}

// =====================================================================

void
thread_busy_idle_init(ThreadStruct *thread_struct)
{
	_thread_struct = thread_struct;
	_notify_last_time = dave_os_time_s();
	_notify_busy_flag = dave_false;
	_app_busy_flag = dave_false;
	#ifndef __DAVE_PRODUCT_SYNC__
	if(thread_cfg_system_startup_flag_get() == dave_false)
	{
		_thread_busy_idle_app_busy();
	}
	#endif
}

void
thread_busy_idle_exit(void)
{
	_thread_busy_idle_app_busy();
}

void
thread_busy_idle_check(void)
{
	if((dave_os_time_s() - _notify_last_time) < HOW_LONG_WILL_IT_TAKE_TO_CONTINUE_DETECTED)
	{
		return;
	}
	_notify_last_time = dave_os_time_s();

	if(_app_busy_flag == dave_true)
	{
		THREADLOG("application lever is busy!");
		return;
	}

	_thread_busy_idle_check();
}

void
thread_busy_idle_app_busy(void)
{
	_thread_busy_idle_app_busy();
}

void
thread_busy_idle_app_idle(void)
{
	_thread_busy_idle_app_idle();
}

#endif

