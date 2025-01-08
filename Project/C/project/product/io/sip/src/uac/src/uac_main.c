/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_base.h"
#include "dave_tools.h"
#include "sip_signal.h"
#include "uac_main.h"
#include "uac_cfg.h"
#include "uac_log.h"

UACClass *_uac_class = NULL;

static UACClass *
_uac_main_creat(
	s8 *server_ip, s8 *server_port, s8 *username, s8 *password,
	s8 *local_ip, s8 *local_port,
	s8 *rtp_ip, s8 *rtp_port)
{
	UACClass *pClass;

	pClass = dave_ralloc(sizeof(UACClass));
	pClass->phone_number_kv = kv_malloc("uacnumberkv", 0, NULL);
	pClass->signal = sip_signal_creat(server_ip, server_port, username, password, local_ip, local_port, rtp_ip, rtp_port);

	UACLOG("server:%s:%s user:%s/%s local:%s:%s rtp:%s:%s",
		pClass->signal->server_ip, pClass->signal->server_port,
		pClass->signal->username, pClass->signal->password,
		pClass->signal->local_ip, pClass->signal->local_port,
		pClass->signal->rtp_ip, pClass->signal->rtp_port);

	return pClass;
}

static RetCode
_uac_main_call_recycle(void *ramkv, s8 *key)
{
	UACCall *pCall = kv_del_key_ptr(ramkv, key);

	if(pCall == NULL)
		return RetCode_empty_data;

	dave_free(pCall);

	return RetCode_OK;
}

static void
_uac_main_call_delay_release(TIMERID timer_id, ub thread_index, void *param_ptr)
{
	UACCall *pCall = (RTP *)param_ptr;

	dave_free(pCall);

	base_timer_die(timer_id);
}

static void
_uac_main_release(UACClass *pClass)
{
	UACLOG("server:%s:%s user:%s/%s local:%s:%s rtp:%s:%s",
		pClass->signal->server_ip, pClass->signal->server_port,
		pClass->signal->username, pClass->signal->password,
		pClass->signal->local_ip, pClass->signal->local_port,
		pClass->signal->rtp_ip, pClass->signal->rtp_port);

	kv_free(pClass->phone_number_kv, _uac_main_call_recycle);

	sip_signal_release(pClass->signal);

	dave_free(pClass);
}

static void
_uac_main_booting(TIMERID timer_id, ub thread_index)
{
	s8 *server_ip, *server_port, *username, *password, *local_ip, *local_port, *rtp_ip, *rtp_port;

	server_ip = uac_cfg_sbc_server();
	server_port = uac_cfg_sbc_port();
	username = uac_cfg_username();
	password = uac_cfg_password();
	local_ip = uac_cfg_local_ip();
	local_port = uac_cfg_local_port();
	rtp_ip = uac_cfg_rtp_ip();
	rtp_port = uac_cfg_rtp_port();

	_uac_class = _uac_main_creat(server_ip, server_port, username, password, local_ip, local_port, rtp_ip, rtp_port);

	if(_uac_class == NULL)
	{
		UACLOG("%s:%s->%s:%s class creat failed!",
			local_ip, local_port, server_ip, server_port);
		return;
	}

	UACLOG("%s:%s->%s:%s socket:%d",
		local_ip, local_port, server_ip, server_port,
		_uac_class->signal->signal_socket);

	base_timer_die(timer_id);
}

static void
_uac_main_init(void)
{
	_uac_class = NULL;
}

static void
_uac_main_exit(void)
{
	_uac_main_release(_uac_class);

	_uac_class = NULL;
}

// =====================================================================

void
uac_main_init(void)
{
	_uac_main_init();

	base_timer_creat("umainbooting", _uac_main_booting, 5000);
}

void
uac_main_exit(void)
{
	_uac_main_exit();
}

SIPSignal *
uac_main_signal(void)
{
	return _uac_class->signal;
}

UACCall *
uac_main_build_call(ThreadId owner_id, s8 *phone_number)
{
	UACCall *pUACCall = dave_ralloc(sizeof(UACCall));

	pUACCall->owner_id = owner_id;
	pUACCall->call = NULL;

	kv_add_key_ptr(_uac_class->phone_number_kv, phone_number, pUACCall);

	return pUACCall;
}

void
uac_main_setup_call(UACCall *pUACCall, SIPCall *pCall)
{
	pUACCall->call = pCall;

	pCall->user_ptr = pUACCall;
}

UACCall *
uac_main_inq_phone_number(s8 *phone_number)
{
	return (UACCall *)kv_inq_key_ptr(_uac_class->phone_number_kv, phone_number);
}

void
uac_main_del_call(s8 *phone_number)
{
	UACCall *pUACCall;
	s8 release_timer_name[64];

	pUACCall = kv_del_key_ptr(_uac_class->phone_number_kv, phone_number);
	if(pUACCall != NULL)
	{
		UACLOG("phone_number:%s", phone_number);

		dave_snprintf(release_timer_name, sizeof(release_timer_name), "UACCALLD%lx", pUACCall);
		base_timer_param_creat(release_timer_name, _uac_main_call_delay_release, pUACCall, sizeof(void *), 3000);
	}
}

