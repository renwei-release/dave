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
_rtp_client_connect(s8 *server_ip, s8 *server_port, s8 *local_port)
{
	SocketConnectReq *pReq = thread_msg(pReq);
	SocketConnectRsp *pRsp;

	pReq->NetInfo.domain = DM_SOC_PF_INET;
	pReq->NetInfo.type = TYPE_SOCK_DGRAM;
	pReq->NetInfo.addr_type = NetAddrIPType;
	pReq->NetInfo.addr.ip.ver = IPVER_IPV4;
	domainip(pReq->NetInfo.addr.ip.ip_addr, &(pReq->NetInfo.port), server_ip);
	pReq->NetInfo.port = stringdigital(server_port);
	if(local_port == NULL)
	{
		pReq->NetInfo.fixed_src_flag = NotFixedPort;
		pReq->NetInfo.src_port = 0;
	}
	else
	{
		pReq->NetInfo.fixed_src_flag = FixedPort;
		pReq->NetInfo.src_port = stringdigital(local_port);
	}
	pReq->NetInfo.enable_keepalive_flag = KeepAlive_disable;
	pReq->NetInfo.netcard_bind_flag = NetCardBind_disable;

	RTPLOG("%s src_port:%d",
		ipv4str(pReq->NetInfo.addr.ip.ip_addr, pReq->NetInfo.port),
		pReq->NetInfo.src_port);

	pRsp = (SocketConnectRsp *)name_co(SOCKET_THREAD_NAME, SOCKET_CONNECT_REQ, pReq, SOCKET_CONNECT_RSP);
	if(pRsp == NULL)
	{
		RTPABNOR("bind failed!");
		return INVALID_SOCKET_ID;
	}

	return pRsp->socket;
}

static void
_rtp_client_disconnect(s32 socket)
{
	SocketDisconnectReq *pReq = thread_msg(pReq);
	SocketDisconnectRsp *pRsp;

	pReq->socket = socket;

	pRsp = (SocketDisconnectRsp *)name_co(SOCKET_THREAD_NAME, SOCKET_DISCONNECT_REQ, pReq, SOCKET_DISCONNECT_RSP);

	RTPLOG("socket:%d result:%d", pRsp->socket, pRsp->result);
}

// =====================================================================

s32
rtp_client_creat(s8 *server_ip, s8 *server_port, s8 *local_port)
{
	return _rtp_client_connect(server_ip, server_port, local_port);
}

void
rtp_client_release(s32 socket)
{
	_rtp_client_disconnect(socket);
}

