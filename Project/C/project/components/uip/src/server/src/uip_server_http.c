/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_http.h"
#include "uip_server_http.h"
#include "uip_log.h"

#define UIP_HTTP_MAX_LINK 3

typedef enum {
	UIPHttpLinkState_idle,
	UIPHttpLinkState_creat,
	UIPHttpLinkState_start,
	UIPHttpLinkState_work,
	UIPHttpLinkState_release,
	UIPHttpLinkState_stop,
} UIPHttpLinkState;

typedef struct {
	UIPHttpLinkState state;

	u16 port;
	HTTPListenType type;
	s8 path[DAVE_PATH_LEN];

	uip_server_recv_fun recv_fun;
} UIPHttpLink;

static TLock _uip_http_link_pv;
static UIPHttpLink _uip_http_link[UIP_HTTP_MAX_LINK];

static void
_uip_server_http_reset(UIPHttpLink *pLink)
{
	if(pLink != NULL)
	{
		pLink->state = UIPHttpLinkState_idle;
		pLink->port = 0;
		pLink->type = ListenMax;
		dave_memset(pLink->path, 0x00, DAVE_PATH_LEN);
		pLink->recv_fun = NULL;
	}
}

static UIPHttpLink *
_uip_server_http_find(ub port, dave_bool find_new)
{
	ub link_index = (ub)(port % UIP_HTTP_MAX_LINK);
	ub safe_counter;

	if(port == 0)
	{
		return NULL;
	}

	for(safe_counter=0; safe_counter<UIP_HTTP_MAX_LINK; safe_counter++)
	{
		if(link_index >= UIP_HTTP_MAX_LINK)
		{
			link_index = 0;
		}

		if(((find_new == dave_true) && (_uip_http_link[link_index].state == UIPHttpLinkState_idle))
			|| (_uip_http_link[link_index].port == port))
		{
			_uip_http_link[link_index].port = port;

			return &_uip_http_link[link_index];
		}

		link_index ++;
	}

	return NULL;
}

static dave_bool
_uip_server_http_listen(UIPHttpLink *pLink)
{
	HTTPListenReq *pReq;
	HTTPListenRsp *pRsp;

	if(pLink->state != UIPHttpLinkState_creat)
	{
		UIPLOG("invalid state:%d, port:%d", pLink->state, pLink->port);
		return dave_false;
	}

	pLink->state = UIPHttpLinkState_start;

	pReq = thread_msg(pReq);
	pReq->listen_port = (ub)pLink->port;
	pReq->rule = LocationMatch_Accurate;
	pReq->type = pLink->type;
	dave_snprintf(pReq->path, DAVE_PATH_LEN, "%s", pLink->path);
	pReq->ptr = pLink;

	pRsp = name_go(HTTP_THREAD_NAME, HTTPMSG_LISTEN_REQ, pReq, HTTPMSG_LISTEN_RSP);

	if(pRsp->ret == RetCode_OK)
	{
		UIPLOG("port:%d listen on:%s success!", pLink->port, pLink->path);
		pLink->state = UIPHttpLinkState_work;
		return dave_true;
	}
	else
	{
		UIPABNOR("%s port:%d", retstr(pRsp->ret), pLink->port);
		_uip_server_http_reset(pLink);
		return dave_false;
	}
}

static void
_uip_server_http_close(UIPHttpLink *pLink)
{
	HTTPCloseReq *pReq;
	HTTPCloseRsp *pRsp;

	if(pLink->state == UIPHttpLinkState_release)
	{
		pLink->state = UIPHttpLinkState_stop;
	}
	else
	{
		UIPLOG("invalid state:%d, port:%d", pLink->state, pLink->port);
		return;
	}

	pReq = thread_msg(pReq);
	pReq->listen_port = pLink->port;
	pReq->ptr = pLink;

	pRsp = name_go(HTTP_THREAD_NAME, HTTPMSG_CLOSE_REQ, pReq, HTTPMSG_CLOSE_RSP);
	if(pRsp->ret == RetCode_OK)
	{
		UIPLOG("port:%d close success! state:%d", pRsp->listen_port, pLink->state);
		_uip_server_http_reset(pLink);
	}
	else
	{
		UIPABNOR("%s", retstr(pRsp->ret));
	}
}

static UIPHttpLink *
_uip_server_http_start_(ub port)
{
	UIPHttpLink *pLink = NULL;
	dave_bool ret = dave_false;

	pLink = _uip_server_http_find(port, dave_false);
	if(pLink == NULL)
	{
		pLink = _uip_server_http_find(port, dave_true);
		ret = dave_true;
	}

	if(ret == dave_false)
	{
		UIPLOG("the port:%d is busy, please _uip_http_server_stop it!", port);
		return NULL;
	}

	if(pLink->state != UIPHttpLinkState_idle)
	{
		UIPLOG("invalid state:%d, port:%d", pLink->state, port);
		return NULL;
	}

	pLink->state = UIPHttpLinkState_creat;

	return pLink;
}

static UIPHttpLink *
_uip_server_http_stop_(ub port)
{
	return _uip_server_http_find(port, dave_false);
}

static dave_bool
_uip_server_http_start(ub port, HTTPListenType type, s8 *path, uip_server_recv_fun recv_fun)
{
	UIPHttpLink *pLink;

	pLink = _uip_server_http_start_(port);
	if(pLink == NULL)
	{
		UIPABNOR("start port:%d path:%s failed!", port, path);
		return dave_false;
	}

	pLink->state = UIPHttpLinkState_creat;
	pLink->port = port;
	pLink->type = type;
	dave_strcpy(pLink->path, path, DAVE_PATH_LEN);
	pLink->recv_fun = recv_fun;

	return _uip_server_http_listen(pLink);
}

static void
_uip_server_http_stop(ub port)
{
	UIPHttpLink *pLink;

	pLink = _uip_server_http_stop_(port);
	if(pLink == NULL)
	{
		return;
	}

	pLink->state = UIPHttpLinkState_release;

	_uip_server_http_close(pLink);
}

static void
_uip_server_http_init(void)
{
	ub link_index;

	for(link_index=0; link_index<UIP_HTTP_MAX_LINK; link_index++)
	{
		_uip_server_http_reset(&_uip_http_link[link_index]);
	}
}

static void
_uip_server_http_exit(void)
{
	ub link_index;

	for(link_index=0; link_index<UIP_HTTP_MAX_LINK; link_index++)
	{
		if(_uip_http_link[link_index].state == UIPHttpLinkState_work)
		{
			_uip_server_http_stop(_uip_http_link[link_index].port);
		}
	}
}

// =====================================================================

void
uip_server_http_init(void)
{
	t_lock_reset(&_uip_http_link_pv);

	SAFECODEv1(_uip_http_link_pv, { _uip_server_http_init(); } );
}

void
uip_server_http_exit(void)
{
	SAFECODEv1(_uip_http_link_pv, { _uip_server_http_exit(); } );
}

dave_bool
uip_server_http_start(ub port, HTTPListenType type, s8 *path, uip_server_recv_fun recv_fun)
{
	dave_bool ret;

	SAFECODEv1(_uip_http_link_pv, { ret = _uip_server_http_start(port, type, path, recv_fun); } );

	return ret;
}

void
uip_server_http_stop(ub port)
{
	SAFECODEv1(_uip_http_link_pv, { _uip_server_http_stop(port); } );
}

uip_server_recv_fun
uip_server_http_recv_fun(ub port)
{
	UIPHttpLink *pLink;

	pLink = _uip_server_http_find(port, dave_false);
	if(pLink == NULL)
	{
		return NULL;
	}

	return pLink->recv_fun;
}

