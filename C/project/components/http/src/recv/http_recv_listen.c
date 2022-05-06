/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_http.h"
#include "dave_3rdparty.h"
#include "http_recv.h"
#include "http_recv_param.h"
#include "http_fastcgi.h"
#include "http_tools.h"
#include "http_recv_listen.h"
#include "http_recv_data.h"
#include "http_log.h"

#define CFG_SSL_CERTIFICATE_PATH "ssl_certificate"
#define CFG_SSL_CERTIFICATE_KEY_PATH "ssl_certificate_key"

#define RECV_LISTEN_MAX (64)
#define RECV_CGI_LISTEN_START (10000)

static void _http_recv_listen_bind_req(ub port);

typedef struct {
	ub nginx_port;
	HTTPListenType nginx_type;
	s8 nginx_path[DAVE_PATH_LEN];
	dave_bool nginx_booting;

	ub cgi_listen_port;
	s32 cgi_listen_socket;

	s8 listen_thread[DAVE_THREAD_NAME_LEN];
} RecvListen;

static TLock _recv_listen_pv;
static TLock _recv_api_pv;
static volatile ub _cgi_listen_port = RECV_CGI_LISTEN_START;
static RecvListen _recv_listen[RECV_LISTEN_MAX];
static s8 _ssl_certificate_path[256];
static s8 _ssl_certificate_key_path[256];

static ub
_http_recv_listen_port(void)
{
	if((_cgi_listen_port >= 65535) || (_cgi_listen_port == 0))
	{
		_cgi_listen_port = RECV_CGI_LISTEN_START;
	}

	return _cgi_listen_port ++;
}

static void
_http_recv_listen_reset(RecvListen *pListen)
{
	ub cgi_listen_port = pListen->cgi_listen_port;

	dave_memset(pListen, 0x00, sizeof(RecvListen));

	pListen->nginx_port = 0;
	pListen->nginx_type = ListenHttp;
	pListen->nginx_path[0] = '\0';
	pListen->nginx_booting = dave_false;

	pListen->cgi_listen_port = cgi_listen_port;
	pListen->cgi_listen_socket = INVALID_SOCKET_ID;
}

static void
_http_recv_listen_reset_all(void)
{
	ub listen_index;

	for(listen_index=0; listen_index<RECV_LISTEN_MAX; listen_index++)
	{
		_recv_listen[listen_index].cgi_listen_port = 0;

		_http_recv_listen_reset(&_recv_listen[listen_index]);
	}
}

static void
_http_recv_listen_cfg_load(void)
{
	if(cfg_get(CFG_SSL_CERTIFICATE_PATH, (u8 *)_ssl_certificate_path, sizeof(_ssl_certificate_path)) == dave_false)
	{
		dave_strcpy(_ssl_certificate_path, "/dave/tools/nginx/key/214602082670658.pem", sizeof(_ssl_certificate_path));
		cfg_set(CFG_SSL_CERTIFICATE_PATH, (u8 *)_ssl_certificate_path, dave_strlen(_ssl_certificate_path));
	}
	if(cfg_get(CFG_SSL_CERTIFICATE_KEY_PATH, (u8 *)_ssl_certificate_key_path, sizeof(_ssl_certificate_key_path)) == dave_false)
	{
		dave_strcpy(_ssl_certificate_key_path, "/dave/tools/nginx/key/214602082670658.key", sizeof(_ssl_certificate_key_path));
		cfg_set(CFG_SSL_CERTIFICATE_KEY_PATH, (u8 *)_ssl_certificate_key_path, dave_strlen(_ssl_certificate_key_path));
	}
}

static RecvListen *
_http_recv_listen_port_to_ptr(ub cgi_listen_port)
{
	ub listen_index;

	if((cgi_listen_port >= 65535) || (cgi_listen_port == 0))
	{
		return NULL;
	}

	for(listen_index=0; listen_index<RECV_LISTEN_MAX; listen_index++)
	{
		if(_recv_listen[listen_index].cgi_listen_port == cgi_listen_port)
		{
			return &_recv_listen[listen_index];
		}
	}

	return NULL;
}

static RecvListen *
_http_recv_listen_find(ub nginx_port, dave_bool find_new)
{
	ub listen_index;

	for(listen_index=0; listen_index<RECV_LISTEN_MAX; listen_index++)
	{
		if(((find_new == dave_true) && (_recv_listen[listen_index].nginx_port == 0))
			|| (_recv_listen[listen_index].nginx_port == nginx_port))
		{
			return &_recv_listen[listen_index];
		}
	}

	return NULL;
}

static RetCode
_http_recv_listen_start(ub nginx_port, HTTPListenType type, ub cgi_port, s8 *nginx_path, s8 *pem_path, s8 *key_path)
{
	RetCode ret = RetCode_Resource_conflicts;

	SAFECODEv1(_recv_api_pv, { ret = dave_nginx_start(nginx_port, type, cgi_port, nginx_path, pem_path, key_path); } );

	return ret;
}

static RetCode
_http_recv_listen_stop(ub nginx_port)
{
	RetCode ret = RetCode_Resource_conflicts;

	SAFECODEv1(_recv_api_pv, { ret = dave_nginx_stop(nginx_port); } );

	return ret;
}

static void
_http_recv_listen_bind_rsp(MSGBODY *ptr)
{
	SocketBindRsp *pRsp = (SocketBindRsp *)(ptr->msg_body);
	RecvListen *pListen;
	RetCode ret;

	pListen = _http_recv_listen_port_to_ptr(pRsp->NetInfo.port);
	if(pListen == NULL)
	{
		HTTPABNOR("the port:%d not being listen!", pRsp->NetInfo.port);
		return;
	}

	switch(pRsp->BindInfo)
	{
		case SOCKETINFO_BIND_OK:
				pListen->cgi_listen_socket = pRsp->socket;
				ret = _http_recv_listen_start(
					pListen->nginx_port, pListen->nginx_type, pListen->cgi_listen_port, pListen->nginx_path,
					_ssl_certificate_path, _ssl_certificate_key_path);
				if(ret == RetCode_OK)
				{
					pListen->nginx_booting = dave_true;

					HTTPTRACE("%s listen on socket:%d cgi-port:%d nginx-port:%d listen_type:%d nginx-path:%s",
						pListen->listen_thread,
						pListen->cgi_listen_socket,
						pRsp->NetInfo.port,
						pListen->nginx_port, pListen->nginx_type,
						pListen->nginx_path);
				}
			break;
		default:
				pListen->cgi_listen_port = _http_recv_listen_port();
				HTTPLOG("the port:%d listen failed:%d, then listen:%d nginx-port:%d",
					pRsp->NetInfo.port, pRsp->BindInfo, pListen->cgi_listen_port,
					pListen->nginx_port);
				_http_recv_listen_bind_req(pListen->cgi_listen_port);
			break;
	}
}

static void
_http_recv_listen_bind_req(ub port)
{
	SocketBindReq *pReq;

	if(port == 0)
	{
		HTTPABNOR("can't bind zero port!");
		return;
	}

	if(base_power_state() == dave_false)
	{
		return;
	}

	HTTPDEBUG("port:%d", port);

	pReq = thread_reset_msg(pReq);

	pReq->NetInfo.domain = DM_SOC_PF_INET;
	pReq->NetInfo.type = TYPE_SOCK_STREAM;
	pReq->NetInfo.addr_type = NetAddrIPType;
	pReq->NetInfo.addr.ip.ver = IPVER_IPV4;
	pReq->NetInfo.addr.ip.ip_addr[0] = 127;
	pReq->NetInfo.addr.ip.ip_addr[1] = 0;
	pReq->NetInfo.addr.ip.ip_addr[2] = 0;
	pReq->NetInfo.addr.ip.ip_addr[3] = 1;
	pReq->NetInfo.port = port;
	pReq->NetInfo.fixed_src_flag = NotFixedPort;
	pReq->NetInfo.enable_keepalive_flag = KeepAlive_disable;
	pReq->NetInfo.netcard_bind_flag = NetCardBind_disable;

	id_event(thread_id(SOCKET_THREAD_NAME), SOCKET_BIND_REQ, pReq, SOCKET_BIND_RSP, _http_recv_listen_bind_rsp);
}

static void
_http_recv_listen_disconnect_rsp(MSGBODY *ptr)
{
	SocketDisconnectRsp *pRsp = (SocketDisconnectRsp *)(ptr->msg_body);

	if(pRsp->result != SOCKETINFO_DISCONNECT_OK)
	{
		HTTPABNOR("socket:%d disconnect failed:%d!", pRsp->socket, pRsp->result);
	}
}

static void
_http_recv_listen_disconnect_req(s32 socket)
{
	SocketDisconnectReq *pReq;

	if(socket == INVALID_SOCKET_ID)
	{
		return;
	}


	pReq = thread_msg(pReq);

	HTTPDEBUG("socket:%d", socket);

	pReq->socket = socket;

	id_event(thread_id(SOCKET_THREAD_NAME), SOCKET_DISCONNECT_REQ, pReq, SOCKET_DISCONNECT_RSP, _http_recv_listen_disconnect_rsp);
}

static void
_http_recv_listen_plugin(MSGBODY *msg)
{
	SocketPlugIn *pPlugin = (SocketPlugIn *)(msg->msg_body);
	RecvListen *pListen;

	HTTPDEBUG("the listen port:%d!", pPlugin->NetInfo.port);

	pListen = _http_recv_listen_port_to_ptr(pPlugin->NetInfo.src_port);
	if(pListen == NULL)
	{
		HTTPABNOR("the port:%d/%d not on listen! socket:%d",
			pPlugin->NetInfo.src_port,
			pPlugin->NetInfo.port,
			pPlugin->child_socket);
	}
	else
	{
		if(pListen->cgi_listen_socket == pPlugin->father_socket)
		{
			http_recv_data_plugin(pPlugin->child_socket, pListen->nginx_port, msg->msg_build_serial);
		}
		else
		{
			HTTPABNOR("father socket mismatch! socket:%d/%d port:%d/%d",
				pListen->cgi_listen_socket, pPlugin->father_socket,
				pListen->cgi_listen_port, pPlugin->NetInfo.src_port);
		}
	}
}

static void
_http_recv_listen_plugout(MSGBODY *msg)
{
	SocketPlugOut *pPlugout = (SocketPlugOut *)(msg->msg_body);
	RecvListen *pListen;

	pListen = _http_recv_listen_port_to_ptr(pPlugout->NetInfo.src_port);
	if(pListen != NULL)
	{
		HTTPDEBUG("port:%d", pPlugout->NetInfo.src_port);

		http_recv_data_plugout(pPlugout->socket);
	}
	else
	{
		pListen = _http_recv_listen_port_to_ptr(pPlugout->NetInfo.port);
		if(pListen != NULL)
		{
			_http_recv_listen_stop(pListen->nginx_port);

			_http_recv_listen_reset(pListen);

			if(pListen->cgi_listen_socket == pPlugout->socket)
			{
				HTTPABNOR("socket:%d nginx_port:%d cgi-port:%d lost!",
					pPlugout->socket,
					pListen->nginx_port,
					pListen->cgi_listen_port);
		
				_http_recv_listen_bind_req(pListen->cgi_listen_port);
			}
			else
			{
				pListen->cgi_listen_port = 0;

				HTTPTRACE("father socket close! socket:%d/%d port:%d/%d",
					pListen->cgi_listen_socket, pPlugout->socket,
					pListen->cgi_listen_port, pPlugout->NetInfo.src_port);
			}
		}
		else
		{
			HTTPABNOR("can't find the listen port:%d!", pPlugout->NetInfo.port);
		}
	}
}

static RetCode
_http_recv_listen_bind(RecvListen *pListen)
{
	_http_recv_listen_bind_req(pListen->cgi_listen_port);

	return RetCode_OK;
}

static RetCode
_http_recv_listen_unbind(RecvListen *pListen)
{
	s32 cgi_listen_socket = pListen->cgi_listen_socket;

	pListen->cgi_listen_socket = INVALID_SOCKET_ID;

	_http_recv_listen_disconnect_req(cgi_listen_socket);

	return RetCode_OK;
}

static void
_http_recv_listen_recycling(void)
{
	ub listen_index;
	RecvListen *pListen;

	for(listen_index=0; listen_index<RECV_LISTEN_MAX; listen_index++)
	{
		pListen = &_recv_listen[listen_index];
		if(pListen->nginx_booting == dave_true)
		{
			HTTPLOG("recycling nginx-port:%d cgi-listen-port:%d", pListen->nginx_port, pListen->cgi_listen_port);

			_http_recv_listen_unbind(pListen);
		}
	}
}

static RetCode 
_http_recv_location_rule_match(HTTPMathcRule rule, s8 *path, s8 *location)
{
	if ((NULL == path) && (rule == LocationMatch_Accurate))
	{
		HTTPABNOR("Invalid parameter, rule:%d path:%x", rule, path);
		return RetCode_Invalid_parameter;
	}
	
	switch (rule){
		case LocationMatch_Accurate:
				dave_sprintf(location, " = %s ", path);
			break;
		case LocationMatch_Prefix:
				if (NULL == path){
					dave_sprintf(location, " ^~ %s", "/static/ ");
				}else{
					dave_sprintf(location, " ^~ %s", path);
				}
			break;
		case LocationMatch_CaseRegular:
				if (NULL == path){
					dave_sprintf(location, " ~* %s", "\\.png$ ");
				}else{
					dave_sprintf(location, " ~* %s", path);
				}
			break;
		case LocationMatch_Regular:
				if (NULL == path){
					dave_sprintf(location, " ~ %s", "\\.(gif|jpg|png|js|css)$ ");
				}else{
					dave_sprintf(location, " ~ %s", path);
				}
			break;
		case LocationMatch_CaseRegularExcl:
				if (NULL == path){
					dave_sprintf(location, " !~* %s", "\\.xhtml$ ");
				}else{
					dave_sprintf(location, " !~* %s", path);
				}
			break;
		case LocationMatch_RegularExcl:
				if (NULL == path){
					dave_sprintf(location, " !~ %s", "\\.xhtml$ ");
				}else{
					dave_sprintf(location, " !~ %s", path);
				}
			break;
		case LocationMatch_Wildcard:
				dave_sprintf(location, " / ");
			break;
		default:
				HTTPABNOR("Invalid match rule[%d]!", rule);
				return RetCode_Invalid_parameter;
			break;
	}
	
	HTTPTRACE("location:%s", location);
	
	return RetCode_OK;
}

static RetCode
_http_recv_listen_action(s8 *listen_thread, ub port, HTTPListenType type, HTTPMathcRule rule, s8 *path)
{
	s8 location[DAVE_PATH_LEN];
	RecvListen *pListen;
	RetCode ret;

	if((port == 0) || (port >= 65536))
	{
		HTTPABNOR("Invalid parameter, port:%d path:%x", port, path);
		return RetCode_Invalid_parameter;
	}

	dave_memset(location, 0x00, sizeof(location));

	_http_recv_location_rule_match(rule, path, location);

	pListen = _http_recv_listen_find(port, dave_false);
	if(pListen != NULL)
	{
		if ((pListen->nginx_port == port)
			&& (pListen->nginx_type == type)
			&&	(dave_strcmp(pListen->nginx_path, location) == dave_true))
		{
			HTTPTRACE("%s/%s repeat listening port:%d https:%d path:%s",
				listen_thread, pListen->listen_thread,
				port, type, path);

			dave_strcpy(pListen->listen_thread, listen_thread, sizeof(pListen->listen_thread));
			return RetCode_repeated_request;
		}
	}

	pListen = _http_recv_listen_find(port, dave_true);
	if(pListen == NULL)
	{
		HTTPABNOR("limited resources! port:%d", port);
		return RetCode_Limited_resources;
	}
	
	pListen->nginx_port = port;
	pListen->nginx_type = type;
	dave_strcpy(pListen->nginx_path, location, DAVE_PATH_LEN);
	pListen->nginx_booting = dave_false;

	pListen->cgi_listen_port = _http_recv_listen_port();
	pListen->cgi_listen_socket = INVALID_SOCKET_ID;
	
	dave_strcpy(pListen->listen_thread, listen_thread, sizeof(pListen->listen_thread));

	ret = _http_recv_listen_bind(pListen);
	if(ret != RetCode_OK)
	{
		_http_recv_listen_reset(pListen);
	}

	HTTPDEBUG("%s port:%d(%s) cgi_listen_port:%d path:%s bind %s",
		listen_thread,
		port, t_auto_HTTPListenType_str(type),
		pListen->cgi_listen_port,
		path,
		retstr(ret));

	return ret;
}

static RetCode
_http_recv_listen_close(ThreadId src, ub port)
{
	RecvListen *pListen;

	pListen = _http_recv_listen_find(port, dave_false);
	if(pListen == NULL)
	{
		HTTPTRACE("port:%d already closed!", port);
		return RetCode_OK;
	}

	return _http_recv_listen_unbind(pListen);
}

// =====================================================================

void
http_recv_listen_init(void)
{
	t_lock_reset(&_recv_listen_pv);
	t_lock_reset(&_recv_api_pv);
	_cgi_listen_port = RECV_CGI_LISTEN_START;
	_http_recv_listen_reset_all();
	_http_recv_listen_cfg_load();

	reg_msg(SOCKET_PLUGIN, _http_recv_listen_plugin);
	reg_msg(SOCKET_PLUGOUT, _http_recv_listen_plugout);
}

void
http_recv_listen_exit(void)
{
	_http_recv_listen_recycling();
}

RetCode
http_recv_listen_action(ThreadId src, ub port, HTTPListenType type, HTTPMathcRule rule, s8 *path)
{
	s8 listen_thread[DAVE_THREAD_NAME_LEN];
	RetCode ret = RetCode_Resource_conflicts;

	dave_strcpy(listen_thread, thread_name(src), sizeof(listen_thread));

	if((dave_strcmp(listen_thread, "NULL") == dave_true)
		|| (port == 0)
		|| (type >= ListenMax)
		|| (path == NULL)
		|| (path[0] == '\0'))
	{
		HTTPLOG("invalid param! src:%s port:%d type:%d path:%s", thread_name(src), port, type, path);
		return RetCode_Invalid_parameter;
	}

	HTTPDEBUG("src:%s port:%d", thread_name(src), port);

	SAFECODEv1(_recv_listen_pv, { ret = _http_recv_listen_action(listen_thread, port, type, rule, path); } );

	return ret;
}

RetCode
http_recv_listen_close(ThreadId src, ub port)
{
	RetCode ret;

	HTTPDEBUG("src:%s port:%d", thread_name(src), port);

	SAFECODEv1(_recv_listen_pv, { ret = _http_recv_listen_close(src, port); } );

	return ret;
}

void
http_recv_listen_release(s32 socket)
{
	_http_recv_listen_disconnect_req(socket);
}

s8 *
http_recv_listen_thread(ub cgi_port)
{
	RecvListen *pListen;

	pListen = _http_recv_listen_port_to_ptr(cgi_port);
	if(pListen == NULL)
	{
		HTTPABNOR("cgi_port:%d not listen!", cgi_port);
		return NULL;
	}

	return pListen->listen_thread;
}

ub
http_recv_listen_port(ub cgi_port)
{
	RecvListen *pListen;

	pListen = _http_recv_listen_port_to_ptr(cgi_port);
	if(pListen == NULL)
	{
		HTTPABNOR("cgi_port:%d not listen!", cgi_port);
		return 0;
	}

	return pListen->nginx_port;
}

