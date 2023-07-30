/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_LINUX__
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h> 
#include <netinet/in.h>
#include <netinet/ip.h>
#include <net/if.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <sys/sysinfo.h>
#include "dave_os.h"
#include "dave_tools.h"
#include "os_log.h"

static u8 _mac[DAVE_MAC_ADDR_LEN];
static dave_bool _mac_flag = dave_false;
static u8 _ip_v4[DAVE_IP_V4_ADDR_LEN];
static u8 _ip_v6[DAVE_IP_V6_ADDR_LEN];
static dave_bool _ip_flag = dave_false;
static s8 _hostname[64];
static dave_bool _hostname_flag = dave_false;

static RetCode
_os_load_one_netcard_mac(char *netcard_name, u8 *mac)
{
	int sock;
	struct ifreq ifr;
	RetCode ret;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1)
	{
		return RetCode_invalid_option;
	}

	strcpy(ifr.ifr_name, netcard_name);

	if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0)
	{
		ret = RetCode_invalid_option;
	}
	else
	{
		mac[0] = (u8)ifr.ifr_hwaddr.sa_data[0];
		mac[1] = (u8)ifr.ifr_hwaddr.sa_data[1];
		mac[2] = (u8)ifr.ifr_hwaddr.sa_data[2];
		mac[3] = (u8)ifr.ifr_hwaddr.sa_data[3];
		mac[4] = (u8)ifr.ifr_hwaddr.sa_data[4];
		mac[5] = (u8)ifr.ifr_hwaddr.sa_data[5];

		ret = RetCode_OK;
	}

	close(sock);

	return ret;
}

static RetCode
_os_load_some_netcard_mac(u8 *mac)
{
	if(_os_load_one_netcard_mac("eth0", mac) == RetCode_OK)
		return RetCode_OK;

	if(_os_load_one_netcard_mac("ens33", mac) == RetCode_OK)
		return RetCode_OK;

	if(_os_load_one_netcard_mac("em3", mac) == RetCode_OK)
		return RetCode_OK;

	return RetCode_invalid_option;
}

static RetCode
_os_load_on_loop_mac(u8 *mac)
{
	int sock;
	int i, index;
	struct ifreq ifr;

	dave_memset(mac, 0, DAVE_MAC_ADDR_LEN);

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1)
	{
		return RetCode_invalid_option;
	}

	for (index = 1; index < 64; index++)
	{
		dave_memset(&ifr, 0, sizeof(ifr));

		ifr.ifr_ifindex = index;

		if (ioctl(sock, SIOCGIFNAME, &ifr) < 0)
		{
			break;
		}

		if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0)
		{
			continue;
		}

		if((ifr.ifr_flags & IFF_UP) != IFF_UP)
		{
			continue;
		}

		if ((ifr.ifr_flags & IFF_LOOPBACK) == IFF_LOOPBACK)
		{
			continue;
		}

		if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0)
		{
			continue;
		}

		dave_memcpy(mac, (u8 *)ifr.ifr_hwaddr.sa_data, DAVE_MAC_ADDR_LEN);

		for (i = 0; i < DAVE_MAC_ADDR_LEN; i++)
		{
			if (mac[i] != 0)
			{
				close(sock);

				return RetCode_OK;
			}
		}
	}

	close(sock);

	return RetCode_Can_not_find_node;
}

static RetCode
_os_load_mac(u8 *mac)
{
	if(_os_load_some_netcard_mac(mac) != RetCode_OK)
	{
		if(_os_load_on_loop_mac(mac) != RetCode_OK)
		{
			OSABNOR("Can't find MAC address!");
		
			return RetCode_Can_not_find_node;
		}
	}

	return RetCode_OK;
}

/*
 * 可以从这个文件<include/linux/sockios.h>，获取ioctl能传输的标记。
 */

static RetCode
_os_load_ip(u8 ip_v4[DAVE_IP_V4_ADDR_LEN], u8 ip_v6[DAVE_IP_V6_ADDR_LEN])
{
	int index, sock;
	struct ifreq ifr;
	RetCode ret = RetCode_Can_not_find_node;

	dave_memset(ip_v4, 0x00, DAVE_IP_V4_ADDR_LEN);
	dave_memset(ip_v6, 0x00, DAVE_IP_V6_ADDR_LEN);

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1)
	{
		return RetCode_invalid_option;
	}

	for (index = 1; index < 16; index++)
	{
		dave_memset(&ifr, 0x00, sizeof(ifr));
		ifr.ifr_ifindex = index;

		if (ioctl(sock, SIOCGIFNAME, &ifr) < 0)
		{
			continue;
		}

		if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0)
		{
			continue;
		}

		if((ifr.ifr_flags & IFF_LOOPBACK) || (ifr.ifr_flags != 4163))
		{
			continue;
		}

		if (ioctl(sock, SIOCGIFADDR, &ifr) < 0)
		{
			continue;
		}

		ip_v4[0] = (u8)((((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr.s_addr));
		ip_v4[1] = (u8)((((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr.s_addr) >> 8);
		ip_v4[2] = (u8)((((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr.s_addr) >> 16);
		ip_v4[3] = (u8)((((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr.s_addr) >> 24);

		if (ioctl(sock, SIOCGIFMAP, &ifr) < 0)
		{
			continue;
		}		

		if((ifr.ifr_map.mem_start != 0) && (ifr.ifr_map.mem_end != 0))
		{
			break;
		}
	}

	close(sock);

	return ret;
}

static RetCode
_os_load_host_name(s8 *hostname, ub hostname_len)
{
	dave_memset(hostname, 0x00, hostname_len);

	if(hostname_len <= 1)
	{
		return RetCode_Invalid_data_too_short;
	}

	hostname_len --;

	if(gethostname((char *)hostname, hostname_len) == 0)
	{
		return RetCode_OK;
	}
	else
	{
		return RetCode_Invalid_data;
	}
}

// =====================================================================

RetCode
dave_os_load_mac(u8 *mac)
{
	RetCode ret;

	if(_mac_flag == dave_false)
	{
		ret = _os_load_mac(_mac);

		if(ret == RetCode_OK)
		{
			_mac_flag = dave_true;
		}
	}
	else
	{
		ret = RetCode_OK;
	}

	dave_memcpy(mac, _mac, DAVE_MAC_ADDR_LEN);

	return ret;
}

RetCode
dave_os_load_ip(u8 ip_v4[DAVE_IP_V4_ADDR_LEN], u8 ip_v6[DAVE_IP_V6_ADDR_LEN])
{
	RetCode ret;

	if(_ip_flag == dave_false)
	{
		ret = _os_load_ip(_ip_v4, _ip_v6);

		if(ret == RetCode_OK)
		{
			_ip_flag = dave_true;
		}
	}
	else
	{
		ret = RetCode_OK;
	}

	if(ip_v4 != NULL)
	{
		dave_memcpy(ip_v4, _ip_v4, DAVE_IP_V4_ADDR_LEN);
	}
	if(ip_v6 != NULL)
	{
		dave_memcpy(ip_v6, _ip_v6, DAVE_IP_V6_ADDR_LEN);
	}

	return ret;
}

RetCode
dave_os_load_host_name(s8 *hostname, ub hostname_len)
{
	RetCode ret;

	if(_hostname_flag == dave_false)
	{
		ret = _os_load_host_name(_hostname, sizeof(_hostname));

		if(ret == RetCode_OK)
		{
			_hostname_flag = dave_true;
		}
	}
	else
	{
		ret = RetCode_OK;	
	}

	dave_strcpy(hostname, _hostname, hostname_len);

	return ret;
}

ub
dave_os_cpu_process_number(void)
{
	ub cpu_number = (ub)get_nprocs();

	if(cpu_number > 128)
	{
		OSABNOR("invalid cpu_number:%ld", cpu_number);
		cpu_number = 64;
	}

	return cpu_number;
}

ub
dave_os_memory_use_percentage(void)
{
	struct sysinfo s_info;

	if(sysinfo(&s_info) == 0)
	{
		unsigned long useram = s_info.totalram - s_info.freeram;
		if(useram > s_info.totalram)
		{
			OSABNOR("system memory overflow：%ld/%ld!", useram, s_info.totalram);
			return 100;
		}

		return (useram * 100) / s_info.totalram;
	}

	OSABNOR("can't get sysinfo!");
	return 0;
}

#endif

