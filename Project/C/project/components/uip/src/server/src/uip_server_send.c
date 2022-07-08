/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_3rdparty.h"
#include "uip_server_register.h"
#include "uip_server_http.h"
#include "uip_server_monitor.h"
#include "uip_parsing.h"
#include "uip_tools.h"
#include "uip_log.h"

#ifdef LEVEL_PRODUCT_alpha
// #define WRITE_STACK_MESSAGE_ENABLE
#endif

#ifdef WRITE_STACK_MESSAGE_ENABLE

static void
_uip_server_write_stack(UIPStack *pRecvStack, UIPStack *pSendStack)
{
	UIPLOG("method:%s", pRecvStack->head.method);

	uip_write_stack("recv", pRecvStack);
	uip_write_stack("send", pSendStack);
}

#endif

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

	pSendStack = uip_clone(pRecvStack);

	pSendStack->head.req_flag = dave_false;
	pSendStack->head.rsp_code = pRsp->ret;
	dave_strcpy(pSendStack->head.method, pRecvStack->head.method, DAVE_UIP_METHOD_MAX_LEN);
	t_time_get_date(&(pSendStack->head.date));
	pSendStack->head.serial = pRecvStack->head.serial;

	if(pRsp->data != NULL)
	{
		pSendStack->body.pJson = dave_string_to_json((s8 *)(pRsp->data->payload), (sb)(pRsp->data->len));
	}
	else
	{
		pSendStack->body.pJson = NULL;
	}

	pSendStack->src = pRecvStack->src;
	pSendStack->ptr = pRecvStack->ptr;

	pJson = uip_encode(pSendStack);

#ifdef WRITE_STACK_MESSAGE_ENABLE
	_uip_server_write_stack(pRecvStack, pSendStack);
#endif

	if(pSendStack->body.pJson != NULL)
	{
		dave_json_free(pSendStack->body.pJson);
		pSendStack->body.pJson = NULL;
	}

	uip_free(pSendStack);

	return pJson;
}

