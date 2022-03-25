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

typedef enum {
	BaseDllRunningMode_Inner_Loop,
	BaseDllRunningMode_Outer_Loop,
	BaseDllRunningMode_max
} BaseDllRunningMode;

static void *_dave_main_thread_id = NULL;
static void *_dave_inner_loop_id = NULL;
static dll_callback_fun _dll_init_fun = NULL;
static dll_callback_fun _dll_main_fun = NULL;
static dll_callback_fun _dll_exit_fun = NULL;
static BaseDllRunningMode _base_dll_running_mode = BaseDllRunningMode_max;

static void
_dave_dll_booting(char *my_verno)
{
	if(dave_verno_reset((s8 *)my_verno) == dave_false)
	{
		printf("dll set invalid verno:%s\n", my_verno);
	}
}

static BaseDllRunningMode
_dave_dll_mode_decode(char *mode)
{
	return dave_strcmp(mode, "Inner Loop") == dave_true ? BaseDllRunningMode_Inner_Loop : BaseDllRunningMode_Outer_Loop;
}

static void *
_dave_dll_inner_loop(void *arg)
{
	sigset_t set;
	int sig;
	int ret;

	sigemptyset(&set);
	sigaddset(&set, TIMER_SIG);
	sigaddset(&set, QUIT_SIG);
	sigaddset(&set, IO_SIG);
	sigaddset(&set, KILL_SIG);

	while(1)
	{
		ret = sigwait(&set, &sig);

		if (ret == 0)
		{
			if (sig == TIMER_SIG)
			{
				dave_os_timer_notify((unsigned long)sig);
			}
			else if (sig == KILL_SIG)
			{
				dave_restart("KILL");
			}
			else if (sig == QUIT_SIG)
			{
				break;
			}
		}
	}

	return NULL;
}

static void *
_dave_dll_main_thread(void *arg)
{
	base_init(_dave_main_thread_id);

	dave_dll_main_init(_dll_main_fun);

	if(_dll_init_fun != NULL)
	{
		_dll_init_fun(NULL);
	}

	base_running(dave_false);

	if(_dll_exit_fun != NULL)
	{
		_dll_exit_fun(NULL);
	}

	dave_dll_main_exit();

	base_exit();

	kill(getpid(), QUIT_SIG);

	return NULL;
}

static void
_dave_dll_init(
	char *my_verno, char *mode,
	dll_callback_fun init_fun, dll_callback_fun main_fun, dll_callback_fun exit_fun)
{
	_dll_init_fun = init_fun;
	_dll_main_fun = main_fun;
	_dll_exit_fun = exit_fun;

	_base_dll_running_mode = _dave_dll_mode_decode(mode);

	_dave_dll_booting(my_verno);

	dave_os_init_thread();

	_dave_main_thread_id = dave_os_create_thread("dave", _dave_dll_main_thread, NULL);
	if(_dave_main_thread_id == NULL)
	{
		dave_restart("main reboot");
	}
	else
	{
		dave_os_sleep(1000);

		if(_base_dll_running_mode == BaseDllRunningMode_Inner_Loop)
		{
			_dave_inner_loop_id = dave_os_create_thread("dave-inner-loop", _dave_dll_inner_loop, NULL);
		}
	}
}

static void
_dave_dll_exit(void)
{
	dave_os_thread_exit(_dave_main_thread_id);

	dave_os_release_thread(_dave_main_thread_id);

	dave_os_thread_exit(_dave_inner_loop_id);

	dave_os_release_thread(_dave_inner_loop_id);

	dave_os_exit_thread();
}

// =====================================================================

void
dave_dll_init(
	char *my_verno, char *mode,
	dll_callback_fun init_fun, dll_callback_fun main_fun, dll_callback_fun exit_fun)
{
	_dave_dll_init(my_verno, mode, init_fun, main_fun, exit_fun);
}

void
dave_dll_running(void)
{
	if(_base_dll_running_mode == BaseDllRunningMode_Outer_Loop)
	{
		_dave_dll_inner_loop(NULL);
	}
	else
	{
		printf("Run in inner loop mode, no need to call this function!\n");
	}
}

void
dave_dll_exit(void)
{
	if(dave_power_state() == dave_true)
	{
		dave_restart("dll exit");
	}

	_dave_dll_exit();
}

#endif

