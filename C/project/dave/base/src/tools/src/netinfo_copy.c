/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "base_define.h"
#include "dave_tools.h"

// =====================================================================

void
__T_CopyNetInfo__(SocNetInfo *dst, SocNetInfo *src, s8 *file, ub line)
{
	if((src == NULL) || (dst == NULL))
	{
		return;
	}

	dst->domain = src->domain;
	if(src->type >= TYPE_SOCK_max)
	{
		src->type = TYPE_SOCK_STREAM;
	}
	dst->type = src->type;
	dst->addr_type = src->addr_type;
	if((dst->addr_type == NetAddrIPType) || (dst->addr_type == NetAddrIPBroadcastType))
	{
		dst->addr.ip.ver = src->addr.ip.ver;
		dave_memcpy(dst->addr.ip.ip_addr, src->addr.ip.ip_addr, sizeof(dst->addr.ip.ip_addr));
	}
	else
	{
		dave_strcpy(dst->addr.url, src->addr.url, DAVE_URL_LEN);
	}
	dst->port = src->port;
	dst->fixed_src_flag = src->fixed_src_flag;
	dst->src_ip.ver = src->src_ip.ver;
	dave_memcpy(dst->src_ip.ip_addr, src->src_ip.ip_addr, sizeof(dst->src_ip.ip_addr));
	dst->src_port = src->src_port;
	dst->enable_keepalive_flag = src->enable_keepalive_flag;
	dst->keepalive_second = src->keepalive_second;
	dst->netcard_bind_flag = src->netcard_bind_flag;
	if(dst->netcard_bind_flag == NetCardBind_enable)
	{
		dave_strcpy(dst->netcard_name, src->netcard_name, sizeof(dst->netcard_name));
	}
	else
	{
		dst->netcard_name[0] = '\0';
	}
}

void
T_NetToIPInfo(IPBaseInfo *pIPInfo, SocNetInfo *pSocInfo)
{
	if(pSocInfo->type == TYPE_SOCK_STREAM)
	{
		pIPInfo->protocol = IPProtocol_TCP;
	}
	else
	{
		pIPInfo->protocol = IPProtocol_UDP;
	}
	if((pSocInfo->domain == DM_SOC_PF_INET6) || (pSocInfo->domain == DM_SOC_PF_LOCAL_INET6))
	{
		pIPInfo->ver = IPVER_IPV6;
	}
	else
	{
		pIPInfo->ver = IPVER_IPV4;
	}
	if(pIPInfo->ver == IPVER_IPV4)
	{
		dave_memcpy(pIPInfo->src_ip, pSocInfo->addr.ip.ip_addr, 4);
		dave_memcpy(pIPInfo->dst_ip, pSocInfo->addr.ip.ip_addr, 4);
	}
	else
	{
		dave_memcpy(pIPInfo->src_ip, pSocInfo->addr.ip.ip_addr, 16);
		dave_memcpy(pIPInfo->dst_ip, pSocInfo->addr.ip.ip_addr, 16);
	}
	pIPInfo->src_port = pSocInfo->port;
	pIPInfo->dst_port = pSocInfo->port;
	if(pSocInfo->enable_keepalive_flag == KeepAlive_enable)
	{
		pIPInfo->keepalive_second = pSocInfo->keepalive_second;
	}
	else
	{
		pIPInfo->keepalive_second = -1;
	}
	if(pSocInfo->netcard_bind_flag == NetCardBind_enable)
	{
		dave_strcpy(pIPInfo->netcard_name, pSocInfo->netcard_name, sizeof(pIPInfo->netcard_name));
	}
	else
	{
		pIPInfo->netcard_name[0] = '\0';
	}
	pIPInfo->fixed_port_flag = pSocInfo->fixed_src_flag;
}

#endif

