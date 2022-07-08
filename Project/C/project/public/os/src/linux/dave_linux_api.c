/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#if defined(__DAVE_CYGWIN__) || defined(__DAVE_LINUX__)
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <pthread.h>
#include <sys/resource.h>
#include "dave_os.h"
#include "dave_tools.h"
#include "dave_verno.h"
#include "dave_base.h"
#include "dave_linux.h"
#include "os_log.h"

// =====================================================================

void *
dave_os_malloc(ub len)
{
	if(len == 0)
	{
		return NULL;
	}

	return (void *)malloc((unsigned long)len);
}

void
dave_os_free(void *ptr)
{
	if(ptr == NULL)
	{
		return;
	}

	free(ptr);
}

ub
dave_os_size(void *ptr)
{
	if(ptr == NULL)
	{
		return 0;
	}

	return (ub)malloc_usable_size(ptr);
}

void
dave_os_restart(s8 *reason)
{
	OSLOG("%s", reason);
}

void
dave_os_power_off(s8 *reason)
{
	OSLOG("system power off reason:%s", reason);

	dave_os_sleep(3000);

	exit(0);
}

dave_bool
dave_os_process_exist(s8 *process_name)
{
	s8 temp_buffer[128];
	FILE *fp;
	dave_bool ret;

	dave_snprintf(temp_buffer, sizeof(temp_buffer), "ps -e | grep \'%s\' | awk \'{print $1}\'", process_name);

	fp = popen((const char *)temp_buffer, "r");

	if(fp == NULL)
	{
		ret = dave_false;
	}
	else
	{
		ret = fgets((char *)temp_buffer, sizeof(temp_buffer), fp) == NULL ? dave_false : dave_true;

		pclose(fp);
	}

	return ret;
}

dave_bool
dave_os_system(char *cmdstring, char *result_ptr, int result_len)
{
	FILE *result_file;

	dave_memset(result_ptr, 0x00, result_len);

	result_file = popen((const char *)cmdstring, "r");
	if(result_file == NULL)
	{
		return dave_false;
	}

	if(result_ptr != NULL)
	{
		fread(result_ptr, 1, result_len, result_file);
	}

	pclose(result_file);

	return dave_true;
}

sb
dave_os_get_system_file_max(void)
{
	struct rlimit limit;

	if(getrlimit(RLIMIT_NOFILE, &limit) == -1)
	{
		return -1;
	}

	return (sb)(limit.rlim_cur);
}

dave_bool
dave_os_set_system_file_max(sb file_max)
{
	struct rlimit limit;

	limit.rlim_cur = file_max;
	limit.rlim_max = file_max + 1;

	if(setrlimit(RLIMIT_NOFILE, &limit) == -1)
	{
		return dave_false;
	}

	return dave_true;
}

s8 *
dave_os_errno(sb *ret_errno)
{
	static s8 errno_str[1024];

	dave_snprintf(errno_str, sizeof(errno_str), "%s<%d>", strerror(errno), errno);

	if(ret_errno != NULL)
	{
		*ret_errno = (sb)errno;
	}

	return errno_str;
}

dave_bool
dave_os_on_docker(void)
{
	return dave_os_file_valid("/.dockerenv");
}

#endif

