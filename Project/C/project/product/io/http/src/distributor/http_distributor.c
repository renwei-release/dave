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
#include "dave_echo.h"
#include "http_recv.h"
#include "http_distributor.h"
#include "distributor_info.h"
#include "http_tools.h"
#include "http_log.h"

static ub _distributor_port_list[] = { 443, 1823, 0 };

#define DISTRIBUTOR_THREAD_MAX 8
#define DISTRIBUTOR_ROOT_PATH "/"

static ThreadId _distributor_thread = INVALID_THREAD_ID;
static ThreadId _http_thread = INVALID_THREAD_ID;

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

	if(pRsp->ret != RetCode_OK)
	{
		HTTPABNOR("error:%s", retstr(pRsp->ret));
	}
}

static void
_distributor_http_close_req(ub port)
{
	HTTPCloseReq *pReq = thread_reset_msg(pReq);

	pReq->listen_port = port;
	pReq->ptr = NULL;

	id_event(_http_thread, HTTPMSG_CLOSE_REQ, pReq, HTTPMSG_CLOSE_RSP, _distributor_http_close_rsp);
}

static void
_distributor_http_listen_rsp(MSGBODY *ptr)
{
	HTTPListenRsp *pRsp = (HTTPListenRsp *)(ptr->msg_body);

	if(pRsp->ret != RetCode_OK)
	{
		HTTPABNOR("error:%s", retstr(pRsp->ret));
	}
}

static void
_distributor_http_listen_req(ub port)
{
	HTTPListenReq *pReq = thread_reset_msg(pReq);

	pReq->listen_port = port;
	pReq->rule = LocationMatch_CaseRegular;
	pReq->type = ListenHttps;
	if(dave_strlen(DISTRIBUTOR_ROOT_PATH) > 0)
	{
		dave_snprintf(pReq->path, sizeof(pReq->path), "%s", DISTRIBUTOR_ROOT_PATH);
	}
	else
	{
		dave_snprintf(pReq->path, sizeof(pReq->path), "/");
	}
	pReq->ptr = NULL;

	id_event(_http_thread, HTTPMSG_LISTEN_REQ, pReq, HTTPMSG_LISTEN_RSP, _distributor_http_listen_rsp);
}

static void
_distributor_listen_rsp(ThreadId dst, RetCode ret, s8 *path, void *ptr)
{
	HTTPListenRsp *pRsp = thread_reset_msg(pRsp);

	pRsp->ret = ret;
	pRsp->listen_port = _distributor_port_list[0];
	_distributor_copy_path(pRsp->path, path);
	pRsp->ptr = ptr;

	id_msg(dst, HTTPMSG_LISTEN_RSP, pRsp);
}

static RetCode
_distributor_listen_req(ThreadId src, s8 *path, ub listening_seconds_time, void *ptr)
{
	HttpDistributorInfo *pInfo;
	s8 temp_path[DAVE_PATH_LEN];
	dave_bool ret;

	if(t_is_all_show_char((u8 *)(path), dave_strlen(path)) == dave_false)
	{
		return RetCode_Invalid_parameter;
	}

	t_stdio_remove_the_char_on_frist(path, '/');
	t_stdio_remove_the_char_on_frist(path, '\\');

	pInfo = distributor_info_inq(path);
	if(pInfo != NULL)
	{
		if(dave_strcmp(pInfo->thread_name, thread_name(src)) == dave_false)
		{
			HTTPABNOR("the owner thread is changed:%s->%s", pInfo->thread_name, thread_name(src));
			dave_strcpy(pInfo->thread_name, thread_name(src), sizeof(pInfo->thread_name));
		}
		ret = dave_true;
	}
	else
	{
		ret = distributor_info_malloc(src, path, listening_seconds_time);
	}

	HTTPLOG("%s listen on path:%s %s! time:%d",
		thread_name(src), _distributor_copy_path(temp_path, path),
		ret==dave_false?"failed":"success",
		listening_seconds_time);

	return RetCode_OK;
}

static void
_distributor_close_rsp(ThreadId dst, RetCode ret, void *ptr)
{
	HTTPCloseRsp *pRsp = thread_reset_msg(pRsp);

	pRsp->ret = ret;
	pRsp->ptr = ptr;

	id_msg(dst, HTTPMSG_CLOSE_RSP, pRsp);
}

static void
_distributor_close_req(ThreadId src, HTTPCloseReq *pReq)
{
	t_stdio_remove_the_char_on_frist(pReq->path, '/');
	t_stdio_remove_the_char_on_frist(pReq->path, '\\');

	distributor_info_free(pReq->path);

	HTTPLOG("%s close on path:%s success!",
		thread_name(src), pReq->path);

	_distributor_close_rsp(src, RetCode_OK, pReq->ptr);
}

static void
_distributor_recv_error_rsp(RetCode ret, void *ptr)
{
	HTTPRecvRsp *pRsp = thread_reset_msg(pRsp);

	pRsp->ret = ret;
	pRsp->content_type = HttpContentType_json;
	pRsp->content = t_a2b_param_to_mbuf("{ \"Error_Message\" : \"%s\"}", retstr(ret));
	pRsp->local_creat_time = dave_os_time_us();
	pRsp->ptr = ptr;

	id_msg(_http_thread, HTTPMSG_RECV_RSP, pRsp);
}

static inline void
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

	dave_strcpy(pReq->remote_address, path, sizeof(pReq->remote_address));
}

static inline HttpDistributorInfo *
_distributor_recv_info(MSGBODY *msg)
{
	HTTPRecvReq *pReq = (HTTPRecvReq *)(msg->msg_body);
	s8 path[DAVE_URL_LEN];
	HttpDistributorInfo *pInfo;

	_distributor_process_path(path, pReq);

	pInfo = distributor_info_inq(path);
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
		name_qmsg(pInfo->thread_name, HTTPMSG_RECV_REQ, pReq);

		t_lock_spin(&(pInfo->pv));
		pInfo->receive_counter ++;
		t_unlock_spin(&(pInfo->pv));
	}
	else
	{
		_distributor_recv_error_rsp(RetCode_Can_not_find_path, pReq->ptr);
	
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
	else
	{
		HTTPABNOR("Failed to forward data:%s!", msgstr(msg->msg_id));
	}
}

static ub
_distributor_info(s8 *info_ptr, ub info_len)
{
	ub info_index = 0, index = 0;

	info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "listen port:\n");
	for(index=0; index<1024; index++)
	{
		if(_distributor_port_list[index] == 0)
			break;

		if(index == 0)
			info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, " ");
		if(index > 0)
			info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,  " ", _distributor_port_list[index]);
		info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "%d", _distributor_port_list[index]);

		if(_distributor_port_list[index + 1] == 0)
			info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "\n");
	}

	info_index += distributor_info_info(&info_ptr[info_index], info_len-info_index);

	return info_index;
}

static void
_distributor_debug(ThreadId src, DebugReq *pReq)
{
	DebugRsp *pRsp = thread_reset_msg(pRsp);

	switch(pReq->msg[0])
	{
		case 'i':
				_distributor_info(pRsp->msg, sizeof(pRsp->msg));				
			break;
		default:
				dave_snprintf(pRsp->msg, sizeof(pRsp->msg), "%s", pReq->msg);
			break;
	}

	pRsp->ptr = pReq->ptr;

	id_msg(src, MSGID_DEBUG_RSP, pRsp);
}

static void
_distributor_restart(RESTARTREQMSG *pRestart)
{
	ub list_index;

	if(pRestart->times == 1)
	{
		for(list_index=0; list_index<1024; list_index++)
		{
			if(_distributor_port_list[list_index] == 0)
				break;

			_distributor_http_close_req(_distributor_port_list[list_index]);
		}
	}
}

static void
_distributor_listen_path(ThreadId src, HTTPListenReq *pReq)
{
	RetCode ret;

	ret = _distributor_listen_req(src, pReq->path, 0, pReq->ptr);
	
	_distributor_listen_rsp(src, ret, pReq->path, pReq->ptr);
}

static void
_distributor_listen_path_time(ThreadId src, HTTPListenAutoCloseReq *pReq)
{
	HTTPListenAutoCloseRsp *pRsp = thread_reset_msg(pRsp);

	pRsp->ret = _distributor_listen_req(src, pReq->path, pReq->listening_seconds_time, pReq->ptr);
	_distributor_copy_path(pRsp->path, pReq->path);
	pRsp->ptr = pReq->ptr;

	id_msg(src, HTTPMSG_LISTEN_AUTO_CLOSE_RSP, pRsp);
}

static void
_distributor_init(MSGBODY *msg)
{
	ub list_index;

	distributor_info_init();

	_http_thread = thread_id(HTTP_THREAD_NAME);

	for(list_index=0; list_index<1024; list_index++)
	{
		if(_distributor_port_list[list_index] == 0)
			break;
	
		_distributor_http_listen_req(_distributor_port_list[list_index]);
	}
}

static void
_distributor_main(MSGBODY *msg)
{
	switch((ub)msg->msg_id)
	{
		case MSGID_DEBUG_REQ:
				_distributor_debug(msg->msg_src, (DebugReq *)(msg->msg_body));
			break;
		case MSGID_RESTART_REQ:
				_distributor_restart((RESTARTREQMSG *)(msg->msg_body));
			break;
		case MSGID_ECHO_REQ:
		case MSGID_ECHO_RSP:
				dave_echo(msg->msg_src, msg->msg_dst, msg->msg_id, msg->msg_body);
			break;
		case HTTPMSG_LISTEN_REQ:
				_distributor_listen_path(msg->msg_src, (HTTPListenReq *)(msg->msg_body));
			break;
		case HTTPMSG_LISTEN_AUTO_CLOSE_REQ:
				_distributor_listen_path_time(msg->msg_src, (HTTPListenAutoCloseReq *)(msg->msg_body));
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
	distributor_info_exit();
}

// =====================================================================

void
http_distributor_init(void)
{
	ub thread_number = _distributor_thread_number();

	_distributor_thread = base_thread_creat(
		DISTRIBUTOR_THREAD_NAME, thread_number, THREAD_THREAD_FLAG|THREAD_dCOROUTINE_FLAG,
		_distributor_init, _distributor_main, _distributor_exit);
	if(_distributor_thread == INVALID_THREAD_ID)
		base_restart(DISTRIBUTOR_THREAD_NAME);
}

void
http_distributor_exit(void)
{
	if(_distributor_thread != INVALID_THREAD_ID)
		base_thread_del(_distributor_thread);
	_distributor_thread = INVALID_THREAD_ID;
}

