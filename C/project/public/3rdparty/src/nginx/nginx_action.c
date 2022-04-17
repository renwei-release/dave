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
#include <netinet/in.h>
#include <time.h> 
#include <dirent.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <dlfcn.h>
#include "dave_os.h"
#include "dave_tools.h"
#include "dave_verno.h"
#include "dave_base.h"
#include "dave_http.h"
#include "nginx_conf.h"
#include "party_log.h"

#define PCRE_LIB_DIR (s8 *)"/dave/tools/pcre/lib"
#define NGINX_BIN_DIR (s8 *)"/dave/tools/nginx/sbin"
#define NGINX_BIN_NAME (s8 *)"davenginx"

static TLock _nginx_action_pv;
static dave_bool _nginx_working = dave_false;
static TIMERID _nginx_action_timer = INVALID_TIMER_ID;

static void
_nginx_copy_nginx(void)
{
	// 这是为了防止同一台机器上还有其他服务也启动了nginx.
	s8 cmd[512];
	int ret;

	dave_snprintf(cmd, sizeof(cmd), "cp -rf %s/%s %s/%s", NGINX_BIN_DIR, "nginx", NGINX_BIN_DIR, NGINX_BIN_NAME);

	ret = system((const char *)cmd);

	if(ret != 0)
	{
		PARTYABNOR("system run %s failed:%d", cmd, ret);
	}
}

static RetCode
_nginx_action_start(void)
{
	s8 cmd[512];
	dave_bool ret;

	if(dave_os_process_exist(NGINX_BIN_NAME) == dave_false)
	{
		dave_sprintf(cmd, "export LD_LIBRARY_PATH=%s; %s/%s", PCRE_LIB_DIR, NGINX_BIN_DIR, NGINX_BIN_NAME);
	}
	else
	{
		dave_sprintf(cmd, "%s/%s -s reload", NGINX_BIN_DIR, NGINX_BIN_NAME);
	}

	ret = dave_os_system((char *)cmd, NULL, 0);

	PARTYDEBUG("cmd:%s ret:%d", cmd, ret);

	if(ret != dave_true)
	{
		PARTYABNOR("system run %s failed:%d", cmd, ret);
		return RetCode_script_execution_error;
	}

	return RetCode_OK;
}

static RetCode
_nginx_action_stop(void)
{
	s8 kill_cmd[128];
	int ret = -1;

	if(dave_os_process_exist(NGINX_BIN_NAME) == dave_true)
	{
		dave_sprintf(kill_cmd, "killall -9 %s", NGINX_BIN_NAME);

		ret = system((const char *)kill_cmd);

		PARTYDEBUG("cmd:%s ret:%d", kill_cmd, ret);
	}
	else
	{
		ret = 0;
	}

	if((ret != 0) && (ret != 256))
	{
		return RetCode_script_execution_error;
	}

	return RetCode_OK;
}

static RetCode
_nginx_action_safe_start(void)
{
	RetCode ret = RetCode_Resource_conflicts;

	SAFECODEv1(_nginx_action_pv, ret = _nginx_action_start(); );

	return ret;
}

static RetCode
_nginx_action_safe_stop(void)
{
	RetCode ret = RetCode_Resource_conflicts;

	SAFECODEv1( _nginx_action_pv, ret = _nginx_action_stop(); );

	return ret;
}

static dave_bool
_nginx_action_safe_conf_add(ub work_process, ub nginx_port, HTTPListenType type, ub cgi_port, s8 *nginx_path, s8 *pem_path, s8 *key_path)
{
	dave_bool ret = dave_false;

	SAFECODEv1( _nginx_action_pv, ret = nginx_conf_add(work_process, nginx_port, type, cgi_port, nginx_path, pem_path, key_path); );

	return ret;
}

static ub
_nginx_action_safe_conf_del(ub work_process, ub nginx_port)
{
	ub has_server_number = 0;

	SAFECODEv1( _nginx_action_pv, has_server_number = nginx_conf_del(work_process, nginx_port); );

	return has_server_number;
}

static void
_nginx_action_timer_out(TIMERID timer_id, ub thread_index)
{
	if(timer_id == _nginx_action_timer)
	{
		if(_nginx_working == dave_true)
		{
			_nginx_action_safe_start();
		}

		_nginx_action_timer = INVALID_TIMER_ID;
	}

	dave_timer_die(timer_id);
}

// =====================================================================

void
nginx_action_init(void)
{
	t_lock_reset(&_nginx_action_pv);

	_nginx_working = dave_false;

	_nginx_action_timer = INVALID_TIMER_ID;

	_nginx_copy_nginx();
}

void
nginx_action_exit(void)
{
	if(nginx_conf_number() == 0)
	{
		_nginx_action_safe_stop();
	}
	else
	{
		_nginx_action_safe_start();
	}
}

dave_bool
nginx_action_conf_add(ub work_process, ub nginx_port, HTTPListenType type, ub cgi_port, s8 *nginx_path, s8 *pem_path, s8 *key_path)
{
	return _nginx_action_safe_conf_add(work_process, nginx_port, type, cgi_port, nginx_path, pem_path, key_path);
}

ub
nginx_action_conf_del(ub work_process, ub nginx_port)
{
	return _nginx_action_safe_conf_del(work_process, nginx_port);
}

RetCode
nginx_action_start(void)
{
	SAFECODEv1( _nginx_action_pv, _nginx_working = dave_true; );

	if(_nginx_action_timer != INVALID_TIMER_ID)
	{
		dave_timer_die(_nginx_action_timer);

		_nginx_action_timer = INVALID_TIMER_ID;
	}

	_nginx_action_timer = dave_timer_creat("nginxaction", _nginx_action_timer_out, 6000);

	return RetCode_OK;
}

void
nginx_action_stop(void)
{
	SAFECODEv1( _nginx_action_pv, _nginx_working = dave_false; );

	_nginx_action_safe_stop();
}

#endif

