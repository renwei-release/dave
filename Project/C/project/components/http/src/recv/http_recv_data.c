/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_http.h"
#include "http_recv.h"
#include "http_recv_param.h"
#include "http_recv_listen.h"
#include "http_recv_data.h"
#include "http_fastcgi.h"
#include "http_tools.h"
#include "http_log.h"

#define HTTP_RECV_MAX (DAVE_SERVER_SUPPORT_SOCKET_MAX)
#define HTTP_TIMER_OUT_EFFECTIVE_MIN (1000000)
#define HTTP_REQ_EMPTY_SERIAL 0

typedef struct {
	s32 socket;
	ub nginx_port;
	ub cgi_port;
	Fcgi fcgi;

	ub old_req_serial;
	ub req_serial;

	ub recv_buf_index;
	u8 *recv_buf;

	ub last_plugin_time;
	ub last_read_time;
	ub last_plugout_time;

	ub recv_index;

	TLock opt_pv;

	void *ptr;
} HTTPRecv;

static dave_bool _http_recv_req(HTTPRecv *pRecv, s32 socket);

static ThreadId _socket_thread_id = INVALID_SOCKET_ID;
static HTTPRecv _http_recv[HTTP_RECV_MAX];
static TLock _http_req_serial_pv;
static volatile ub _http_req_serial = 0;

static ub
_http_recv_serial(void)
{
	ub serial = 0;

	t_lock_spin(&_http_req_serial_pv);

	{
		if(_http_req_serial == HTTP_REQ_EMPTY_SERIAL)
		{
			_http_req_serial = HTTP_REQ_EMPTY_SERIAL + 1;
		}

		serial = _http_req_serial ++;
	}

	t_unlock_spin(&_http_req_serial_pv);

	return serial;
}

static void *
_http_recv_recv_to_ptr(HTTPRecv *pRecv)
{
	ub serial, recv_index;

	serial = pRecv->req_serial;
	recv_index = (pRecv->recv_index) & 0xffffffff;

	SAFECODEv1(pRecv->opt_pv, {

		if((pRecv->socket == INVALID_SOCKET_ID) && (pRecv->req_serial != HTTP_REQ_EMPTY_SERIAL))
		{
			HTTPABNOR("An unprocessed node(serial:%ld) was found.", pRecv->req_serial);
		}

		if(pRecv->req_serial == HTTP_REQ_EMPTY_SERIAL)
		{
			pRecv->req_serial = serial = _http_recv_serial() & 0xffffffff;
		}

	} );

	return (void *)((serial << 32) | (recv_index));
}

static HTTPRecv *
_http_recv_ptr_to_recv(void *ptr)
{
	ub serial, recv_index;
	ub consumption_time;
	HTTPRecv *pRecv;
	dave_bool check_error;

	serial = ((ub)ptr) >> 32;
	recv_index = ((ub)ptr) & 0xffffffff;

	if(recv_index >= HTTP_RECV_MAX)
	{
		HTTPABNOR("invalid recv_index:%ld", recv_index);
		return NULL;
	}

	pRecv = &_http_recv[recv_index];

	check_error = dave_true;

	SAFECODEv1(pRecv->opt_pv, {

		if(pRecv->req_serial != serial)
		{
			consumption_time = dave_os_time_us() - pRecv->last_read_time;

			HTTPLOG("socket:%d serial mismatch:%lx/%lx/%lx! time:%lds I:%ld/R:%ld/O:%ld",
				pRecv->socket,
				pRecv->req_serial, pRecv->old_req_serial, serial, consumption_time/1000000,
				pRecv->last_plugin_time, pRecv->last_read_time, pRecv->last_plugout_time);

			check_error = dave_false;
		}

	} );

	if(check_error == dave_false)
	{
		return NULL;
	}
	else
	{
		return pRecv;
	}
}

static void
_http_recv_data_reset(HTTPRecv *pRecv)
{
	pRecv->socket = INVALID_SOCKET_ID;
	pRecv->nginx_port = 0;
	pRecv->fcgi.params = NULL;
	pRecv->fcgi.content_length_max = 0;
	pRecv->fcgi.content_length = 0;
	pRecv->fcgi.content = NULL;

	pRecv->old_req_serial = pRecv->req_serial; 
	pRecv->req_serial = HTTP_REQ_EMPTY_SERIAL;

	pRecv->recv_buf_index = 0;
	pRecv->recv_buf = NULL;
}

static void
_http_recv_data_clean(HTTPRecv *pRecv)
{
	if(pRecv->recv_buf != NULL)
	{
		dave_free(pRecv->recv_buf);
		pRecv->recv_buf = NULL;
	}

	_http_recv_data_reset(pRecv);
}

static void
_http_recv_data_reset_all(void)
{
	ub recv_index;

	for(recv_index=0; recv_index<HTTP_RECV_MAX; recv_index++)
	{
		dave_memset(&_http_recv[recv_index], 0x00, sizeof(HTTPRecv));

		_http_recv[recv_index].last_plugin_time = 0;
		_http_recv[recv_index].last_read_time = 0;
		_http_recv[recv_index].last_plugout_time = 0;

		_http_recv[recv_index].recv_index = recv_index;

		_http_recv_data_reset(&_http_recv[recv_index]);

		t_lock_reset(&(_http_recv[recv_index].opt_pv));
	}
}

static dave_bool
_http_recv_get_content(Fcgi *pFcgi)
{
	FCGIParamsBody *pRequestParam, *pQueryParam;

	pRequestParam = http_fastcgi_find_head(pFcgi->params, "REQUEST_METHOD");
	if(pRequestParam == NULL)
	{
		return dave_false;
	}

	if(dave_strcmp(pRequestParam->value, "GET") == dave_false)
	{
		return dave_false;
	}

	pQueryParam = http_fastcgi_find_head(pFcgi->params, "QUERY_STRING");
	if(pQueryParam == NULL)
	{
		return dave_false;
	}

	if(pFcgi->content != NULL)
	{
		dave_free(pFcgi->content);
	}

	pFcgi->content_length = dave_strlen(pQueryParam->value);
	if(pFcgi->content_length == 0)
	{
		return dave_false;
	}

	pFcgi->content = dave_malloc(pFcgi->content_length + 1);

	pFcgi->content_length = http_copy_uri(pFcgi->content, pFcgi->content_length, pQueryParam->value, pFcgi->content_length);

	pFcgi->content[pFcgi->content_length] = '\0';

	return dave_true;
}

static void
_http_recv_tidy_content(MBUF *content)
{
	s8 *content_ptr;
	ub content_len;

	if(content != NULL)
	{
		content_ptr = content->payload;
		content_len = content->len;
		while((content_len > 0) && (content_ptr[content_len - 1] == '\0')) -- content_len;
		content->len = content_len;
	}
	else
	{
		content_ptr = NULL;
		content_len = 0;
	}
}

static dave_bool
_http_recv_check_fcgi(HTTPRecv *pRecv)
{
	Fcgi *pFcgi;

	if(pRecv == NULL)
	{
		HTTPABNOR("pRecv is NULL!");
		return dave_false;
	}

	pFcgi = &(pRecv->fcgi);

	if(pFcgi->request.role != FCGI_RESPONDER)
	{
		HTTPLOG("only support RESPONDER now! role:%d", pFcgi->request.role);
	}

	if(pFcgi->request.flags != FCGI_DONOT_KEEP_CONN)
	{
		HTTPLOG("only support do't keep connect! flags:%d", pFcgi->request.flags);
	}

	if(pFcgi->content_length == 0)
	{
		HTTPDEBUG("content is empty, get other content!");

		if(_http_recv_get_content(pFcgi) == dave_false)
		{
			HTTPDEBUG("empty content!");
		}
	}

	return dave_true;
}

static dave_bool
_http_recv_get_cgi_data(
	HTTPRecv *pRecv,
	ub cgi_port,
	u8 *data, ub data_len)
{
	HTTPDEBUG("socket:%d cgi-port:%d data-len:%d", pRecv->socket, cgi_port, data_len);

	if(pRecv->socket == INVALID_SOCKET_ID)
	{
		HTTPLOG("the socket:%d is pllugout!", pRecv->socket);
		return dave_false;
	}

	if(cgi_port == 0)
	{
		HTTPLOG("invalid cgi_port:%d", cgi_port);
	}

	pRecv->cgi_port = cgi_port;

	return http_fastcgi_parse(data, data_len, &(pRecv->fcgi));
}

static ub
_http_recv_set_cgi_data(HTTPRecv *pRecv, HttpContentType content_type, ub content_length, s8 *content_ptr, ub cgi_len, u8 *cgi_ptr)
{
	return http_fastcgi_build(&(pRecv->fcgi), content_type, content_ptr, content_length, cgi_ptr, cgi_len);
}

static ub
_http_recv_set_cgi_error(HTTPRecv *pRecv, ub cgi_len, u8 *cgi)
{
	return http_fastcgi_build_error(&(pRecv->fcgi), cgi, cgi_len);
}

static ub
_http_recv_set_cgi_ok(HTTPRecv *pRecv, ub cgi_len, u8 *cgi)
{
	return http_fastcgi_build_ok(&(pRecv->fcgi), cgi, cgi_len);
}

static void
_http_recv_write(s32 socket, MBUF *pMbuf)
{
	SocketWrite *pWrite;

	HTTPDEBUG("socket:%d length:%d", socket, pMbuf->len);

	if(socket == INVALID_SOCKET_ID)
	{
		dave_mfree(pMbuf);
		return;
	}

	pWrite = thread_msg(pWrite);

	pWrite->socket = socket;
	pWrite->data_len = pMbuf->len;
	pWrite->data = pMbuf;
	pWrite->close_flag = SOCKETINFO_WRITE_THEN_CLOSE;

	if(_socket_thread_id == INVALID_SOCKET_ID)
	{
		_socket_thread_id = thread_id(SOCKET_THREAD_NAME);
	}

	if(id_msg(_socket_thread_id, SOCKET_WRITE, pWrite) == dave_false)
	{
		dave_mfree(pMbuf);
	}
}

static void
_http_recv_write_error(HTTPRecv *pRecv, s32 socket)
{
	MBUF *pMbuf;

	if(pRecv == NULL)
	{
		http_recv_listen_release(socket);
	}
	else
	{
		pRecv->req_serial = HTTP_REQ_EMPTY_SERIAL;

		pMbuf = dave_mmalloc(1024);

		pMbuf->len = pMbuf->tot_len =
			_http_recv_set_cgi_error(pRecv, pMbuf->len, (u8 *)(pMbuf->payload));

		_http_recv_write(socket, pMbuf);
	}
}

static void
_http_recv_write_ok(HTTPRecv *pRecv, s32 socket)
{
	MBUF *pMbuf;

	if(pRecv == NULL)
	{
		http_recv_listen_release(socket);
	}
	else
	{
		pRecv->req_serial = HTTP_REQ_EMPTY_SERIAL;

		pMbuf = dave_mmalloc(2048);

		pMbuf->len = pMbuf->tot_len =
			_http_recv_set_cgi_ok(pRecv, pMbuf->len, (u8 *)(pMbuf->payload));

		_http_recv_write(socket, pMbuf);
	}
}

static void
_http_recv_check_plugout_abnormal(HTTPRecv *pRecv)
{
	ub consumption_time;

	if(pRecv->req_serial != HTTP_REQ_EMPTY_SERIAL)
	{
		consumption_time = dave_os_time_us() - pRecv->last_plugin_time;
	
		if(consumption_time > HTTP_TIMER_OUT_EFFECTIVE_MIN)
		{
			HTTPDEBUG("http req timer out! socket:%d nginx_port:%d cgi_port:%d serial:%ld consumption-time:%dus content:%s",
				pRecv->socket, pRecv->nginx_port, pRecv->cgi_port, pRecv->req_serial,
				consumption_time,
				pRecv->fcgi.content);
		}
		else
		{
			HTTPABNOR("http req timer out! socket:%d nginx_port:%d cgi_port:%d serial:%ld consumption-time:%dus content:%s",
				pRecv->socket, pRecv->nginx_port, pRecv->cgi_port, pRecv->req_serial,
				consumption_time,
				pRecv->fcgi.content);
		}
	}

	if(pRecv->last_read_time < pRecv->last_plugin_time)
	{
		HTTPLOG("the socket:%d no data received! %ld/%ld/%ld",
			pRecv->socket,
			pRecv->last_plugin_time, pRecv->last_read_time, pRecv->last_plugout_time);
	}
}

static void
_http_recv_data_plugin(HTTPRecv *pRecv, s32 socket, ub nginx_port, ub msg_serial)
{
	HTTPDEBUG("socket:%d", socket);

	if(pRecv->socket != INVALID_SOCKET_ID)
	{
		HTTPLOG("repeat plugin socket:%d/%d cur-time:%ld msg-serial:%d",
			socket, pRecv->socket, dave_os_time_us(), msg_serial);
	}
	else
	{
		_http_recv_data_clean(pRecv);

		pRecv->socket = socket;

		pRecv->nginx_port = nginx_port;

		pRecv->last_plugin_time = dave_os_time_us();
	}
}

static void
_http_recv_data_plugout(HTTPRecv *pRecv, s32 socket)
{
	HTTPDEBUG("socket:%d", pRecv->socket);

	if(pRecv->socket != socket)
	{
		HTTPABNOR("invalid plugout socket:%d/%d", pRecv->socket, socket);
	}
	else
	{
		pRecv->last_plugout_time = dave_os_time_us();

		_http_recv_check_plugout_abnormal(pRecv);

		http_fastcgi_parse_release(&(pRecv->fcgi));

		_http_recv_data_clean(pRecv);
	}
}

static dave_bool
_http_recv_parse(HTTPRecv *pRecv, SocketRead *pRead)
{
	dave_bool parse_end = dave_false;

	HTTPDEBUG("recv data:%d+%d", pRecv->recv_buf_index, pRead->data_len);

	if((pRecv->recv_buf_index == 0) || (pRecv->recv_buf == NULL))
	{
		parse_end = _http_recv_get_cgi_data(pRecv, pRead->IPInfo.dst_port, (u8 *)(pRead->data->payload), pRead->data_len);

		if(parse_end == dave_false)
		{
			if(pRecv->recv_buf == NULL)
			{
				pRecv->recv_buf = dave_malloc(HTTP_RECV_BUF_MAX);
	
				pRecv->recv_buf_index = 0;
			}

			if((pRecv->recv_buf_index + pRead->data_len) <= HTTP_RECV_BUF_MAX)
			{
				dave_memcpy(&(pRecv->recv_buf[pRecv->recv_buf_index]), (u8 *)(pRead->data->payload), pRead->data_len);
	
				pRecv->recv_buf_index += pRead->data_len;
			}
			else
			{
				HTTPABNOR("recv data too longer:%d/%d!", pRecv->recv_buf_index, pRead->data_len);
			}
		}
	}
	else
	{
		if((pRecv->recv_buf_index + pRead->data_len) <= HTTP_RECV_BUF_MAX)
		{
			dave_memcpy(&(pRecv->recv_buf[pRecv->recv_buf_index]), (u8 *)(pRead->data->payload), pRead->data_len);

			pRecv->recv_buf_index += pRead->data_len;

			parse_end = _http_recv_get_cgi_data(pRecv, pRead->IPInfo.dst_port, (u8 *)(pRecv->recv_buf), pRecv->recv_buf_index);
		}
		else
		{
			HTTPABNOR("recv data too longer:%d/%d!", pRecv->recv_buf_index, pRead->data_len);
		}
	}

	return parse_end;
}

static void
_http_recv_read(HTTPRecv *pRecv, SocketRead *pRead, ub msg_serial)
{
	dave_bool parse_end = dave_false;

	HTTPDEBUG("socket:%d data_len:%d", pRead->socket, pRead->data_len);

	if(pRecv->socket == INVALID_SOCKET_ID)
	{
		HTTPLOG("socket:%d plugin message delayed arrival! cur-time:%ld msg_serial:%d time:%ld/%ld/%ld %s->%s",
			pRead->socket,
			dave_os_time_us(),
			msg_serial,
			pRecv->last_plugin_time, pRecv->last_read_time, pRecv->last_plugout_time,
			ipv4str(pRead->IPInfo.src_ip, pRead->IPInfo.src_port),
			ipv4str2(pRead->IPInfo.dst_ip, pRead->IPInfo.dst_port));

		_http_recv_data_plugin(pRecv, pRead->socket, http_recv_listen_port(pRead->IPInfo.dst_port), msg_serial);
	}

	if(pRecv->socket != pRead->socket)
	{
		HTTPABNOR("socket:%d/%d mismatch!",
			pRecv->socket, pRead->socket);
		http_recv_listen_release(pRead->socket);
		return;
	}

	pRecv->last_read_time = dave_os_time_us();

	parse_end = _http_recv_parse(pRecv, pRead);

	if(parse_end == dave_true)
	{
		HTTPDEBUG("parse end:%d/%d!", pRecv->recv_buf_index, pRead->data_len);

		if(_http_recv_check_fcgi(pRecv) == dave_false)
		{
			HTTPABNOR("invalid fcgi! dst_port:%d", pRead->IPInfo.dst_port);

			_http_recv_write_error(pRecv, pRead->socket);
		}

		if(_http_recv_req(pRecv, pRead->socket) == dave_false)
		{
			_http_recv_write_error(pRecv, pRead->socket);
		}
	}
	else
	{
		HTTPDEBUG("parse not end:%d/%d!", pRecv->recv_buf_index, pRead->data_len);

		http_fastcgi_parse_release(&(pRecv->fcgi));
	}
}

static dave_bool
_http_recv_req(HTTPRecv *pRecv, s32 socket)
{
	HTTPRecvReq *pReq;
	ub content_length;
	s8 *listen_thread;

	pReq = thread_msg(pReq);

	pReq->listen_port = pRecv->nginx_port;

	content_length = http_fastcgi_load_head(pReq, pRecv->fcgi.params);

	if(pReq->method == HttpMethod_options)
	{
		_http_recv_write_ok(pRecv, socket);
		thread_msg_release(pReq);
		return dave_true;
	}

	if(((pRecv->fcgi.content_length == 0)
			&& (pReq->method != HttpMethod_options)
			&& (pReq->method != HttpMethod_delete))
		|| (pReq->method >= HttpMethod_max))
	{
		HTTPLOG("invalid req, content_length:%d method:%s",
			pRecv->fcgi.content_length, t_auto_HttpMethod_str(pReq->method));
		thread_msg_release(pReq);
		return dave_false;
	}

	if(content_length != pRecv->fcgi.content_length)
	{
		HTTPDEBUG("content_length:%d/%d mismatch! nginx_port:%d cgi_port:%d",
			content_length, pRecv->fcgi.content_length,
			pRecv->nginx_port, pRecv->cgi_port);
		content_length = pRecv->fcgi.content_length;
	}

	if(content_length > 0)
	{
		pReq->content = dave_mmalloc(content_length);
		dave_memcpy(pReq->content->payload, pRecv->fcgi.content, content_length);
	}
	else
	{
		pReq->content = NULL;
	}
	pReq->ptr = pRecv->ptr;

	listen_thread = http_recv_listen_thread(pRecv->cgi_port);
	if(listen_thread != NULL)
	{
		HTTPDEBUG("listen_thread:%s content length:%d cgi_port:%d",
			listen_thread, pRecv->fcgi.content_length, pRecv->cgi_port);

		pReq->local_creat_time = dave_os_time_us();

		name_msg(listen_thread, HTTPMSG_RECV_REQ, pReq);

		return dave_true;
	}
	else
	{
		thread_msg_release(pReq);
		return dave_false;
	}
}

static void
_http_recv_rsp(HTTPRecv *pRecv, HTTPRecvRsp *pRsp)
{
	s8 *content_ptr;
	ub content_len;
	MBUF *pMbuf;
	ub local_current_time;

	if(pRecv->socket == INVALID_SOCKET_ID)
	{
		HTTPLOG("the socket lost! content:%s time:%ld/%ld/%ld/%ld",
			pRsp->content,
			dave_os_time_us(), pRecv->last_plugin_time,
			pRecv->last_read_time, pRecv->last_plugout_time);
		return;
	}

	local_current_time = dave_os_time_us();
	if(local_current_time > pRsp->local_creat_time)
	{
		if((local_current_time - pRsp->local_creat_time) > 30000000)
		{
			HTTPLOG("get the msg has long time! time:%lds/%ldus/%ldus socket:%d port:%d/%d content:%x",
				(local_current_time - pRsp->local_creat_time)/1000000,
				local_current_time, pRsp->local_creat_time,
				pRecv->socket,
				pRecv->nginx_port, pRecv->cgi_port,
				pRsp->content);
		}
	}
	else
	{
		HTTPLTRACE(60,1,
			"Please note that the time of the server applying for this port(%d/%d %ld/%ld) is different from the local time.",
			pRecv->nginx_port, pRecv->cgi_port, pRsp->local_creat_time, local_current_time);
	}

	_http_recv_tidy_content(pRsp->content);

	content_ptr = dave_mptr(pRsp->content);
	content_len = dave_mlen(pRsp->content);

	pMbuf = dave_mmalloc(2048 + content_len);

	pMbuf->len = pMbuf->tot_len =
		_http_recv_set_cgi_data(pRecv,
			pRsp->content_type, content_len, content_ptr,
			pMbuf->len, (u8 *)(pMbuf->payload));

	pRecv->req_serial = HTTP_REQ_EMPTY_SERIAL;

	_http_recv_write(pRecv->socket, pMbuf);
}

static void
_http_recv_safe_read(MSGBODY *msg)
{
	SocketRead *pRead = (SocketRead *)(msg->msg_body);
	HTTPRecv *pRecv;

	HTTPDEBUG("socket:%d data_len:%d", pRead->socket, pRead->data_len);

	pRecv = &_http_recv[pRead->socket % HTTP_RECV_MAX];

	pRecv->ptr = _http_recv_recv_to_ptr(pRecv);

	SAFECODEv1(pRecv->opt_pv, _http_recv_read(pRecv, pRead,  msg->msg_build_serial); );

	dave_mfree(pRead->data);
}

static void
_http_recv_safe_rsp(MSGBODY *msg)
{
	HTTPRecvRsp *pRsp = (HTTPRecvRsp *)(msg->msg_body);
	HTTPRecv *pRecv;

	HTTPDEBUG("content length:%d", pRsp->content->len);

	pRecv = _http_recv_ptr_to_recv(pRsp->ptr);
	if(pRecv == NULL)
	{
		HTTPLOG("%s->%s:%d error rsp, maybe timer out! time:%dus ptr:%lx",
			thread_name(msg->msg_src), thread_name(msg->msg_dst),
			msg->msg_id,
			(dave_os_time_us() - pRsp->local_creat_time),
			pRsp->ptr);
	}
	else
	{
		SAFECODEv1(pRecv->opt_pv, _http_recv_rsp(pRecv, pRsp); );
	}

	dave_mfree(pRsp->content);
}

// =====================================================================

void
http_recv_data_init(void)
{
	_socket_thread_id = INVALID_SOCKET_ID;

	_http_recv_data_reset_all();

	t_lock_reset(&_http_req_serial_pv);
	_http_req_serial = 0;

	reg_msg(SOCKET_READ, _http_recv_safe_read);
	reg_msg(HTTPMSG_RECV_RSP, _http_recv_safe_rsp);
}

void
http_recv_data_exit(void)
{
	unreg_msg(SOCKET_READ);
	unreg_msg(HTTPMSG_RECV_RSP);
}

void
http_recv_data_plugin(s32 socket, ub nginx_port, ub msg_serial)
{
	HTTPRecv *pRecv;

	HTTPDEBUG("socket:%d", socket);

	pRecv = &_http_recv[socket % HTTP_RECV_MAX];

	SAFECODEv1(pRecv->opt_pv, _http_recv_data_plugin(pRecv, socket, nginx_port, msg_serial); );
}

void
http_recv_data_plugout(s32 socket)
{
	HTTPRecv *pRecv;

	HTTPDEBUG("socket:%d", socket);

	pRecv = &_http_recv[socket % HTTP_RECV_MAX];

	SAFECODEv1(pRecv->opt_pv, _http_recv_data_plugout(pRecv, socket); );
}

