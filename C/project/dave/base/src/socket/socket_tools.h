/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SOCKET_TOOLS_H__
#define __SOCKET_TOOLS_H__
#include "socket_core.h"

void CopyIPBaseInfo(IPBaseInfo *dst, IPBaseInfo *src);

void IPToNetInfo(SocNetInfo *pSocInfo, IPBaseInfo *pIPInfo, SOCDOMAIN domain, u8 *ip_addr, u16 port);

void BuildIPInfo(IPBaseInfo *pIPInfo, IPVER ver, IPProtocol protocol, u8 *src_ip, u16 src_port, u8 *dst_ip, u16 dst_port);

dave_bool CmpMac(u8 *mac_1, u8 *mac_2);

void CpyMac(u8 *dst, u8 *src);

dave_bool ip_v4_broadcast(u8 ip[4]);

dave_bool ip_v4_zero(u8 ip[4]);

dave_bool ip_v4_loopback(u8 ip[4]);

dave_bool ip_v4_same(u8 src_ip[4], u8 dst_ip[4]);

dave_bool ip_v4_in_same_subnet(u8 ip[4], u8 gw[4], u8 mask[4]);

dave_bool mac_broadcast(u8 *mac);

dave_bool mac_zero(u8 *mac);

s8 * socket_SOCKETTYPE_str(SOCKETTYPE type);

#endif

