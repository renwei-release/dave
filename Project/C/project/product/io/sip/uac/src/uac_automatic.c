/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_osip.h"
#include "uac_main.h"
#include "uac_reg.h"
#include "uac_cfg.h"
#include "uac_log.h"

static dave_bool
_uac_automatic_save_request(UACClass *pClass, osip_message_t *sip)
{
	if(sip->sip_method == NULL)
		return dave_false;

	if(dave_strcmp(sip->sip_method, "REGISTER") == dave_true)
	{
		if(pClass->signal.register_request != NULL)
		{
			UACLOG("has new request!");
			osip_message_free(pClass->signal.register_request);
		}
		pClass->signal.get_register_request_intermediate_state = dave_false;
		pClass->signal.register_request = sip;
	}
	else if(dave_strcmp(sip->sip_method, "INVITE") == dave_true)
	{
		if(pClass->signal.invite_request != NULL)
		{
			UACLOG("has new invite!");
			osip_message_free(pClass->signal.invite_request);
		}
		pClass->signal.get_invite_request_intermediate_state = dave_false;
		pClass->signal.invite_request = sip;
	}
	else if(dave_strcmp(sip->sip_method, "BYE") == dave_true)
	{
		if(pClass->signal.bye_request != NULL)
		{
			UACLOG("has new bye!");
			osip_message_free(pClass->signal.bye_request);
		}
		pClass->signal.get_bye_request_intermediate_state = dave_false;
		pClass->signal.bye_request = sip;
	}
	else if(dave_strcmp(sip->sip_method, "ACK") == dave_true)
	{
		return dave_false;
	}
	else
	{
		UACLOG("unsupport method:%s", sip->sip_method);
		return dave_false;
	}

	return dave_true;
}

static osip_message_t *
_uac_automatic_load_request(UACClass *pClass, osip_message_t *response)
{
	osip_message_t *request;

	if(response->sip_method != NULL)
		return NULL;

	if(response->status_code < 200)
		return NULL;

	if(response->cseq == NULL)
	{
		UACLOG("can't find cseq!");
		return NULL;
	}

	if(dave_strcmp(response->cseq->method, "REGISTER") == dave_true)
	{
		request = pClass->signal.register_request;
		pClass->signal.register_request = NULL;
	}
	else if(dave_strcmp(response->cseq->method, "INVITE") == dave_true)
	{
		request = pClass->signal.invite_request;
		pClass->signal.invite_request = NULL;
	}
	else if(dave_strcmp(response->cseq->method, "BYE") == dave_true)
	{
		request = pClass->signal.bye_request;
		pClass->signal.bye_request = NULL;
	}
	else
	{
		UACLOG("unsupport method:%s", response->cseq->method);
		return dave_false;
	}

	if(request == NULL)
	{
		UACLOG("can't find the method:%s request", response->cseq->method);
	}

	return request;
}

static void
_uac_automatic_is_intermediate_state(UACClass *pClass, osip_message_t *response)
{
	if(response->sip_method != NULL)
		return;

	if(response->status_code >= 200)
		return;

	if(response->cseq == NULL)
	{
		return;
	}

	if(dave_strcmp(response->cseq->method, "REGISTER") == dave_true)
	{
		pClass->signal.get_register_request_intermediate_state = dave_true;
	}
	else if(dave_strcmp(response->cseq->method, "INVITE") == dave_true)
	{
		pClass->signal.get_invite_request_intermediate_state = dave_true;
	}
	else if(dave_strcmp(response->cseq->method, "BYE") == dave_true)
	{
		pClass->signal.get_bye_request_intermediate_state = dave_true;
	}
	else
	{
		UACLOG("unsupport method:%s", response->cseq->method);
	}
}

static void
_uac_automatic_ack(UACClass *pClass, osip_message_t *response)
{
	osip_message_t *request, *sip;

	request = _uac_automatic_load_request(pClass, response);
	if(request == NULL)
	{
		_uac_automatic_is_intermediate_state(pClass, response);
		return;
	}

	sip = osip_ack(request, response);

	uac_class_send(pClass, sip);

	osip_message_free(request);
}

static void
_uac_automatic_status(UACClass *pClass, osip_message_t *request, int status_code)
{
	osip_message_t *sip = NULL;

	if(status_code != 0)
	{
		sip = osip_status(status_code, request);
	}

	if(sip != NULL)
	{
		UACLOG("method:%s/%s status:%d/%s",
			sip->cseq->method, sip->cseq->number,
			sip->status_code, sip->reason_phrase);

		uac_class_send(pClass, sip);
	}
}

// =====================================================================

void
uac_automatic_init(void)
{

}

void
uac_automatic_exit(void)
{

}

dave_bool
uac_automatic_send(UACClass *pClass, osip_message_t *sip)
{
	return _uac_automatic_save_request(pClass, sip);
}

void
uac_automatic_recv(UACClass *pClass, osip_message_t *sip, int status_code)
{
	if(sip->sip_method == NULL)
		_uac_automatic_ack(pClass, sip);
	else
		_uac_automatic_status(pClass, sip, status_code);
}

