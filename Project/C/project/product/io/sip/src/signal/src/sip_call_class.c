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
#include "sip_log.h"

// =====================================================================

SIPCall *
sip_call_build(SIPSignal *pSignal, s8 *call_data)
{
	SIPCall *pCall = (SIPCall *)dave_ralloc(sizeof(SIPCall));

	kv_add_key_ptr(pSignal->call_data_kv, call_data, pCall);

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
	if(kv_del_key_ptr(pSignal->call_id_kv, osip_call_id_get_number(pCall->call_id)) != pCall)
	{
		SIPABNOR("Arithmetic error! call_id:%s",
			osip_call_id_get_number(pCall->call_id));
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

	dave_memset(pCall, 0x00, sizeof(SIPCall));

	dave_free(pCall);
}

