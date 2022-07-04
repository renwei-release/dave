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

#define CFG_NGINX_BIN_NAME "NginxBin"
#define CFG_NGINX_CONF_NAME "NginxConf"

#define NGINX_BIN_NAME (s8 *)"/usr/sbin/nginx"
#define NGINX_CONF_NAME (s8 *)"/etc/nginx/nginx.conf"

static TLock _nginx_action_pv;
static dave_bool _nginx_working = dave_false;
static TIMERID _nginx_action_timer = INVALID_TIMER_ID;
static s8 _nginx_bin_name[128] = { 0x00 };
static s8 _nginx_conf_name[128] = { 0x00 };

static RetCode
_nginx_action_start(void)
{
	s8 cmd[512];
	dave_bool ret;

	if(dave_os_process_exist(_nginx_bin_name) == dave_false)
	{
		dave_snprintf(cmd, sizeof(cmd), "%s -c %s", _nginx_bin_name, _nginx_conf_name);
	}
	else
	{
		dave_snprintf(cmd, sizeof(cmd), "%s -s reload -c %s", _nginx_bin_name, _nginx_conf_name);
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

	if(dave_os_process_exist(_nginx_bin_name) == dave_true)
	{
		dave_sprintf(kill_cmd, "killall -9 %s", _nginx_bin_name);

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

	SAFECODEv1( _nginx_action_pv, ret = nginx_conf_add(_nginx_conf_name, work_process, nginx_port, type, cgi_port, nginx_path, pem_path, key_path); );

	return ret;
}

static ub
_nginx_action_safe_conf_del(ub work_process, ub nginx_port)
{
	ub has_server_number = 0;

	SAFECODEv1( _nginx_action_pv, has_server_number = nginx_conf_del(_nginx_conf_name, work_process, nginx_port); );

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

	base_timer_die(timer_id);
}


static void
_nginx_action_load_config(void)
{
	if(cfg_get(CFG_NGINX_BIN_NAME, _nginx_bin_name, sizeof(_nginx_bin_name)) == dave_false)
	{
		dave_strcpy(_nginx_bin_name, NGINX_BIN_NAME, sizeof(_nginx_bin_name));
		cfg_set(CFG_NGINX_BIN_NAME, _nginx_bin_name, dave_strlen(_nginx_bin_name));
	}

	if(cfg_get(CFG_NGINX_CONF_NAME, _nginx_conf_name, sizeof(_nginx_conf_name)) == dave_false)
	{
		dave_strcpy(_nginx_conf_name, NGINX_CONF_NAME, sizeof(_nginx_conf_name));
		cfg_set(CFG_NGINX_CONF_NAME, _nginx_conf_name, dave_strlen(_nginx_conf_name));
	}
}

// =====================================================================

void
nginx_action_init(void)
{
	t_lock_reset(&_nginx_action_pv);

	_nginx_working = dave_false;
	_nginx_action_timer = INVALID_TIMER_ID;
	_nginx_action_load_config();
}

void
nginx_action_exit(void)
{
	if(nginx_conf_number(_nginx_conf_name) == 0)
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
		base_timer_die(_nginx_action_timer);

		_nginx_action_timer = INVALID_TIMER_ID;
	}

	_nginx_action_timer = base_timer_creat("nginxaction", _nginx_action_timer_out, 6000);

	return RetCode_OK;
}

void
nginx_action_stop(void)
{
	SAFECODEv1( _nginx_action_pv, _nginx_working = dave_false; );

	_nginx_action_safe_stop();
}

#endif

