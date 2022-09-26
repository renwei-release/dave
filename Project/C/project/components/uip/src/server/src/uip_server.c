/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_3rdparty.h"
#include "uip_channel.h"
#include "uip_server_register.h"
#include "uip_server_monitor.h"
#include "uip_server_http.h"
#include "uip_server_distributor.h"
#include "uip_server_send.h"
#include "uip_server_recv.h"
#include "uip_server_wechat.h"
#include "uip_debug.h"
#include "uip_parsing.h"
#include "uip_tools.h"
#include "uip_log.h"

static void _uip_server_http_rsp(ThreadId dst, RetCode ret, void *ptr, void *pJson);
static void _uip_server_uip_rsp(UIPStack *pRecvStack, UIPDataRecvRsp *pRsp);

static inline uip_server_recv_fun
_uip_server_recv_fun(HTTPRecvReq *pReq)
{
	uip_server_recv_fun recv_fun;

	recv_fun = uip_server_http_recv_fun(pReq->listen_port);
	if(recv_fun == NULL)
	{
		recv_fun = uip_server_distributor_recv_fun(pReq->remote_address);
	}

	return recv_fun;
}

static void
_uip_server_stack_rsp(ThreadId src, UIPDataRecvRsp *pRsp)
{
	UIPStack *pRecvStack;

	if(pRsp->ret != RetCode_OK)
	{
		UIPLTRACE(60,1,"method:%s ret:%s", pRsp->method, retstr(pRsp->ret));
	}

	pRecvStack = uip_server_monitor_free(pRsp->ptr);

	if(pRecvStack != NULL)
	{
		_uip_server_uip_rsp(pRecvStack, pRsp);
	}
	else
	{
		UIPLOG("ret:%s method:%s data:%s",
			retstr(pRsp->ret), pRsp->method,
			dave_mptr(pRsp->data));
	}

	dave_mfree(pRsp->data);
}

static RetCode
_uip_server_stack_req(UIPStack *pRecvStack)
{
	UIPDataRecvReq *pReq = thread_msg(pReq);

	dave_strcpy(pReq->remote_address, pRecvStack->remote_address, DAVE_URL_LEN);
	pReq->remote_port = pRecvStack->remote_port;
	pReq->uip_type = pRecvStack->uip_type;
	dave_strcpy(pReq->channel, pRecvStack->head.channel, DAVE_NORMAL_NAME_LEN);
	dave_strcpy(pReq->method, pRecvStack->head.method, DAVE_UIP_METHOD_MAX_LEN);
	pReq->customer_body = dave_mclone(pRecvStack->body.customer_body);
	pReq->data = dave_json_to_mbuf(pRecvStack->body.pJson);

	pReq->ptr = uip_server_monitor_malloc(pRecvStack);

	name_msg(pRecvStack->register_thread, UIP_DATA_RECV_REQ, pReq);

	return RetCode_OK;
}

static void
_uip_server_uip_rsp(UIPStack *pRecvStack, UIPDataRecvRsp *pRsp)
{
	void *pJson;

	pJson = uip_server_send(pRecvStack, pRsp);
	if(pJson == NULL)
	{
		UIPLOG("method:%s can't send!", pRsp->method);
	}
	else
	{
		_uip_server_http_rsp(pRecvStack->src, pRsp->ret, pRecvStack->ptr, pJson);

		dave_json_free(pJson);
	}

	uip_free(pRecvStack);
}

static RetCode
_uip_server_uip_req(ThreadId src, HTTPRecvReq *pReq, void *pJson)
{
	UIPStack *pRecvStack;
	RetCode ret;

	ret = uip_server_recv(&pRecvStack, src, pReq, pJson);
	if(ret != RetCode_OK)
	{
		return ret;
	}

	return _uip_server_stack_req(pRecvStack);
}

static RetCode
_uip_server_wechat_req(ThreadId src, HTTPRecvReq *pReq, void *pJson)
{
	if(pReq->content == NULL)
	{
		return RetCode_empty_data;
	}

	pJson = uip_server_wechat(pReq->content->payload, pReq->content->len);
	if(pJson != NULL)
	{
		return _uip_server_uip_req(src, pReq, pJson);
	}
	else
	{
		return RetCode_Invalid_data;
	}
}

static void
_uip_server_http_rsp(ThreadId dst, RetCode ret, void *ptr, void *pJson)
{
	HTTPRecvRsp *pRsp = thread_msg(pRsp);

	pRsp->ret = ret;
	pRsp->content_type = HttpContentType_json;
	if(pJson != NULL)
	{
		pRsp->content = dave_json_to_mbuf(pJson);
	}
	else if(ret != RetCode_OK)
	{
		pRsp->content = dave_mmalloc(4096);
		pRsp->content->len = pRsp->content->tot_len = uip_encode_error(pRsp->content->payload, pRsp->content->len, ret);
	}
	else
	{
		pRsp->content = NULL;
	}
	pRsp->local_creat_time = dave_os_time_us();
	pRsp->ptr = ptr;

	id_msg(dst, HTTPMSG_RECV_RSP, pRsp);
}

static void
_uip_server_http_req(ThreadId src, HTTPRecvReq *pReq)
{
	uip_server_recv_fun recv_fun;
	RetCode ret;

	recv_fun = _uip_server_recv_fun(pReq);

	if(recv_fun != NULL)
	{
		ret = recv_fun(src, pReq, NULL);
	}
	else
	{
		ret = RetCode_function_not_supported;
	}

	if(ret != RetCode_OK)
	{
		_uip_server_http_rsp(src, ret, pReq->ptr, NULL);
	}

	dave_mfree(pReq->content);
}

static void
_uip_server_register_req(ThreadId src, UIPRegisterReq *pReq)
{
	ub method_index;
	UIPRegisterRsp *pRsp = thread_msg(pRsp);
	RetCode ret;

	UIPTRACE("%s register method:%s", thread_name(src), pReq->method[0]);

	pRsp->ret = RetCode_OK;
	for(method_index=0; method_index<DAVE_UIP_METHOD_MAX_NUM; method_index++)
	{
		if(pReq->method[method_index][0] != '\0')
		{
			ret = uip_server_register(src, pReq->method[method_index]);
			if(ret != RetCode_OK)
			{
				UIPLOG("invalid ret:%s on method:%s", retstr(ret), pReq->method[method_index]);
				pRsp->ret = ret;
			}
		}

		dave_strcpy(pRsp->method[method_index], pReq->method[method_index], DAVE_UIP_METHOD_MAX_LEN);
	}
	pRsp->ptr = pReq->ptr;

	id_msg(src, UIP_REGISTER_RSP, pRsp);
}

static void
_uip_server_unregister_req(ThreadId src, UIPUnregisterReq *pReq)
{
	ub method_index;
	UIPUnregisterRsp *pRsp = thread_msg(pRsp);
	RetCode ret;

	pRsp->ret = RetCode_OK;
	for(method_index=0; method_index<DAVE_UIP_METHOD_MAX_NUM; method_index++)
	{
		if(pReq->method[method_index][0] != '\0')
		{
			ret = uip_server_unregister(pReq->method[method_index]);
			if(ret != RetCode_OK)
			{
				UIPLOG("invalid ret:%s on method:%s", retstr(ret), pReq->method[method_index]);
				pRsp->ret = ret;
			}
		}

		dave_strcpy(pRsp->method, pReq->method, DAVE_UIP_METHOD_MAX_LEN);
	}
	pRsp->ptr = pReq->ptr;

	id_msg(src, UIP_UNREGISTER_RSP, pRsp);
}

static void
_uip_server_http_start(void)
{
	uip_server_http_start(UIP_SERVER_HTTPs_PORT, ListenHttps, "/uips", _uip_server_uip_req);
	uip_server_http_start(UIP_SERVER_H5_PORT, ListenHttps, "/h5", _uip_server_uip_req);
	uip_server_http_start(UIP_SERVER_WeChat_PORT, ListenHttps, "/wechat", _uip_server_wechat_req);

	uip_server_distributor_start("/uips", _uip_server_uip_req);
	uip_server_distributor_start("/h5", _uip_server_uip_req);
	uip_server_distributor_start("/wechat", _uip_server_wechat_req);
}

static void
_uip_server_http_stop(void)
{
	uip_server_http_exit();

	uip_server_distributor_exit();
}

static void 
_uip_server_local_ready(ThreadLocalReadyMsg *pMsg)
{
	if(dave_strcmp(pMsg->local_thread_name, HTTP_THREAD_NAME) == dave_true)
	{
		_uip_server_http_start();
	}
}

static void
_uip_server_restart(RESTARTREQMSG *pRestart)
{
	if(pRestart->times == 1)
	{
		_uip_server_http_stop();

		uip_server_register_exit();
	}
}

// =====================================================================

void
uip_server_init(MSGBODY *pMsg)
{
	uip_server_monitor_init(_uip_server_http_rsp);

	uip_server_http_init();

	uip_server_distributor_init();

	uip_server_register_init();
}

void
uip_server_main(MSGBODY *pMsg)
{
	switch((sb)(pMsg->msg_id))
	{
		case MSGID_LOCAL_THREAD_READY:
				_uip_server_local_ready((ThreadLocalReadyMsg *)(pMsg->msg_body));
			break;
		case HTTPMSG_RECV_REQ:
				_uip_server_http_req(pMsg->msg_src, (HTTPRecvReq *)(pMsg->msg_body));
			break;
		case UIP_REGISTER_REQ:
				_uip_server_register_req(pMsg->msg_src, (UIPRegisterReq *)(pMsg->msg_body));
			break;
		case UIP_UNREGISTER_REQ:
				_uip_server_unregister_req(pMsg->msg_src, (UIPUnregisterReq *)(pMsg->msg_body));
			break;
		case UIP_DATA_RECV_RSP:
				_uip_server_stack_rsp(pMsg->msg_src, (UIPDataRecvRsp *)(pMsg->msg_body));
			break;
		default:
			break;
	}
}

void
uip_server_exit(MSGBODY *pMsg)
{
	_uip_server_http_stop();

	uip_server_http_exit();

	uip_server_distributor_exit();

	uip_server_monitor_exit();
}

void
uip_server_restart(RESTARTREQMSG *pRestart)
{
	_uip_server_restart(pRestart);
}

ub
uip_server_info(s8 *info_ptr, ub info_len)
{
	return uip_server_register_info(info_ptr, info_len);
}

