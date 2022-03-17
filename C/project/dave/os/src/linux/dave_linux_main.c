/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

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
#include "dave_os.h"
#include "dave_third_party.h"
#include "dave_linux.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_verno.h"

static int _dave_has_argc_number;
static s8 _dave_has_reboot_message[2048];
static void *_dave_main_thread_id = NULL;

static void
_linux_copy_reboot_message(int argc, char **argv)
{
	_dave_has_argc_number = argc;
	dave_memset(_dave_has_reboot_message, 0x00, sizeof(_dave_has_reboot_message));

	// daemon_process gave argv data.
	if(_dave_has_argc_number >= 2)
	{
		dave_strcpy(_dave_has_reboot_message, (s8 *)(argv[1]), sizeof(_dave_has_reboot_message));
	}
}

static void
_linux_if_has_reboot_message(void)
{
	if(dave_strlen(_dave_has_reboot_message) > 0)
	{
		DAVELOG("***********REBOOT MESSAGE**********\r\n");
		DAVEABNORMAL("reboot message:%s", _dave_has_reboot_message);
		DAVELOG("***********************************\r\n");
	}
}

static void *
_linux_main_thread(void *arg)
{
	base_init(_dave_main_thread_id);

	_linux_if_has_reboot_message();

	base_running(dave_false);

	base_exit();

	dave_os_release_thread(_dave_main_thread_id);

	dave_os_thread_exit(_dave_main_thread_id);

	kill(getpid(), QUIT_SIG);

	return NULL;
}

static void
_linux_handle_signal(void)
{
	sigset_t set;
	int sig;
	int ret;

	sigemptyset(&set);
	sigaddset(&set, TIMER_SIG);
	sigaddset(&set, QUIT_SIG);
	sigaddset(&set, IO_SIG);
	sigaddset(&set, KILL_SIG);

	while (1)
	{
		ret = sigwait(&set, &sig);

		if (ret == 0)
		{
			if(sig == TIMER_SIG)
			{
				dave_os_timer_notify((unsigned long)sig);
			}
			else if(sig == KILL_SIG)
			{
				base_restart("KILL");
			}
			else if(sig == QUIT_SIG)
			{
				break;
			}
		}
	}
}

// =====================================================================

int
main(int argc, char **argv)
{
	dave_os_init_thread();

	_linux_copy_reboot_message(argc, argv);

	_dave_main_thread_id = dave_os_create_thread(dave_verno_product(NULL, NULL, 0), _linux_main_thread, NULL);
	if (_dave_main_thread_id == NULL)
	{
		base_restart("main reboot");
	}
	else
	{
		_linux_handle_signal();
	}

	dave_os_exit_thread();

	return 0;
}

