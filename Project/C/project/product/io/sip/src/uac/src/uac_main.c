/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_base.h"
#include "dave_tools.h"
#include "sip_signal.h"
#include "sip_call.h"
#include "uac_main.h"
#include "uac_rtp_buffer.h"
#include "uac_cfg.h"
#include "uac_call.h"
#include "uac_log.h"

UACClass *_uac_class = NULL;

static RetCode _uac_main_call_recycle(void *ramkv, s8 *key);

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

static UACCall *
_uac_main_call_build(ThreadId owner_id, s8 *phone_number)
{
	UACCall *pUACCall = dave_ralloc(sizeof(UACCall));

	pUACCall->owner_id = owner_id;
	dave_strcpy(pUACCall->phone_number, phone_number, sizeof(pUACCall->phone_number));
	uac_rtp_buffer_init(&(pUACCall->rtp_buffer));
	pUACCall->call = NULL;

	return pUACCall;
}

static void
_uac_main_call_release(UACCall *pUACCall)
{
	uac_rtp_buffer_exit(&(pUACCall->rtp_buffer));

	pUACCall->call = NULL;

	dave_free(pUACCall);
}

static RetCode
_uac_main_call_recycle(void *ramkv, s8 *key)
{
	UACCall *pUACCall = kv_del_key_ptr(ramkv, key);

	if(pUACCall == NULL)
		return RetCode_empty_data;

	if((pUACCall->call != NULL) && (pUACCall->call->signal != NULL))
	{
		sip_bye((SIPSignal *)(pUACCall->call->signal), pUACCall->phone_number);
	}

	_uac_main_call_release(pUACCall);

	return RetCode_OK;
}

static void
_uac_main_call_delay_release(TIMERID timer_id, ub thread_index, void *param_ptr)
{
	UACCall *pUACCall = (UACCall *)param_ptr;

	_uac_main_call_release(pUACCall);

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

	if(uac_cfg_sbc_server() != NULL)
	{
		base_timer_creat("umainbooting", _uac_main_booting, 5000);
	}
}

void
uac_main_exit(void)
{
	_uac_main_exit();
}

SIPSignal *
uac_main_signal(void)
{
	if(_uac_class == NULL)
		return NULL;

	return _uac_class->signal;
}

UACCall *
uac_main_build_call(ThreadId owner_id, s8 *phone_number)
{
	if(_uac_class == NULL)
		return NULL;

	UACCall *pUACCall = _uac_main_call_build(owner_id, phone_number);

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
	if(_uac_class == NULL)
		return NULL;

	return (UACCall *)kv_inq_key_ptr(_uac_class->phone_number_kv, phone_number);
}

void
uac_main_del_call(s8 *phone_number)
{
	UACCall *pUACCall;
	s8 release_timer_name[64];

	if(_uac_class == NULL)
		return;

	pUACCall = kv_del_key_ptr(_uac_class->phone_number_kv, phone_number);
	if(pUACCall != NULL)
	{
		dave_snprintf(release_timer_name, sizeof(release_timer_name), "UACCALLD%lx", pUACCall);
		base_timer_param_creat(release_timer_name, _uac_main_call_delay_release, pUACCall, sizeof(void *), 3000);
	}
}

void
uac_main_del_owner_id_all_call(ThreadId owner_id)
{
	UACCall *pUACCall;
	ub call_index;
	s8 call_id[128];

	if(_uac_class == NULL)
		return;

	for(call_index=0; call_index<9999999999; call_index++)
	{
		pUACCall = (UACCall *)kv_index_key_ptr(_uac_class->phone_number_kv, call_index);
		if(pUACCall == NULL)
		{
			UACDEBUG("call_index:%d", call_index);
			break;
		}

		UACLOG("owner_id:%lx/%lx", pUACCall->owner_id, owner_id);

		if((pUACCall->owner_id & 0x0000ffffffffffff) == (owner_id & 0x0000ffffffffffff))
		{
			uac_bye(call_id, sizeof(call_id), owner_id, pUACCall->phone_number);
		}
	}
}

