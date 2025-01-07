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
#include "sip_global_lock.h"
#include "sip_log.h"

static void *_sip_state_kv = NULL;

static void
_sip_state_resend_request(SIPSignal *pSignal)
{
	if((pSignal->get_register_request_intermediate_state == dave_false) && (pSignal->register_request != NULL))
	{
		SIPLOG("auto resend register request!");

		sip_signal_send(pSignal, pSignal->register_request);
	}

	if((pSignal->get_invite_request_intermediate_state == dave_false) && (pSignal->invite_request != NULL))
	{
		SIPLOG("auto resend invite request!");

		sip_signal_send(pSignal, pSignal->invite_request);
	}

	if((pSignal->get_bye_request_intermediate_state == dave_false) && (pSignal->bye_request != NULL))
	{
		SIPLOG("auto resend bye request!");

		sip_signal_send(pSignal, pSignal->bye_request);
	}
}

static void
_sip_state_rereg(SIPSignal *pSignal)
{
	ub reg_base_counter = 90;

	if((++ pSignal->reg.reg_timer_counter) % reg_base_counter == 0)
	{
		SIPLOG("re register:%d", pSignal->reg.reg_timer_counter / reg_base_counter);
		sip_reg(pSignal);
	}
}

static void
_sip_state_kv_timer(void *ramkv, s8 *key)
{
	SIPSignal *pSignal = kv_inq_key_ptr(ramkv, key);

	sip_global_lock();

	_sip_state_resend_request(pSignal);
	_sip_state_rereg(pSignal);

	sip_global_unlock();
}

// =====================================================================

void
sip_state_init(void)
{
	_sip_state_kv = kv_malloc("sipstatekv", 3, _sip_state_kv_timer);
}

void
sip_state_exit(void)
{
	if(_sip_state_kv != NULL)
	{
		kv_free(_sip_state_kv, NULL);
		_sip_state_kv = NULL;
	}
}

void
sip_state_creat(SIPSignal *pSignal)
{
	kv_add_ub_ptr(_sip_state_kv, (ub)pSignal, pSignal);
}

void
sip_state_release(SIPSignal *pSignal)
{
	kv_del_ub_ptr(_sip_state_kv, (ub)pSignal);
}

