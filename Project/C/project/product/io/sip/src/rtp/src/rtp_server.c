/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_base.h"
#include "dave_tools.h"
#include "rtp_log.h"

static s32
_rtp_server_bind(s8 *port)
{
	SocketBindReq *pReq = thread_msg(pReq);
	SocketBindRsp *pRsp;

	pReq->NetInfo.domain = DM_SOC_PF_INET;
	pReq->NetInfo.type = TYPE_SOCK_DGRAM;
	pReq->NetInfo.addr_type = NetAddrIPType;
	pReq->NetInfo.addr.ip.ver = IPVER_IPV4;
	dave_memset(pReq->NetInfo.addr.ip.ip_addr, 0x00, 16);
	pReq->NetInfo.port = stringdigital(port);
	pReq->NetInfo.fixed_src_flag = NotFixedPort;
	pReq->NetInfo.enable_keepalive_flag = KeepAlive_disable;
	pReq->NetInfo.netcard_bind_flag = NetCardBind_disable;

	pRsp = (SocketBindRsp *)name_co(SOCKET_THREAD_NAME, SOCKET_BIND_REQ, pReq, SOCKET_BIND_RSP);
	if(pRsp == NULL)
	{
		RTPLOG("bind failed!");
		return INVALID_SOCKET_ID;
	}

	RTPLOG("%s socket:%d", ipv4str(pReq->NetInfo.addr.ip.ip_addr, pReq->NetInfo.port), pRsp->socket);

	return pRsp->socket;
}

static void
_rtp_server_disconnect(s32 socket)
{
	SocketDisconnectReq *pReq = thread_msg(pReq);
	SocketDisconnectRsp *pRsp;

	pReq->socket = socket;

	pRsp = (SocketDisconnectRsp *)name_co(SOCKET_THREAD_NAME, SOCKET_DISCONNECT_REQ, pReq, SOCKET_DISCONNECT_RSP);

	RTPLOG("socket:%d result:%d", pRsp->socket, pRsp->result);
}

// =====================================================================

s32
rtp_server_creat(s8 *port)
{
	return _rtp_server_bind(port);
}

void
rtp_server_release(s32 socket)
{
	_rtp_server_disconnect(socket);
}

