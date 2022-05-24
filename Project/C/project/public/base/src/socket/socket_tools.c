/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_os.h"
#include "dave_tools.h"
#include "dave_base.h"
#include "socket_tools.h"

// =====================================================================

void
CopyIPBaseInfo(IPBaseInfo *dst, IPBaseInfo *src)
{
	ub ip_len;

	if((dst == NULL) || (src == NULL))
	{
		return;
	}

	dst->protocol = src->protocol;
	dst->ver = src->ver;
	if(src->ver == IPVER_IPV4)
		ip_len = 4;
	else
		ip_len = 16;
	dave_memcpy(dst->src_ip, src->src_ip, ip_len);
	dst->src_port = src->src_port;
	dave_memcpy(dst->dst_ip, src->dst_ip, ip_len);
	dst->dst_port = src->dst_port;
	dst->keepalive_second = src->keepalive_second;
	dave_strcpy(dst->netcard_name, src->netcard_name, sizeof(dst->netcard_name));
	dst->fixed_port_flag = src->fixed_port_flag;
}

void
IPToNetInfo(SocNetInfo *pSocInfo, IPBaseInfo *pIPInfo, SOCDOMAIN domain, u8 *ip_addr, u16 port)
{	
	pSocInfo->domain = domain;
	if(pIPInfo->protocol == IPProtocol_TCP)	
		pSocInfo->type = TYPE_SOCK_STREAM;
	else
		pSocInfo->type = TYPE_SOCK_DGRAM;
	pSocInfo->addr_type = NetAddrIPType;
	if(pIPInfo->ver == IPVER_IPV4)
	{
		pSocInfo->addr.ip.ver = IPVER_IPV4;
		dave_memcpy(pSocInfo->addr.ip.ip_addr, ip_addr, 4);
	}
	else
	{
		pSocInfo->addr.ip.ver = IPVER_IPV6;
		dave_memcpy(pSocInfo->addr.ip.ip_addr, ip_addr, 16);
	}
	pSocInfo->port = port;
	pSocInfo->fixed_src_flag = NotFixedPort;
	pSocInfo->enable_keepalive_flag = KeepAlive_disable;
	pSocInfo->keepalive_second = -1;
	if(pIPInfo->netcard_name[0] == '\0')
		pSocInfo->netcard_bind_flag = NetCardBind_disable;
	else
		pSocInfo->netcard_bind_flag = NetCardBind_enable;
	dave_strcpy(&(pSocInfo->netcard_name), &(pIPInfo->netcard_name), sizeof(pSocInfo->netcard_name));
}

void
BuildIPInfo(IPBaseInfo *pIPInfo, IPVER ver, IPProtocol protocol, u8 *src_ip, u16 src_port, u8 *dst_ip, u16 dst_port)
{
	pIPInfo->protocol = protocol;
	pIPInfo->ver = ver;
	if(pIPInfo->ver == IPVER_IPV4)
	{
		dave_memcpy(pIPInfo->src_ip, src_ip, 4);
		dave_memcpy(pIPInfo->dst_ip, dst_ip, 4);
	}
	else
	{
		dave_memcpy(pIPInfo->src_ip, src_ip, 16);
		dave_memcpy(pIPInfo->dst_ip, dst_ip, 16);
	}
	pIPInfo->src_port = src_port;
	pIPInfo->dst_port = dst_port;
}

dave_bool
CmpMac(u8 *mac_1, u8 *mac_2)
{
	return dave_memcmp(mac_1, mac_2, DAVE_MAC_ADDR_LEN);
}

void
CpyMac(u8 *dst, u8 *src)
{
	dave_memcpy(dst, src, DAVE_MAC_ADDR_LEN);
}

dave_bool
ip_v4_broadcast(u8 ip[4])
{
	if((ip[0] == 0xff)
		&& (ip[1] == 0xff)
		&& (ip[2] == 0xff)
		&& (ip[3] == 0xff))
		return dave_true;

	return dave_false;
}

dave_bool
ip_v4_zero(u8 ip[4])
{
	if((ip[0] == 0x00)
		&& (ip[1] == 0x00)
		&& (ip[2] == 0x00)
		&& (ip[3] == 0x00))
		return dave_true;

	return dave_false;
}

dave_bool
ip_v4_loopback(u8 ip[4])
{
	if((ip[0] == 127)
		&& (ip[1] == 0)
		&& (ip[2] == 0)
		&& (ip[3] == 1))
		return dave_true;

	return dave_false;
}

dave_bool
ip_v4_same(u8 src_ip[4], u8 dst_ip[4])
{
	if((src_ip[0] == dst_ip[0])
		&& (src_ip[1] == dst_ip[1])
		&& (src_ip[2] == dst_ip[2])
		&& (src_ip[3] == dst_ip[3]))
		return dave_true;

	return dave_false;
}

dave_bool
ip_v4_in_same_subnet(u8 ip[4], u8 gw[4], u8 mask[4])
{
	u8 index;

	for(index=0; index<4; index++)
	{
		if(((ip[index])&(mask[index])) != ((gw[index])&(mask[index])))
			return dave_false;
	}
	return dave_true;
}

dave_bool
mac_broadcast(u8 *mac)
{
	if((mac[0] == 0xff)
		&& (mac[1] == 0xff)
		&& (mac[2] == 0xff)
		&& (mac[3] == 0xff)
		&& (mac[4] == 0xff)
		&& (mac[5] == 0xff))
		return dave_true;

	return dave_false;
}

dave_bool
mac_zero(u8 *mac)
{
	if((mac[0] == 0x00)
		&& (mac[1] == 0x00)
		&& (mac[2] == 0x00)
		&& (mac[3] == 0x00)
		&& (mac[4] == 0x00)
		&& (mac[5] == 0x00))
		return dave_true;

	return dave_false;
}

s8 *
socket_SOCKETTYPE_str(SOCKETTYPE type)
{
	s8 *type_str;

	switch(type)
	{
		case SOCKET_TYPE_SERVER_FATHER:
				type_str = (s8 *)"father";
			break;
		case SOCKET_TYPE_SERVER_CHILD:
				type_str = (s8 *)"child ";
			break;
		case SOCKET_TYPE_CLIENT:
				type_str = (s8 *)"client";
			break;
		case SOCKET_TYPE_CLIENT_WAIT:
				type_str = (s8 *)"client wait";
			break;
		default:
				type_str = (s8 *)"***EMPTY***";
			break;
	}

	return type_str;
}

#endif

