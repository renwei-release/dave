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
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "public/os/inc/dave_os.h"
#include "dave_3rdparty.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_verno.h"

void product_init(void);
void product_exit(void);

static void *_dave_main_thread_id = NULL;

static dave_bool
_linux_parse_the_command_line(int argc, char **argv)
{
	dave_bool boot_main_flag = dave_true;

	if(argc < 2)
	{
		return boot_main_flag;
	}

	if(dave_strcmp(argv[1], "-v")
		|| dave_strcmp(argv[1], "-ver")
		|| dave_strcmp(argv[1], "-version"))
	{
		boot_main_flag = dave_false;
		fprintf(stdout,
			"Version:%s\nHost:%s\nCompiler:%s\nMac:%s\n",
			dave_verno(),
			__BUILD_HOSTNAME__, __BUILD_USERNAME__, __BUILD_MAC_ADDRESS__);
	}

	return boot_main_flag;
}

static void *
_linux_main_thread(void *arg)
{
	base_init(_dave_main_thread_id, NULL);

	product_init();

	base_running(dave_false);

	product_exit();

	base_exit();

	kill(getpid(), QUIT_SIG);

	return NULL;
}

static void
_linux_handle_alarm(int signo)
{
	printf("_linux_handle_alarm:%d\n", signo);
}

static void
_linux_handle_signal(void)
{
	sigset_t set;
	int sig;
	int ret;

	signal(SIGALRM, _linux_handle_alarm);

	sigemptyset(&set);
	sigaddset(&set, TIMER_SIG);
	sigaddset(&set, QUIT_SIG);
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

static void
_linux_main_function(void)
{
	dave_os_init_thread();
	
	_dave_main_thread_id = dave_os_create_thread(dave_verno_my_product(), _linux_main_thread, NULL);
	if (_dave_main_thread_id == NULL)
	{
		base_restart("main reboot");
	}
	else
	{
		_linux_handle_signal();

		dave_os_release_thread(_dave_main_thread_id);

		dave_os_sleep(500);
	}

	dave_os_exit_thread();
}

// =====================================================================

int
main(int argc, char **argv)
{
	if(_linux_parse_the_command_line(argc, argv) == dave_true)
	{
		_linux_main_function();
	}

	return 0;
}

