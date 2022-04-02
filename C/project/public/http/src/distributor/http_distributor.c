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
#include "http_distributor.h"
#include "http_tools.h"
#include "http_test.h"
#include "http_log.h"

#define HTTPS_DISTRIBUTOR_SERVER_PORT 1823
#define HTTP_DISTRIBUTOR_SERVER_PORT 1824
#define DISTRIBUTOR_THREAD_MAX 8
#define DISTRIBUTOR_ROOT_PATH "/"

typedef struct {
	s8 thread_name[DAVE_THREAD_NAME_LEN];
	s8 path[DAVE_PATH_LEN];
} HttpDistributorInfo;

static ThreadId _distributor_thread = INVALID_THREAD_ID;
static ThreadId _http_thread = INVALID_THREAD_ID;
static void *_distributor_kv = NULL;

static ub
_distributor_thread_number(void)
{
	ub thread_number;

	thread_number = dave_os_cpu_process_number();
	if(thread_number > DISTRIBUTOR_THREAD_MAX)
	{
		thread_number = DISTRIBUTOR_THREAD_MAX;
	}

	return thread_number;
}

static s8 *
_distributor_copy_path(s8 *dst_path, s8 *src_path)
{
	t_stdio_remove_the_char_on_frist(src_path, '/');
	t_stdio_remove_the_char_on_frist(src_path, '\\');

	if(dave_strlen(DISTRIBUTOR_ROOT_PATH) == 0)
	{
		dave_snprintf(dst_path, DAVE_PATH_LEN, "/%s", src_path);
	}
	else
	{
		if(((s8 *)DISTRIBUTOR_ROOT_PATH)[0] != '/')
		{
			HTTPLOG("invalid root path:%s", DISTRIBUTOR_ROOT_PATH);
		}
		if(dave_strlen(DISTRIBUTOR_ROOT_PATH) > 1)
		{
			dave_snprintf(dst_path, DAVE_PATH_LEN, "%s/%s", DISTRIBUTOR_ROOT_PATH, src_path);
		}
		else
		{
			dave_snprintf(dst_path, DAVE_PATH_LEN, "/%s", src_path);
		}
	}

	return dst_path;
}

static void
_distributor_http_close_rsp(MSGBODY *ptr)
{
	HTTPCloseRsp *pRsp = (HTTPCloseRsp *)(ptr->msg_body);

	if(pRsp->ret != ERRCODE_OK)
	{
		HTTPABNOR("error:%s", errorstr(pRsp->ret));
	}
}

static void
_distributor_http_close_req(ub port)
{
	HTTPCloseReq *pReq = thread_msg(pReq);

	pReq->listen_port = port;
	pReq->ptr = NULL;

	write_event(_http_thread, HTTPMSG_CLOSE_REQ, pReq, HTTPMSG_CLOSE_RSP, _distributor_http_close_rsp);
}

static void
_distributor_http_listen_rsp(MSGBODY *ptr)
{
	HTTPListenRsp *pRsp = (HTTPListenRsp *)(ptr->msg_body);

	if(pRsp->ret != ERRCODE_OK)
	{
		HTTPABNOR("error:%s", errorstr(pRsp->ret));
	}
}

static void
_distributor_http_listen_req(ub port)
{
	HTTPListenReq *pReq = thread_reset_msg(pReq);

	pReq->listen_port = port;
	pReq->rule = LocationMatch_CaseRegular;
	if(port == HTTPS_DISTRIBUTOR_SERVER_PORT)
		pReq->type = ListenHttps;
	else
		pReq->type = ListenHttp;
	if(dave_strlen(DISTRIBUTOR_ROOT_PATH) > 0)
	{
		dave_snprintf(pReq->path, sizeof(pReq->path), "%s", DISTRIBUTOR_ROOT_PATH);
	}
	else
	{
		dave_snprintf(pReq->path, sizeof(pReq->path), "/");
	}
	pReq->ptr = NULL;

	write_event(_http_thread, HTTPMSG_LISTEN_REQ, pReq, HTTPMSG_LISTEN_RSP, _distributor_http_listen_rsp);
}

static HttpDistributorInfo *
_distributor_malloc_info(ThreadId src, HTTPListenReq *pReq)
{
	HttpDistributorInfo *pInfo = dave_malloc(sizeof(HttpDistributorInfo));

	dave_strcpy(pInfo->thread_name, thread_name(src), sizeof(pInfo->thread_name));
	dave_strcpy(pInfo->path, pReq->path, sizeof(pInfo->path));

	return pInfo;
}

static void
_distributor_free_info(HttpDistributorInfo *pInfo)
{
	dave_free(pInfo);
}

static ErrCode
_distributor_clean_info(void *kv, s8 *key)
{
	HttpDistributorInfo *pInfo;

	pInfo = base_kv_inq_key_ptr(_distributor_kv, NULL);
	if(pInfo == NULL)
	{
		return ERRCODE_empty_data;
	}

	base_kv_del_key_ptr(kv, pInfo->path);

	_distributor_free_info(pInfo);

	return ERRCODE_OK;
}

static void
_distributor_listen_rsp(ThreadId dst, ErrCode ret, s8 *path, void *ptr)
{
	HTTPListenRsp *pRsp = thread_reset_msg(pRsp);

	pRsp->ret = ret;
	pRsp->listen_port = HTTP_DISTRIBUTOR_SERVER_PORT;
	_distributor_copy_path(pRsp->path, path);
	pRsp->ptr = ptr;

	write_msg(dst, HTTPMSG_LISTEN_RSP, pRsp);
}

static void
_distributor_listen_req(ThreadId src, HTTPListenReq *pReq)
{
	HttpDistributorInfo *pInfo;
	s8 path[DAVE_PATH_LEN];

	if(t_is_all_show_char((u8 *)(pReq->path), dave_strlen(pReq->path)) == dave_false)
	{
		_distributor_listen_rsp(src, ERRCODE_Invalid_parameter, pReq->path, pReq->ptr);
		return;
	}

	t_stdio_remove_the_char_on_frist(pReq->path, '/');
	t_stdio_remove_the_char_on_frist(pReq->path, '\\');

	pInfo = base_kv_inq_key_ptr(_distributor_kv, pReq->path);
	if(pInfo != NULL)
	{
		if(dave_strcmp(pInfo->thread_name, thread_name(src)) == dave_false)
		{
			HTTPABNOR("the owner thread is changed:%s->%s", pInfo->thread_name, thread_name(src));
			dave_strcpy(pInfo->thread_name, thread_name(src), sizeof(pInfo->thread_name));
		}
	}
	else
	{
		pInfo = _distributor_malloc_info(src, pReq);

		base_kv_add_key_ptr(_distributor_kv, pInfo->path, pInfo);
	}

	HTTPLOG("%s listen on path:%s success! %d/%d/%d/%d",
		thread_name(src), _distributor_copy_path(path, pReq->path),
		pReq->listen_port,
		pReq->rule, pReq->type,
		pReq->ptr);

	_distributor_listen_rsp(src, ERRCODE_OK, pReq->path, pReq->ptr);
}

static void
_distributor_close_rsp(ThreadId dst, ErrCode ret, void *ptr)
{
	HTTPCloseRsp *pRsp = thread_msg(pRsp);

	pRsp->ret = ret;
	pRsp->listen_port = HTTP_DISTRIBUTOR_SERVER_PORT;
	pRsp->ptr = ptr;

	write_msg(dst, HTTPMSG_CLOSE_RSP, pRsp);
}

static void
_distributor_close_req(ThreadId src, HTTPCloseReq *pReq)
{
	HttpDistributorInfo *pInfo;

	pInfo = base_kv_inq_key_ptr(_distributor_kv, pReq->path);
	if(pInfo != NULL)
	{
		base_kv_del_key_ptr(_distributor_kv, pInfo->path);

		_distributor_free_info(pInfo);
	}

	HTTPLOG("%s close on path:%s success!",
		thread_name(src), pReq->path,
		pReq->listen_port);

	_distributor_close_rsp(src, ERRCODE_OK, pReq->ptr);
}

static void
_distributor_recv_error_rsp(ErrCode ret, void *ptr)
{
	HTTPRecvRsp *pRsp = thread_msg(pRsp);

	pRsp->ret = ret;
	pRsp->content_type = HttpContentType_json;
	pRsp->content = t_a2b_param_to_mbuf("{ \"Error_Message\" : \"%s\"}", errorstr(ret));
	pRsp->local_creat_time = dave_os_time_us();
	pRsp->ptr = ptr;

	write_msg(_http_thread, HTTPMSG_RECV_RSP, pRsp);
}

static void
_distributor_process_path(s8 *path, HTTPRecvReq *pReq)
{
	s8 get_path[DAVE_URL_LEN];

	dave_strcpy(path, http_find_kv(pReq->head, DAVE_HTTP_HEAD_MAX, "REQUEST_URI"), DAVE_URL_LEN);
	if(pReq->method == HttpMethod_get)
	{
		dave_strfind(path, (s8)'?', get_path, DAVE_URL_LEN);
		dave_strcpy(path, get_path, DAVE_URL_LEN);
		http_build_kv(pReq->head, DAVE_HTTP_HEAD_MAX, "REQUEST_URI", path);
	}

	t_stdio_remove_the_char_on_frist(path, '/');
	t_stdio_remove_the_char_on_frist(path, '\\');
}

static HttpDistributorInfo *
_distributor_recv_info(MSGBODY *msg)
{
	HTTPRecvReq *pReq = (HTTPRecvReq *)(msg->msg_body);
	s8 path[DAVE_URL_LEN];
	HttpDistributorInfo *pInfo;

	_distributor_process_path(path, pReq);

	pInfo = (HttpDistributorInfo *)base_kv_inq_key_ptr(_distributor_kv, path);
	if(pInfo == NULL)
	{
		HTTPLOG("remote:%s/%d method:%d can't find the path:%s REQUEST_URI:%s QUERY_STRING:%s content:%s!",
			pReq->remote_address, pReq->remote_port,
			pReq->method,
			path,
			http_find_kv(pReq->head, DAVE_HTTP_HEAD_MAX, "REQUEST_URI"),
			http_find_kv(pReq->head, DAVE_HTTP_HEAD_MAX, "QUERY_STRING"),
			pReq->content==NULL ? "NULL" : pReq->content->payload);
	}

	HTTPDEBUG("path:%s pInfo:%x", path, pInfo);

	return pInfo;
}

static void
_distributor_recv_req(MSGBODY *msg)
{
	HttpDistributorInfo *pInfo;
	HTTPRecvReq *pReq = (HTTPRecvReq *)(msg->msg_body);

	pInfo = _distributor_recv_info(msg);
	if(pInfo != NULL)
	{
		remote_msg(pInfo->thread_name, HTTPMSG_RECV_REQ, pReq);

		msg->mem_state = MsgMemState_captured;
	}
	else
	{
		_distributor_recv_error_rsp(ERRCODE_Can_not_find_path, pReq->ptr);
	
		dave_mfree(pReq->content);
	}
}

static void
_distributor_recv_rsp(MSGBODY *msg)
{
	if(snd_from_msg(_distributor_thread, _http_thread, msg->msg_id, msg->msg_len, msg->msg_body) == dave_true)
	{
		msg->mem_state = MsgMemState_captured;
	}
}

static void
_distributor_restart(RESTARTREQMSG *pRestart)
{
	if(pRestart->times == 1)
	{
		_distributor_http_close_req(HTTPS_DISTRIBUTOR_SERVER_PORT);
		_distributor_http_close_req(HTTP_DISTRIBUTOR_SERVER_PORT);
	}
}

static void
_distributor_init(MSGBODY *msg)
{
	_distributor_kv = base_kv_malloc("http-distributor", KVAttrib_ram, 0, NULL);
	_http_thread = thread_id(HTTP_THREAD_NAME);

	_distributor_http_listen_req(HTTPS_DISTRIBUTOR_SERVER_PORT);
	_distributor_http_listen_req(HTTP_DISTRIBUTOR_SERVER_PORT);
}

static void
_distributor_main(MSGBODY *msg)
{
	switch((ub)msg->msg_id)
	{
		case MSGID_RESTART_REQ:
				_distributor_restart((RESTARTREQMSG *)(msg->msg_body));
			break;
		case HTTPMSG_LISTEN_REQ:
				_distributor_listen_req(msg->msg_src, (HTTPListenReq *)(msg->msg_body));
			break;
		case HTTPMSG_CLOSE_REQ:
				_distributor_close_req(msg->msg_src, (HTTPCloseReq *)(msg->msg_body));
			break;
		case HTTPMSG_RECV_REQ:
				_distributor_recv_req(msg);
			break;
		case HTTPMSG_RECV_RSP:
				_distributor_recv_rsp(msg);
			break;
		default:
			break;
	}
}

static void
_distributor_exit(MSGBODY *msg)
{
	base_kv_free(_distributor_kv, _distributor_clean_info);

	_distributor_kv = NULL;
}

// =====================================================================

void
http_distributor_init(void)
{
	ub thread_number = _distributor_thread_number();

	_distributor_thread = dave_thread_creat(DISTRIBUTOR_THREAD_NAME, thread_number, THREAD_THREAD_FLAG, _distributor_init, _distributor_main, _distributor_exit);
	if(_distributor_thread == INVALID_THREAD_ID)
		dave_restart(DISTRIBUTOR_THREAD_NAME);
}

void
http_distributor_exit(void)
{
	if(_distributor_thread != INVALID_THREAD_ID)
		dave_thread_del(_distributor_thread);
	_distributor_thread = INVALID_THREAD_ID;
}

