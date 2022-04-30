/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#if defined(NGINX_3RDPARTY)
#include <netdb.h>
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
#include <dlfcn.h>
#include "dave_os.h"
#include "dave_tools.h"
#include "dave_base.h"
#include "dave_http.h"
#include "nginx_conf.h"
#include "nginx_action.h"
#include "party_log.h"

#define NGINX_PROCESS_MAX 8

static ub
_nginx_max_work_process(void)
{
	ub process;

	process = dave_os_cpu_process_number();

	if(process > NGINX_PROCESS_MAX)
	{
		process = NGINX_PROCESS_MAX;
	}

	return process;
}

// =====================================================================

void
dave_nginx_init(void)
{
	nginx_action_init();
}

void
dave_nginx_exit(void)
{
	nginx_action_exit();
}

RetCode
dave_nginx_start(ub nginx_port, HTTPListenType type, ub cgi_port, s8 *nginx_path, s8 *pem_path, s8 *key_path)
{
	RetCode ret;
	ub work_process = _nginx_max_work_process();

	if((nginx_port == 0) || (cgi_port == 0) || (nginx_path == NULL) || (nginx_path[0] == '\0'))
	{
		PARTYABNOR("invalid param! nginx_port:%d cgi_port:%d nginx_path:%s", nginx_port, cgi_port, nginx_path);
		return RetCode_Invalid_parameter;
	}

	ret = nginx_action_conf_add(work_process, nginx_port, type, cgi_port, nginx_path, pem_path, key_path);
	if(ret == dave_false)
	{
		PARTYABNOR("conf add failed! work_process:%d nginx_port:%d type:%d cgi_port:%d nginx_path:%s",
			work_process, nginx_port, type, cgi_port, nginx_path);
		return RetCode_save_failed;
	}

	ret = nginx_action_start();
	if(ret != RetCode_OK)
	{
		PARTYABNOR("ret:%s", retstr(ret));
		return ret;
	}

	PARTYLOG("[NGINX]booting, nginx-port:%d listen-type:%s cgi-port:%d nginx-path:%s work-process:%d",
		nginx_port, t_auto_HTTPListenType_str(type), cgi_port, nginx_path,
		work_process);

	return ret;
}

RetCode
dave_nginx_stop(ub nginx_port)
{
	ub has_server_number;
	RetCode ret;
	ub work_process = _nginx_max_work_process();
	
	has_server_number = nginx_action_conf_del(work_process, nginx_port);

	PARTYLOG("[NGINX]stopping nginx-port:%d has-server-number:%d",
		nginx_port, has_server_number);

	if((has_server_number > 0) && (dave_power_state() == dave_true))
	{
		ret = nginx_action_start();
		if(ret != RetCode_OK)
		{
			PARTYABNOR("ret:%s", retstr(ret));
			return ret;
		}
	}
	else
	{
		nginx_action_stop();
		PARTYLOG("[NGINX]stopping ... ");
	}

	return RetCode_OK;
}

#endif

