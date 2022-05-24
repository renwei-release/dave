/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_http.h"
#include "http_recv.h"
#include "http_recv_param.h"
#include "http_fastcgi.h"
#include "http_tools.h"
#include "http_recv_listen.h"
#include "http_recv_data.h"
#include "http_log.h"

// =====================================================================

void
http_recv_init(void)
{
	http_recv_listen_init();

	http_recv_data_init();
}

void
http_recv_exit(void)
{
	http_recv_data_exit();

	http_recv_listen_exit();
}

void
http_recv_listen(ThreadId src, HTTPListenReq *pReq)
{
	HTTPListenRsp *pRsp = thread_reset_msg(pRsp);

	pRsp->ret = http_recv_listen_action(src, pReq->listen_port, pReq->type, pReq->rule, pReq->path);
	pRsp->listen_port = pReq->listen_port;
	dave_strcpy(pRsp->path, pReq->path, sizeof(pRsp->path));
	pRsp->ptr = pReq->ptr;

	HTTPLOG("%s listen:%d %s", thread_name(src), pReq->listen_port, retstr(pRsp->ret));

	id_msg(src, HTTPMSG_LISTEN_RSP, pRsp);
}

void
http_recv_close(ThreadId src, HTTPCloseReq *pReq)
{
	HTTPCloseRsp *pRsp = thread_reset_msg(pRsp);

	pRsp->ret = http_recv_listen_close(src, pReq->listen_port);
	pRsp->listen_port = pReq->listen_port;
	dave_strcpy(pRsp->path, pReq->path, sizeof(pRsp->path));
	pRsp->ptr = pReq->ptr;

	HTTPLOG("%s close:%d %s", thread_name(src), pReq->listen_port, retstr(pRsp->ret));

	id_msg(src, HTTPMSG_CLOSE_RSP, pRsp);
}

