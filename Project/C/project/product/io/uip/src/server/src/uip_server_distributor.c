/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_http.h"
#include "uip_server_distributor.h"
#include "uip_log.h"

typedef struct {
	s8 path[DAVE_PATH_LEN];
	uip_server_recv_fun recv_fun;
} UIPDistributorLink;

static void *_uip_distributor_link_kv = NULL;

static void
_uip_server_distributor_start(s8 *path)
{
	HTTPListenReq *pReq = thread_reset_msg(pReq);
	HTTPListenRsp *pRsp;

	dave_snprintf(pReq->path, DAVE_PATH_LEN, "%s", path);

	pRsp = name_co(DISTRIBUTOR_THREAD_NAME, HTTPMSG_LISTEN_REQ, pReq, HTTPMSG_LISTEN_RSP);
	if(pRsp->ret == RetCode_OK)
	{
		UIPLOG("port:%d listen on:%s success!", pRsp->listen_port, pRsp->path);
	}
	else
	{
		UIPABNOR("%s port:%d listen on:%s failed!", retstr(pRsp->ret), pRsp->listen_port, pRsp->path);
	}
}

static void
_uip_server_distributor_stop(s8 *path)
{
	HTTPCloseReq *pReq = thread_reset_msg(pReq);
	HTTPCloseRsp *pRsp;

	dave_snprintf(pReq->path, DAVE_PATH_LEN, "%s", path);

	pRsp = name_co(DISTRIBUTOR_THREAD_NAME, HTTPMSG_CLOSE_REQ, pReq, HTTPMSG_CLOSE_RSP);
	if(pRsp->ret == RetCode_OK)
	{
		UIPLOG("port:%d close success!", pRsp->listen_port);
	}
	else
	{
		UIPABNOR("%s", retstr(pRsp->ret));
	}
}

static UIPDistributorLink *
_uip_server_distributor_inq(s8 *path)
{
	return kv_inq_key_ptr(_uip_distributor_link_kv, path);
}

static dave_bool
_uip_server_distributor_add(s8 *path, uip_server_recv_fun recv_fun)
{
	UIPDistributorLink *pLink = dave_malloc(sizeof(UIPDistributorLink));
	dave_bool ret;

	dave_strcpy(pLink->path, path, sizeof(pLink->path));
	pLink->recv_fun = recv_fun;

	ret = kv_add_key_ptr(_uip_distributor_link_kv, path, pLink);
	if(ret == dave_false)
	{
		dave_free(pLink);
	}

	return ret;
}

static dave_bool
_uip_server_distributor_del(s8 *path)
{
	UIPDistributorLink *pLink = kv_del_key_ptr(_uip_distributor_link_kv, path);

	if(pLink != NULL)
	{
		dave_free(pLink);
		return dave_true;
	}

	return dave_false;
}

static RetCode
_uip_server_distributor_kv_recycle(void *ramkv, s8 *key)
{
	s8 *path = key;

	if(_uip_server_distributor_del(path) == dave_false)
		return RetCode_empty_data;

	_uip_server_distributor_stop(path);

	return RetCode_OK;
}

// =====================================================================

void
uip_server_distributor_init(void)
{
	_uip_distributor_link_kv = kv_malloc("usdkv", 0, NULL);
}

void
uip_server_distributor_exit(void)
{
	kv_free(_uip_distributor_link_kv, _uip_server_distributor_kv_recycle);
}

dave_bool
uip_server_distributor_start(s8 *path_user, uip_server_recv_fun recv_fun)
{
	s8 path[128];
	UIPDistributorLink *pLink;

	dave_strcpy(path, path_user, sizeof(path));

	t_stdio_remove_the_char_on_frist(path, '/');
	t_stdio_remove_the_char_on_frist(path, '\\');

	pLink = _uip_server_distributor_inq(path);
	if(pLink != NULL)
		return dave_true;

	if(_uip_server_distributor_add(path, recv_fun) == dave_false)
		return dave_false;

	_uip_server_distributor_start(path);

	return dave_true;
}

void
uip_server_distributor_stop(s8 *path_user)
{
	s8 path[128];

	dave_strcpy(path, path_user, sizeof(path));

	t_stdio_remove_the_char_on_frist(path, '/');
	t_stdio_remove_the_char_on_frist(path, '\\');

	if(_uip_server_distributor_del(path) == dave_true)
	{
		_uip_server_distributor_stop(path);
	}
}

uip_server_recv_fun
uip_server_distributor_recv_fun(s8 *path)
{
	UIPDistributorLink *pLink = _uip_server_distributor_inq(path);

	if(pLink == NULL)
		return NULL;

	return pLink->recv_fun;
}

