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

static ub
_sip_reg_recv(void *signal, osip_message_t *pRecv)
{
	SIPLOG("status:%d %s", pRecv->status_code, pRecv->reason_phrase);

	return SIP_OK;
}

static void
_sip_reg_send(SIPSignal *pSignal)
{
	osip_message_t *sip;

	SAFECODEv1(pSignal->request_pv, {
		sip = osip_register(
			pSignal->server_ip, pSignal->server_port, pSignal->username,
			pSignal->local_ip, pSignal->local_port,
			pSignal->cseq_number ++);
	});

	SIPLOG("server:%s:%s local:%s:%s",
		pSignal->server_ip, pSignal->server_port,
		pSignal->local_ip, pSignal->local_port);

	sip_signal_reg_reg(pSignal, _sip_reg_recv, pSignal);

	sip_signal_send(pSignal, NULL, sip);
}

// =====================================================================

void
sip_reg(SIPSignal *pSignal)
{
	if(pSignal == NULL)
	{
		SIPABNOR("pSignal is NULL!");
		return;
	}

	_sip_reg_send(pSignal);
}

