/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_http.h"
#include "dave_os.h"
#include "http_recv.h"
#include "http_recv_param.h"
#include "http_fastcgi.h"
#include "http_param.h"
#include "http_tools.h"
#include "http_log.h"

static void
_http_test_recv_req(MSGBODY *msg)
{
	HTTPRecvReq *pReq = (HTTPRecvReq *)(msg->msg_body);
	HTTPRecvRsp *pRsp = thread_msg(pRsp);

	HTTPLOG("%s:%d method:%d type:%d", pReq->remote_address, pReq->remote_port, pReq->method, pReq->content_type);

	pRsp->ret = RetCode_OK;
	pRsp->content_type = pReq->content_type;
	pRsp->content = pReq->content;
	pRsp->local_creat_time = dave_os_time_us();
	pRsp->ptr = pReq->ptr;

	write_msg(self(), HTTPMSG_RECV_RSP, pRsp);
}

static void
_http_test_listen_rsp(MSGBODY *ptr)
{
	HTTPListenRsp *pRsp = (HTTPListenRsp *)(ptr->msg_body);

	HTTPLOG("%s", retstr(pRsp->ret));

	if(pRsp->ret == RetCode_OK)
	{
		reg_msg(HTTPMSG_RECV_REQ, _http_test_recv_req);
	}
	else
	{
		unreg_msg(HTTPMSG_RECV_REQ);
	}
}

static ub
_http_test_listen_req(s8 *msg, ub msg_len)
{
	HTTPListenReq *pReq = thread_msg(pReq);

	pReq->listen_port = 1816;
	pReq->type = ListenHttp;
	pReq->rule = LocationMatch_Accurate;
	dave_snprintf(pReq->path, DAVE_PATH_LEN, "/jegom/jtp");

	write_event(self(), HTTPMSG_LISTEN_REQ, pReq, HTTPMSG_LISTEN_RSP, _http_test_listen_rsp);

	return dave_snprintf(msg, msg_len, "ok");
}

// =====================================================================

void
http_test(ThreadId src, DebugReq *pReq)
{
	DebugRsp *pRsp = thread_msg(pRsp);

	if(pReq->msg[0] == 'l')
	{
		_http_test_listen_req(pRsp->msg, sizeof(pRsp->msg));
	}

	pRsp->ptr = pReq->ptr;

	write_msg(src, MSGID_DEBUG_REQ, pRsp);
}

