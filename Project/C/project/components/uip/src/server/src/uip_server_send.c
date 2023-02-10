/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_3rdparty.h"
#include "dave_bdata.h"
#include "uip_server_register.h"
#include "uip_server_http.h"
#include "uip_server_monitor.h"
#include "uip_parsing.h"
#include "uip_tools.h"
#include "uip_log.h"

static void
_uip_server_record(UIPStack *pRecvStack, UIPStack *pSendStack)
{
	void *recv = uip_encode(pRecvStack);
	void *send = uip_encode(pSendStack);
	void *json = dave_json_malloc();

	dave_json_add_object(json, "recv", recv);
	dave_json_add_object(json, "send", send);

	BDATAJSON("UIP", json);
}

static UIPStack *
_uip_server_sendstack(UIPStack *pRecvStack, UIPDataRecvRsp *pRsp)
{
	UIPStack *pSendStack;

	pSendStack = uip_clone(pRecvStack);
	pSendStack->head.req_flag = dave_false;
	pSendStack->head.rsp_code = pRsp->ret;
	t_time_get_date(&(pSendStack->head.date));

	if(pRsp->data != NULL)
	{
		pSendStack->body.pJson = dave_string_to_json((s8 *)(pRsp->data->payload), (sb)(pRsp->data->len));
	}
	else
	{
		pSendStack->body.pJson = NULL;
	}

	return pSendStack;
}

// =====================================================================

void *
uip_server_send(UIPStack *pRecvStack, UIPDataRecvRsp *pRsp)
{
	UIPStack *pSendStack;
	void *pJson;

	if(dave_strcmp(pRecvStack->head.method, pRsp->method) == dave_false)
	{
		UIPABNOR("method:%s/%s mismatch!", pRecvStack->head.method, pRsp->method);
	}

	pSendStack = _uip_server_sendstack(pRecvStack, pRsp);
	pJson = uip_encode(pSendStack);

	_uip_server_record(pRecvStack, pSendStack);

	if(pSendStack->body.pJson != NULL)
	{
		dave_json_free(pSendStack->body.pJson);
		pSendStack->body.pJson = NULL;
	}

	uip_free(pSendStack);

	return pJson;
}

