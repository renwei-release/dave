/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_CLIENT)
#include "dave_verno.h"
#include "dave_base.h"
#include "dave_os.h"
#include "dave_tools.h"
#include "sync_base_package.h"
#include "sync_param.h"
#include "sync_cfg.h"
#include "sync_client_tx.h"
#include "sync_client_param.h"
#include "sync_client_link.h"
#include "sync_client_data.h"
#include "sync_log.h"

#define SYNC_LINK_PORT 20000

static void _sync_client_link_server_bind_req(void);

static s32 _link_server_socket = INVALID_SOCKET_ID;
static u8 _link_server_cfg_my_ip[DAVE_IP_V6_ADDR_LEN];
static dave_bool _link_server_cfg_my_ip_flag = dave_false;
static u8 _link_server_detect_my_ip[DAVE_IP_V6_ADDR_LEN];
static dave_bool _link_server_detect_my_ip_flag = dave_false;
static u8 _link_server_sys_my_ip[DAVE_IP_V6_ADDR_LEN];
static u16 _link_server_port = SYNC_LINK_PORT;

static u16
_sync_client_link_port_generator(void)
{
	return (u16)(SYNC_LINK_PORT +  ((t_rand() + _link_server_port) % 9999));
}

static u8 *
_sync_client_link_my_ip(void)
{
	SYNCTRACE("cfg:%d.%d.%d.%d detect:%d.%d.%d.%d sys:%d.%d.%d.%d",
		_link_server_cfg_my_ip[0], _link_server_cfg_my_ip[1], _link_server_cfg_my_ip[2], _link_server_cfg_my_ip[3],
		_link_server_detect_my_ip[0], _link_server_detect_my_ip[1], _link_server_detect_my_ip[2], _link_server_detect_my_ip[3],
		_link_server_sys_my_ip[0], _link_server_sys_my_ip[1], _link_server_sys_my_ip[2], _link_server_sys_my_ip[3]);

	if(_link_server_cfg_my_ip_flag == dave_true)
	{
		return _link_server_cfg_my_ip;
	}
	else
	{
		if((_link_server_detect_my_ip_flag == dave_true) && (_link_server_detect_my_ip[0] != 127))
		{
			return _link_server_detect_my_ip;
		}
		else
		{
			return _link_server_sys_my_ip;
		}
	}
}

static void
_sync_client_link_tell_sync_server(void)
{
	if(_link_server_socket == INVALID_SOCKET_ID)
	{
		return;
	}

	if(_link_server_detect_my_ip_flag == dave_false)
	{
		return;
	}

	sync_client_tx_link_up_req(NULL, _sync_client_link_my_ip(), _link_server_port);
}

static void
_sync_client_link_server_bind_rsp(MSGBODY *pMsg)
{
	SocketBindRsp *bind_rsp = (SocketBindRsp *)(pMsg->msg_body);

	if(bind_rsp->BindInfo == SOCKETINFO_BIND_OK)
	{
		_link_server_socket = bind_rsp->socket;

		SYNCTRACE("socket:%d port:%d success!", _link_server_socket, _link_server_port);

		_sync_client_link_tell_sync_server();
	}
	else
	{
		SYNCTRACE("socket:%d info:%d", bind_rsp->socket, bind_rsp->BindInfo);

		_link_server_port = _sync_client_link_port_generator();

		_sync_client_link_server_bind_req();
	}
}

static void
_sync_client_link_server_bind_req(void)
{
	SocketBindReq *pReq;

	if((base_power_state() == dave_false)
		|| (_link_server_socket != INVALID_SOCKET_ID))
	{
		return;
	}

	if(_link_server_port == 0)
	{
		SYNCLOG("error bing port:%d, stop bind!", _link_server_port);
		return;
	}

	SYNCTRACE("port:%d", _link_server_port);

	pReq = thread_reset_msg(pReq);

	pReq->NetInfo.domain = DM_SOC_PF_RAW_INET;
	pReq->NetInfo.type = TYPE_SOCK_STREAM;
	pReq->NetInfo.addr_type = NetAddrIPType;
	pReq->NetInfo.addr.ip.ver = IPVER_IPV4;
	dave_memset(pReq->NetInfo.addr.ip.ip_addr, 0x00, 16);
	pReq->NetInfo.port = _link_server_port;
	pReq->NetInfo.fixed_src_flag = NotFixedPort;
	pReq->NetInfo.enable_keepalive_flag = KeepAlive_disable;
	pReq->NetInfo.netcard_bind_flag = NetCardBind_disable;

	name_event(SOCKET_THREAD_NAME, SOCKET_BIND_REQ, pReq, SOCKET_BIND_RSP, _sync_client_link_server_bind_rsp);
}

static void
_sync_client_link_server_ubind_req(void)
{
	SocketDisconnectReq *pReq;

	if(_link_server_socket == INVALID_SOCKET_ID)
	{
		return;
	}

	sync_client_tx_link_down_req(NULL, _sync_client_link_my_ip(), _link_server_port);

	pReq = thread_reset_msg(pReq);

	pReq->socket = _link_server_socket;
	pReq->ptr = NULL;

	name_msg(SOCKET_THREAD_NAME, SOCKET_DISCONNECT_REQ, pReq);
}

// =====================================================================

void
sync_client_link_init(void)
{
	_link_server_socket = INVALID_SOCKET_ID;

	dave_memset(_link_server_cfg_my_ip, 0x00, sizeof(_link_server_cfg_my_ip));
	_link_server_cfg_my_ip_flag = sync_cfg_get_local_ip(_link_server_cfg_my_ip);
	dave_memset(_link_server_detect_my_ip, 0x00, sizeof(_link_server_detect_my_ip));
	_link_server_detect_my_ip_flag = dave_false;
	dave_memset(_link_server_sys_my_ip, 0x00, sizeof(_link_server_sys_my_ip));
	dave_os_load_ip(_link_server_sys_my_ip, NULL);
	_link_server_port = _sync_client_link_port_generator();
}

void
sync_client_link_exit(void)
{
	sync_client_link_stop();
}

void
sync_client_link_start(void)
{
	_sync_client_link_server_bind_req();
}

void
sync_client_link_stop(void)
{
	_sync_client_link_server_ubind_req();
}

void
sync_client_link_up(s8 *verno, s8 *globally_identifier, u8 *ip, u16 port)
{
	sync_client_data_server_add_client(verno, globally_identifier, ip, port);
}

void
sync_client_link_down(s8 *verno, s8 *globally_identifier, u8 *ip, u16 port)
{
	sync_client_data_server_del_client(verno, globally_identifier, ip, port);
}

SyncServer *
sync_client_link_plugin(SocketPlugIn *pPlugIn)
{
	if((pPlugIn->father_socket == INVALID_SOCKET_ID)
		|| (_link_server_socket == INVALID_SOCKET_ID)
		|| (_link_server_socket != pPlugIn->father_socket))
	{
		return NULL;
	}

	SYNCTRACE("socket:%d/%d/%d port:%d",
		_link_server_socket,
		pPlugIn->father_socket, pPlugIn->child_socket,
		_link_server_port);

	return sync_client_data_server_add_child(pPlugIn->child_socket, pPlugIn->NetInfo.addr.ip.ip_addr, pPlugIn->NetInfo.port);
}

SyncServer *
sync_client_link_plugout(SocketPlugOut *pPlugOut)
{
	if(_link_server_socket == INVALID_SOCKET_ID)
	{
		return NULL;
	}

	SYNCTRACE("socket:%d/%d port:%d",
		_link_server_socket, pPlugOut->socket,
		_link_server_port);

	if(_link_server_socket == pPlugOut->socket)
	{
		sync_client_link_stop();

		_link_server_socket = INVALID_SOCKET_ID;

		sync_client_link_start();
	}
	else
	{
		return sync_client_data_server_del_child(pPlugOut->socket);
	}

	return NULL;
}

void
sync_client_link_tell_sync_server(u8 *detect_my_ip)
{
	dave_memcpy(_link_server_detect_my_ip, detect_my_ip, sizeof(_link_server_detect_my_ip));
	_link_server_detect_my_ip_flag = dave_true;

	_sync_client_link_tell_sync_server();
}

ub
sync_client_link_info(s8 *info_ptr, ub info_len)
{
	u8 *ip_cfg;
	u8 *ip_detect;
	u8 *ip_sys;
	u8 *ip_link = _sync_client_link_my_ip();
	ub info_index = 0;

	ip_cfg = _link_server_cfg_my_ip;
	ip_detect = _link_server_detect_my_ip;
	ip_sys = _link_server_sys_my_ip;

	info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
		"Link:%s%s%s (1.cfg:%d.%d.%d.%d 2.detect:%d.%d.%d.%d 3.sys:%d.%d.%d.%d->link:%s)\n",
		sync_client_data_get_busy() == dave_false ? "" : "***",
		sync_client_data_get_busy() == dave_false ? "Online" : "Offline",
		sync_client_data_get_busy() == dave_false ? "" : "***",
		ip_cfg[0], ip_cfg[1], ip_cfg[2], ip_cfg[3],
		ip_detect[0], ip_detect[1], ip_detect[2], ip_detect[3],
		ip_sys[0], ip_sys[1], ip_sys[2], ip_sys[3],
		ipv4str(ip_link, _link_server_port));

	return info_index;
}

#endif

