/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_base.h"
#include "dave_tools.h"
#include "sip_signal.h"
#include "sip_reg.h"
#include "sip_call.h"
#include "sip_global_lock.h"
#include "sip_log.h"

static void *_sip_signal_state_kv = NULL;
static void *_sip_call_state_kv = NULL;

static void
_sip_state_signal_resend_request(SIPSignal *pSignal)
{
	if((pSignal->get_register_request_intermediate_state == dave_false) && (pSignal->register_request != NULL))
	{
		SIPLOG("auto resend register request!");

		sip_signal_send(pSignal, NULL, pSignal->register_request);
	}
}

static void
_sip_state_signal_rereg(SIPSignal *pSignal)
{
	ub reg_base_counter = 90;

	if((++ pSignal->reg.reg_timer_counter) % reg_base_counter == 0)
	{
		SIPLOG("re register:%d", pSignal->reg.reg_timer_counter / reg_base_counter);
		sip_reg(pSignal);
	}
}

static void
_sip_state_signal_kv_timer(void *ramkv, s8 *key)
{
	SIPSignal *pSignal = kv_inq_key_ptr(ramkv, key);

	sip_global_lock();

	_sip_state_signal_resend_request(pSignal);
	_sip_state_signal_rereg(pSignal);

	sip_global_unlock();
}


static void
_sip_state_call_resend_request(SIPCall *pCall)
{
	osip_message_t *request = NULL;

	if(pCall == NULL)
		return;

	if((pCall->get_invite_request_intermediate_state == dave_false) && (pCall->invite_request != NULL))
	{
		SIPLOG("auto resend invite request:%d", pCall->counter_request_intermediate_state);

		request = pCall->invite_request;
	}
	if((pCall->get_bye_request_intermediate_state == dave_false) && (pCall->bye_request != NULL))
	{
		SIPLOG("auto resend bye request:%d", pCall->counter_request_intermediate_state);

		request = pCall->bye_request;
	}

	if(request != NULL)
	{
		if((pCall->counter_request_intermediate_state ++) < SIP_MAX_REQUEST_TIME)
		{
			sip_signal_send((SIPSignal *)(pCall->signal), pCall, request);
		}
		else
		{
			SIPLOG("resend times out!");

			sip_end_call(pCall);
		}
	}
}

static void
_sip_state_call_kv_timer(void *ramkv, s8 *key)
{
	SIPCall *pCall = kv_inq_key_ptr(ramkv, key);

	_sip_state_call_resend_request(pCall);
}

// =====================================================================

void
sip_state_init(void)
{
	_sip_signal_state_kv = kv_malloc("sipsignalstatekv", 3, _sip_state_signal_kv_timer);
	_sip_call_state_kv = kv_malloc("sipcallstatekv", 10, _sip_state_call_kv_timer);
}

void
sip_state_exit(void)
{
	if(_sip_signal_state_kv != NULL)
	{
		kv_free(_sip_signal_state_kv, NULL);
		_sip_signal_state_kv = NULL;
	}
	if(_sip_call_state_kv != NULL)
	{
		kv_free(_sip_call_state_kv, NULL);
		_sip_call_state_kv = NULL;
	}
}

void
sip_state_signal_creat(SIPSignal *pSignal)
{
	kv_add_ub_ptr(_sip_signal_state_kv, (ub)pSignal, pSignal);
}

void
sip_state_signal_release(SIPSignal *pSignal)
{
	kv_del_ub_ptr(_sip_signal_state_kv, (ub)pSignal);
}

void
sip_state_call_creat(SIPCall *pCall)
{
	kv_add_ub_ptr(_sip_call_state_kv, (ub)pCall, pCall);
}

void
sip_state_call_release(SIPCall *pCall)
{
	kv_del_ub_ptr(_sip_call_state_kv, (ub)pCall);
}

