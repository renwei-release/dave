/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_osip.h"
#include "sip_log.h"

static dave_bool
_sip_automatic_save_request(SIPSignal *pSignal, osip_message_t *sip)
{
	if(sip->sip_method == NULL)
		return dave_false;

	if(dave_strcmp(sip->sip_method, "REGISTER") == dave_true)
	{
		if(pSignal->register_request != NULL)
		{
			SIPLOG("has new request!");
			osip_message_free(pSignal->register_request);
		}
		pSignal->get_register_request_intermediate_state = dave_false;
		pSignal->register_request = sip;
	}
	else if(dave_strcmp(sip->sip_method, "INVITE") == dave_true)
	{
		if(pSignal->invite_request != NULL)
		{
			SIPLOG("has new invite!");
			osip_message_free(pSignal->invite_request);
		}
		pSignal->get_invite_request_intermediate_state = dave_false;
		pSignal->invite_request = sip;
	}
	else if(dave_strcmp(sip->sip_method, "BYE") == dave_true)
	{
		if(pSignal->bye_request != NULL)
		{
			SIPLOG("has new bye!");
			osip_message_free(pSignal->bye_request);
		}
		pSignal->get_bye_request_intermediate_state = dave_false;
		pSignal->bye_request = sip;
	}
	else if(dave_strcmp(sip->sip_method, "ACK") == dave_true)
	{
		return dave_false;
	}
	else
	{
		SIPLOG("unsupport method:%s", sip->sip_method);
		return dave_false;
	}

	return dave_true;
}

static osip_message_t *
_sip_automatic_load_request(SIPSignal *pSignal, osip_message_t *response)
{
	osip_message_t *request;

	if(response->sip_method != NULL)
		return NULL;

	if(response->status_code < 200)
		return NULL;

	if(response->cseq == NULL)
	{
		SIPLOG("can't find cseq!");
		return NULL;
	}

	if(dave_strcmp(response->cseq->method, "REGISTER") == dave_true)
	{
		request = pSignal->register_request;
		pSignal->register_request = NULL;
	}
	else if(dave_strcmp(response->cseq->method, "INVITE") == dave_true)
	{
		request = pSignal->invite_request;
		pSignal->invite_request = NULL;
	}
	else if(dave_strcmp(response->cseq->method, "BYE") == dave_true)
	{
		request = pSignal->bye_request;
		pSignal->bye_request = NULL;
	}
	else
	{
		SIPLOG("unsupport method:%s", response->cseq->method);
		return dave_false;
	}

	if(request == NULL)
	{
		SIPLOG("can't find the method:%s request", response->cseq->method);
	}

	return request;
}

static void
_sip_automatic_is_intermediate_state(SIPSignal *pSignal, osip_message_t *response)
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
		pSignal->get_register_request_intermediate_state = dave_true;
	}
	else if(dave_strcmp(response->cseq->method, "INVITE") == dave_true)
	{
		pSignal->get_invite_request_intermediate_state = dave_true;
	}
	else if(dave_strcmp(response->cseq->method, "BYE") == dave_true)
	{
		pSignal->get_bye_request_intermediate_state = dave_true;
	}
	else
	{
		SIPLOG("unsupport method:%s", response->cseq->method);
	}
}

static void
_sip_automatic_ack(SIPSignal *pSignal, osip_message_t *response)
{
	osip_message_t *request, *sip;

	request = _sip_automatic_load_request(pSignal, response);
	if(request == NULL)
	{
		_sip_automatic_is_intermediate_state(pSignal, response);
		return;
	}

	sip = osip_ack(request, response);

	sip_signal_send(pSignal, sip);

	osip_message_free(request);
}

static void
_sip_automatic_status(SIPSignal *pSignal, osip_message_t *request, int status_code)
{
	osip_message_t *sip = NULL;

	if(status_code != 0)
	{
		sip = osip_status(status_code, request);
	}

	if(sip != NULL)
	{
		SIPLOG("method:%s/%s status:%d/%s",
			sip->cseq->method, sip->cseq->number,
			sip->status_code, sip->reason_phrase);

		sip_signal_send(pSignal, sip);
	}
}

// =====================================================================

dave_bool
sip_automatic_send(SIPSignal *pSignal, osip_message_t *sip)
{
	return _sip_automatic_save_request(pSignal, sip);
}

void
sip_automatic_recv(SIPSignal *pSignal, osip_message_t *sip, int status_code)
{
	if(sip->sip_method == NULL)
		_sip_automatic_ack(pSignal, sip);
	else
		_sip_automatic_status(pSignal, sip, status_code);
}

