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
#include "uip_channel.h"
#include "uip_parsing.h"
#include "uip_log.h"

static void *
_uip_server_input(HTTPRecvReq *pReq, void *pJson)
{
	if((pReq == NULL) && (pJson == NULL))
	{
		pJson = NULL;
	}
	else if((pReq != NULL) && (pJson == NULL))
	{
		pJson = dave_string_to_json(pReq->content->payload, -1);
	}
	else if((pReq != NULL) && (pJson != NULL))
	{
		pJson = pJson;
	}
	else if((pReq == NULL) && (pJson != NULL))
	{
		pJson = pJson;
	}

	return pJson;
}

static UIPType
_uip_server_port_to_io_type(ub listen_port)
{
	if(listen_port == UIP_SERVER_HTTPs_PORT)
	{
		return UIPType_uip;
	}
	else if(listen_port == UIP_SERVER_H5_PORT)
	{
		return UIPType_h5_form;
	}
	else if(listen_port == UIP_SERVER_H5_PORT)
	{
		return UIPType_weichat_form;
	}
	else
	{
		return UIPType_json;
	}
}

static RetCode
_uip_server_recv_check(UIPStack *pStack)
{
	RetCode ret;

	ret = uip_server_register_data(pStack->register_thread, pStack->head.method);
	if(ret != RetCode_OK)
	{
		return ret;
	}

	return uip_channel_verify(pStack->head.channel, pStack->head.auth_key_str);
}

// =====================================================================

RetCode
uip_server_recv(UIPStack **ppRecvStack, ThreadId src, HTTPRecvReq *pReq, void *pJson)
{
	RetCode ret;
	UIPStack *pStack = NULL;

	pJson = _uip_server_input(pReq, pJson);
	if(pJson == NULL)
	{
		ret = RetCode_empty_data;
	}
	else
	{
		pStack = uip_decode(src, pReq->ptr, pJson);

		if(pStack == NULL)
		{
			ret = RetCode_Invalid_data;
		}
		else
		{
			dave_strcpy(pStack->remote_address, pReq->remote_address, DAVE_URL_LEN);
			pStack->remote_port = pReq->remote_port;
			pStack->uip_type = _uip_server_port_to_io_type(pReq->listen_port);

			ret = _uip_server_recv_check(pStack);
		}
	}

	if(ret != RetCode_OK)
	{
		UIPLOG("%s listen_port:%d remote_address:%s remote_port:%d method:%d",
			retstr(ret),
			pReq->listen_port, pReq->remote_address, pReq->remote_port,
			pReq->method);

		if(pStack != NULL)
		{
			uip_free(pStack);
			pStack = NULL;
		}
	}

	*ppRecvStack = pStack;

	return ret;
}

