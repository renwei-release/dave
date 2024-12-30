/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_osip.h"
#include "uac_class.h"
#include "uac_main.h"
#include "uac_cfg.h"
#include "uac_log.h"

static ub
_uac_reg_recv(void *pClass, osip_message_t *pRecv)
{
	UACLOG("status:%d %s", pRecv->status_code, pRecv->reason_phrase);

	return SIP_OK;
}

static void
_uac_reg_send(UACClass *pClass)
{
	osip_message_t *sip;

	SAFECODEv1(pClass->signal.request_pv, {
		sip = osip_register(
			pClass->signal.server_ip, pClass->signal.server_port, pClass->signal.username,
			pClass->signal.local_ip, pClass->signal.local_port,
			pClass->signal.cseq_number ++);
	});

	UACLOG("server:%s:%s local:%s:%s",
		pClass->signal.server_ip, pClass->signal.server_port,
		pClass->signal.local_ip, pClass->signal.local_port);

	uac_class_reg_reg(pClass, _uac_reg_recv);

	uac_class_send(pClass, sip);
}

// =====================================================================

void
uac_reg_init(void)
{

}

void
uac_reg_exit(void)
{

}

void
uac_reg(UACClass *pClass)
{
	if(pClass == NULL)
		return;

	_uac_reg_send(pClass);
}

