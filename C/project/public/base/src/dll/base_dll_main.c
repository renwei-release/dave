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
#include <linux/unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "dave_linux.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_verno.h"
#include "base_dll.h"
#include "base_dll_main.h"
#include "dll_log.h"

typedef struct {
	char msg_src_name[128];
	unsigned long long msg_src;
	char msg_dst_name[128];
	unsigned long long msg_dst;
	unsigned long long msg_id;
	unsigned long long msg_len;
	unsigned long long msg_check;
	void *msg_body;
} DllMsgBody;

static ThreadId _main_thread = INVALID_THREAD_ID;
static dll_callback_fun _dll_main_fun = NULL;

static char *
_dll_main_name(void)
{
	static s8 main_name[128];
	s8 *product_name = dave_verno_my_product();

	dave_snprintf(main_name, sizeof(main_name), "%s", product_name);

	return (char *)main_name;
}

static ub
_dll_main_number(void)
{
	return dave_os_cpu_process_number();
}

static void
_dll_main_init(MSGBODY *msg)
{

}

static void
_dll_main_main(MSGBODY *msg)
{
	DllMsgBody msg_body;

	if(_dll_main_fun != NULL)
	{
		dave_memset(&msg_body, 0x00, sizeof(DllMsgBody));

		dave_strcpy(msg_body.msg_src_name, thread_name(msg->msg_src), sizeof(msg_body.msg_src_name));
		msg_body.msg_src = msg->msg_src;
		dave_strcpy(msg_body.msg_dst_name, thread_name(msg->msg_dst), sizeof(msg_body.msg_dst_name));
		msg_body.msg_dst = msg->msg_dst;
		msg_body.msg_id = msg->msg_id;
		msg_body.msg_len = msg->msg_len;
		msg_body.msg_check = 1234567890;
		msg_body.msg_body = msg->msg_body;

		DLLDEBUG("msg_src:%s msg_id:%d msg_len:%d msg_body:%lx",
			msg_body.msg_src,
			msg_body.msg_id,
			msg_body.msg_len,
			msg_body.msg_body);

		_dll_main_fun((void *)(&msg_body));
	}
}

static void
_dll_main_exit(MSGBODY *msg)
{

}

// =====================================================================

void
dave_dll_main_init(dll_callback_fun dll_main_fun)
{
	_dll_main_fun = dll_main_fun;

	_main_thread = dave_thread_creat(_dll_main_name(), _dll_main_number(), THREAD_THREAD_FLAG, _dll_main_init, _dll_main_main, _dll_main_exit);
	if(_main_thread == INVALID_THREAD_ID)
		dave_restart(_dll_main_name());
}

void
dave_dll_main_exit(void)
{
	if(_main_thread != INVALID_THREAD_ID)
		dave_thread_del(_main_thread);
	_main_thread = INVALID_THREAD_ID;
}

ThreadId
dave_dll_main_thread_id(void)
{
	return _main_thread;
}

#endif

