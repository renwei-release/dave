/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_CYGWIN__
#include <winsock2.h>
#include <sys/sysinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <iptypes.h>
#include <iphlpapi.h>
#include "dave_base.h"
#include "dave_tools.h"
#include "os_log.h"

static u8 _mac[DAVE_MAC_ADDR_LEN];
static dave_bool _mac_flag = dave_false;
static u8 _ip_v4[DAVE_IP_V4_ADDR_LEN];
static dave_bool _ip_flag = dave_false;

static RetCode
_os_load_mac_and_ip(u8 *mac, u8 ip_v4[DAVE_IP_V4_ADDR_LEN])
{
	PIP_ADAPTER_INFO ipAdapterInfo;
	PIP_ADAPTER_INFO ipAdapter = NULL;
	ULONG ulOutBuffer = sizeof(IP_ADAPTER_INFO);
	DWORD dwRetval = 0;

	ipAdapterInfo = malloc(ulOutBuffer);

	dwRetval = GetAdaptersInfo(ipAdapterInfo, &ulOutBuffer);
	if(dwRetval == ERROR_BUFFER_OVERFLOW)
	{
		free(ipAdapterInfo);
		ipAdapterInfo = malloc(ulOutBuffer);
		dwRetval = GetAdaptersInfo(ipAdapterInfo, &ulOutBuffer);
	}

	if (dwRetval == NO_ERROR)
	{
		ipAdapter = ipAdapterInfo;
		while (ipAdapter)
		{
			if(ipAdapter->Type == 71)
			{
				if(ipAdapter->IpAddressList.IpAddress.String[0] != '0')
				{
					if(mac != NULL)
					{
						dave_memcpy(mac, ipAdapter->Address, MAX_ADAPTER_ADDRESS_LENGTH);
					}

					if(ip_v4 != NULL)
					{
						strip(
							ipAdapter->IpAddressList.IpAddress.String, sizeof(ipAdapter->IpAddressList.IpAddress.String),
							ip_v4, DAVE_IP_V4_ADDR_LEN
						);
					}
				}
			}

			ipAdapter = ipAdapter->Next;
		}
	}

	free(ipAdapterInfo);

	return RetCode_OK;
}

// =====================================================================

RetCode
dave_os_load_mac(u8 *mac)
{
	if(_mac_flag == dave_false)
	{
		_os_load_mac_and_ip(_mac, NULL);
		_mac_flag = dave_true;
	}
	dave_memcpy(mac, _mac, DAVE_MAC_ADDR_LEN);
	return RetCode_OK;
}

RetCode
dave_os_load_ip(u8 ip_v4[DAVE_IP_V4_ADDR_LEN], u8 ip_v6[DAVE_IP_V6_ADDR_LEN])
{
	dave_memset(ip_v6, 0x00, DAVE_IP_V6_ADDR_LEN);
	if(_ip_flag == dave_false)
	{
		_os_load_mac_and_ip(NULL, _ip_v4);
		_ip_flag = dave_true;
	}
	dave_memcpy(ip_v4, _ip_v4, DAVE_IP_V4_ADDR_LEN);
	return RetCode_OK;
}

RetCode
dave_os_load_host_name(s8 *hostname, ub hostname_len)
{
	gethostname(hostname, hostname_len);
	return RetCode_OK;
}

ub
dave_os_cpu_process_number(void)
{
	ub cpu_number = (ub)get_nprocs_conf();

	return cpu_number;
}

#endif

