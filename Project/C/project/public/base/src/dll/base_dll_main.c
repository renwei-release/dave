/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h> 
#include <time.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_verno.h"
#include "dave_os.h"
#include "base_dll.h"
#include "base_dll_main.h"
#include "dll_log.h"

// #define ENABLE_PROTECTOR

#define DLL_MAIN_THREAD_MAX_NUMBER 512
#define CFG_COROUTINE_STACK_SIZE "CoroutineStackSize"

typedef struct {
	char msg_src_gid[64];
	char msg_src_name[128];
	unsigned long long msg_src;
	char msg_dst_name[128];
	unsigned long long msg_dst;
	unsigned long long msg_id;
	unsigned long long msg_len;
	unsigned long long msg_check;
	void *msg_body;
} DllMsgBody;

static void _dll_main_exit(MSGBODY *msg);

static ThreadId _main_thread = INVALID_THREAD_ID;
static dll_callback_fun _dll_init_fun = NULL;
static dll_callback_fun _dll_main_fun = NULL;
static dll_callback_fun _dll_exit_fun = NULL;

static s8 *
_dll_main_name(s8 *product_name)
{
	if((product_name != NULL) && (dave_strlen(product_name) > 0))
	{
		return product_name;
	}
	else
	{
		return t_gp_product_name();
	}
}

static ub
_dll_main_number(ub thread_number)
{
	ub max_thread_number;

	if(thread_number == 0)
	{
		thread_number = dave_os_cpu_process_number();
	}

	max_thread_number = DLL_MAIN_THREAD_MAX_NUMBER;

	if(thread_number > max_thread_number)
	{
		thread_number = max_thread_number;
	}

	return thread_number;
}

static ub
_dll_main_flag(BaseDllRunningMode mode)
{
	if((mode == BaseDllRunningMode_Inner_Loop) || (mode == BaseDllRunningMode_Outer_Loop))
		return THREAD_THREAD_FLAG | THREAD_dCOROUTINE_FLAG;
	else
		return THREAD_THREAD_FLAG | THREAD_COROUTINE_FLAG;
}

static void
_dll_main_restart(RESTARTREQMSG *pRestart)
{
	if(pRestart->times == 1)
	{
		_dll_main_exit(NULL);
	}
}

static void
_dll_main_run_msg(MSGBODY *msg)
{
	DllMsgBody *pBody = dave_malloc(sizeof(DllMsgBody));

	if(_dll_main_fun != NULL)
	{
		dave_strcpy(pBody->msg_src_gid, msg->src_gid, sizeof(pBody->msg_src_gid));

		dave_strcpy(pBody->msg_src_name, thread_name(msg->msg_src), sizeof(pBody->msg_src_name));
		pBody->msg_src = msg->msg_src;

		dave_strcpy(pBody->msg_dst_name, thread_name(msg->msg_dst), sizeof(pBody->msg_dst_name));
		pBody->msg_dst = msg->msg_dst;

		pBody->msg_id = msg->msg_id;
		pBody->msg_len = msg->msg_len;

		pBody->msg_check = 1234567890;

		pBody->msg_body = msg->msg_body;

		DLLDEBUG("%s->%s:%s/%d msg_body:%lx",
			pBody->msg_src_name, pBody->msg_dst_name, msgstr(pBody->msg_id),
			pBody->msg_len,
			pBody->msg_body);

		_dll_main_fun((void *)(pBody));
	}

	dave_free(pBody);
}

#ifdef ENABLE_PROTECTOR

static void
_dll_main_protector_reg(void)
{
	ProtectorReg *pReg = thread_reset_msg(pReg);

	name_msg(GUARDIAN_THREAD_NAME, MSGID_PROTECTOR_REG, pReg);
}

static void
_dll_main_protector_unreg(void)
{
	ProtectorUnreg *pUnreg = thread_reset_msg(pUnreg);

	name_msg(GUARDIAN_THREAD_NAME, MSGID_PROTECTOR_UNREG, pUnreg);
}

#endif

static void
_dll_main_init(MSGBODY *msg)
{
	if(_dll_init_fun != NULL)
	{
		_dll_init_fun(NULL);
		_dll_init_fun = NULL;

#ifdef ENABLE_PROTECTOR
		_dll_main_protector_reg();
#endif
	}
}

static void
_dll_main_main(MSGBODY *msg)
{
	switch((ub)msg->msg_id)
	{
		case MSGID_RESTART_REQ:
				_dll_main_restart((RESTARTREQMSG *)(msg->msg_body));
			break;
		default:
				_dll_main_run_msg(msg);
			break;
	}
}

static void
_dll_main_exit(MSGBODY *msg)
{
	if(_dll_exit_fun != NULL)
	{
#ifdef ENABLE_PROTECTOR
		_dll_main_protector_unreg();
#endif

		_dll_exit_fun(NULL);
		_dll_exit_fun = NULL;
	}
}

// =====================================================================

void
dave_dll_main_init(
	s8 *product_name,
	BaseDllRunningMode mode,
	int thread_number,
	dll_callback_fun dll_init_fun, dll_callback_fun dll_main_fun, dll_callback_fun dll_exit_fun)
{
	ub thread_flag = _dll_main_flag(mode);

	_dll_init_fun = dll_init_fun;
	_dll_main_fun = dll_main_fun;
	_dll_exit_fun = dll_exit_fun;

	if(thread_flag & THREAD_COROUTINE_FLAG)
	{
		cfg_set_ub(CFG_COROUTINE_STACK_SIZE, 512 * 1024);
	}

	_main_thread = base_thread_creat(_dll_main_name(product_name), _dll_main_number(thread_number), thread_flag, _dll_main_init, _dll_main_main, _dll_main_exit);
	if(_main_thread == INVALID_THREAD_ID)
		base_restart(_dll_main_name(product_name));
}

void
dave_dll_main_exit(void)
{
	if(_main_thread != INVALID_THREAD_ID)
		base_thread_del(_main_thread);
	_main_thread = INVALID_THREAD_ID;
}

ThreadId
dave_dll_main_thread_id(void)
{
	return _main_thread;
}

#endif

