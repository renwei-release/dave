/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_osip.h"
#include "sip_signal.h"
#include "sip_state.h"
#include "sip_log.h"

static void
_sip_call_delay_release(TIMERID timer_id, ub thread_index, void *param_ptr)
{
	SIPCall *pCall = (SIPCall *)param_ptr;
	SIPSignal *pSignal = (SIPSignal *)(pCall->signal);

	SIPLOG("pSignal:%lx pCall:%lx", pSignal, pCall);

	base_timer_die(timer_id);

	if(pSignal == NULL)
		return;

	if((~ pCall->magic_1) != pCall->magic_2)
		return;

	if(kv_del_key_ptr(pSignal->call_id_kv, osip_call_id_get_number(pCall->call_id)) != pCall)
	{
		SIPABNOR("Arithmetic error! call_id:%s",
			osip_call_id_get_number(pCall->call_id));
		return;
	}

	if(kv_del_key_ptr(pSignal->call_data_kv, pCall->call_data) != pCall)
	{
		SIPABNOR("Arithmetic error! call_data:%s", pCall->call_data);
	}

	if(pCall->call_id != NULL)
	{
		osip_call_id_free(pCall->call_id);
		pCall->call_id = NULL;
	}

	if(pCall->from != NULL)
	{
		osip_from_free(pCall->from);
		pCall->from = NULL;
	}

	if(pCall->to != NULL)
	{
		osip_to_free(pCall->to);
		pCall->to = NULL;
	}

	if(pCall->cseq != NULL)
	{
		osip_cseq_free(pCall->cseq);
		pCall->cseq = NULL;
	}

	if(pCall->rtp != NULL)
	{
		dave_rtp_release(pCall->rtp);
		pCall->rtp = NULL;
	}

	if(pCall->invite_request != NULL)
	{
		osip_message_free(pCall->invite_request);
		pCall->invite_request = NULL;
	}

	if(pCall->bye_request != NULL)
	{
		osip_message_free(pCall->bye_request);
		pCall->bye_request = NULL;
	}

	dave_memset(pCall, 0x00, sizeof(SIPCall));

	dave_free(pCall);
}

// =====================================================================

SIPCall *
sip_call_build(SIPSignal *pSignal, s8 *call_data)
{
	SIPCall *pCall = (SIPCall *)dave_ralloc(sizeof(SIPCall));

	pCall->magic_1 = t_rand();
	pCall->magic_2 = (~ pCall->magic_1);

	pCall->counter_request_intermediate_state = 0;
	pCall->get_invite_request_intermediate_state = dave_false;
	pCall->invite_request = NULL;
	pCall->get_bye_request_intermediate_state = dave_false;
	pCall->bye_request = NULL;

	pCall->signal = pSignal;

	kv_add_key_ptr(pSignal->call_data_kv, call_data, pCall);

	sip_state_call_creat(pCall);

	return pCall;
}

SIPCall *
sip_call_id_query(SIPSignal *pSignal, s8 *call_id)
{
	if(call_id == NULL)
		return NULL;

	return (SIPCall *)kv_inq_key_ptr(pSignal->call_id_kv, call_id);
}

SIPCall *
sip_call_index_query(SIPSignal *pSignal, ub index)
{
	return (SIPCall *)kv_index_key_ptr(pSignal->call_id_kv, index);
}

SIPCall *
sip_call_data_query(SIPSignal *pSignal, s8 *call_data)
{
	if(call_data == NULL)
		return NULL;

	return (SIPCall *)kv_inq_key_ptr(pSignal->call_data_kv, call_data);
}

void
sip_call_creat(SIPSignal *pSignal, s8 *call_id, SIPCall *pCall)
{
	kv_add_key_ptr(pSignal->call_id_kv, call_id, pCall);

	pCall->signal = pSignal;
}

void
sip_call_release(SIPSignal *pSignal, SIPCall *pCall)
{
	s8 release_timer_name[64];

	sip_state_call_release(pCall);

	dave_snprintf(release_timer_name, sizeof(release_timer_name), "SIPCALLD-%lx", pCall);
	base_timer_param_creat(release_timer_name, _sip_call_delay_release, pCall, sizeof(void *), 3000);
}

