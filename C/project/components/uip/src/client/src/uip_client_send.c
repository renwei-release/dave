/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_3rdparty.h"
#include "uip_client_send.h"
#include "uip_channel.h"
#include "uip_parsing.h"
#include "uip_msg.h"
#include "uip_log.h"

static ThreadId _http_thread_id = INVALID_THREAD_ID;
static ub _uip_client_send_serial = 0;

static void _uip_client_send_rsp(ThreadId dst, RetCode ret, s8 *method, MBUF *data, void *ptr, UIPStack *pStack);

static inline ub
__uip_client_send_serial__(void)
{
	ub serial;

	t_lock_spin(NULL);
	serial = _uip_client_send_serial ++;
	t_unlock_spin(NULL);

	return serial;
}

static void
_uip_client_post_rsp(MSGBODY *ptr)
{
	HTTPPostRsp *pRsp = (HTTPPostRsp *)(ptr->msg_body);
	UIPStack *pStack = (UIPStack *)(pRsp->ptr);

	_uip_client_send_rsp(pStack->src, pRsp->ret, pStack->head.method, pRsp->content, pStack->ptr, pStack);
}

static dave_bool
_uip_client_post_req(UIPStack *pStack)
{
	HTTPPostReq *pReq = thread_msg(pReq);
	void *pPostJson;

	dave_strcpy(pReq->url, pStack->remote_address, DAVE_URL_LEN);

	pPostJson = uip_encode(pStack);

	pReq->head[0].key[0] = '\0';
	pReq->head[0].value[0] = '\0';
	pReq->content_type = HttpContentType_json;
	pReq->content = dave_json_to_mbuf(pPostJson);
	pReq->ptr = pStack;

	dave_json_free(pPostJson);

	if(_http_thread_id == INVALID_THREAD_ID)
	{
		_http_thread_id = thread_id(POST_THREAD_NAME);
	}

	return id_event(_http_thread_id, HTTPMSG_POST_REQ, pReq, HTTPMSG_POST_RSP, _uip_client_post_rsp);
}

static void
_uip_client_send_rsp(ThreadId dst, RetCode ret, s8 *method, MBUF *data, void *ptr, UIPStack *pStack)
{
	UIPDataSendRsp *pRsp = thread_msg(pRsp);

	pRsp->ret = ret;
	dave_strcpy(pRsp->method, method, sizeof(pRsp->method));
	pRsp->data = data;
	pRsp->ptr = ptr;

	uip_free(pStack);

	id_msg(dst, UIP_DATA_SEND_RSP, pRsp);
}

static RetCode
_uip_client_send_req(ThreadId src, UIPDataSendReq *pReq)
{
	s8 *auth_key_str;
	void *pDataJson;
	UIPStack *pStack;

	auth_key_str = uip_channel_inq(pReq->channel);
	if(auth_key_str == NULL)
	{
		return RetCode_Invalid_channel;
	}

	pDataJson = dave_string_to_json((s8 *)(pReq->data->payload), pReq->data->len);
	if(pDataJson == NULL)
	{
		return RetCode_Invalid_data;
	}

	dave_mfree(pReq->data); pReq->data = NULL;

	pStack = uip_malloc();

	dave_strcpy(pStack->head.method, pReq->method, sizeof(pStack->head.method));
	dave_strcpy(pStack->head.channel, pReq->channel, sizeof(pStack->head.channel));
	dave_strcpy(pStack->head.auth_key_str, auth_key_str, sizeof(pStack->head.auth_key_str));
	t_time_get_date(&(pStack->head.date));
	pStack->head.serial = __uip_client_send_serial__();
	pStack->head.customer_head = pReq->customer_head;

	pStack->body.pJson = pDataJson;
	pStack->body.customer_body = pReq->customer_body;

	pStack->src = src;
	pStack->ptr = pReq->ptr;

	dave_strcpy(pStack->remote_address, pReq->remote_url, sizeof(pStack->remote_address));
	pStack->uip_type = UIPType_uip;

	pStack->auto_release_json = pDataJson;

	_uip_client_post_req(pStack);

	return RetCode_OK;
}

// =====================================================================

void
uip_client_send(ThreadId src, UIPDataSendReq *pReq)
{
	RetCode ret;

	ret = _uip_client_send_req(src, pReq);
	if(ret != RetCode_OK)
	{
		UIPLOG("src:%s method:%s ret:%s", thread_name(src), pReq->method, retstr(ret));

		_uip_client_send_rsp(src, ret, pReq->method, NULL, pReq->ptr, NULL);

		dave_mfree(pReq->customer_head); pReq->customer_head = NULL;
		dave_mfree(pReq->customer_body); pReq->customer_body = NULL;
		dave_mfree(pReq->data); pReq->data = NULL;
	}
}

