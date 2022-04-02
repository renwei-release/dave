/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_CLIENT) || defined(SYNC_STACK_SERVER)
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"

#define SYNC_SERVICE_PORT 6004

// =====================================================================

void
sync_cfg_get_syncs_ip(u8 ip[DAVE_IP_V4_ADDR_LEN])
{
	if(cfg_get(CFG_SYNC_ADDRESS, ip, DAVE_IP_V4_ADDR_LEN) == dave_false)
	{
		s8 *ip_str = (s8 *)t_gp_localhost();

		if(strip(ip_str, dave_strlen(ip_str), ip, DAVE_IP_V4_ADDR_LEN) != DAVE_IP_V4_ADDR_LEN)
		{
			ip[0] = 127;
			ip[1] = 0;
			ip[2] = 0;
			ip[3] = 1;
		}

		cfg_set(CFG_SYNC_ADDRESS, ip, DAVE_IP_V4_ADDR_LEN);
	}
}

u16
sync_cfg_get_syncs_port(void)
{
	u16 port;

	if(cfg_get(CFG_SYNC_PORT, (u8 *)(&(port)), sizeof(u16)) == dave_false)
	{
		port = SYNC_SERVICE_PORT;

		cfg_set(CFG_SYNC_PORT, (u8 *)(&(port)), sizeof(u16));
	}

	return port;
}

dave_bool
sync_cfg_get_local_ip(u8 ip[DAVE_IP_V4_ADDR_LEN])
{
	s8 ip_str[128];
	dave_bool real_cfg = dave_false;

	dave_memset(ip_str, 0x00, sizeof(ip_str));

	if((cfg_get(CFG_SYNC_CLIENT_ADDRESS, (u8 *)ip_str, sizeof(ip_str)) == dave_false)
		|| (strip(ip_str, dave_strlen(ip_str), ip, DAVE_IP_V4_ADDR_LEN) < DAVE_IP_V4_ADDR_LEN))
	{
		if(dave_os_on_docker() == dave_false)
		{
			dave_os_load_ip(ip, NULL);

			ipstr(ip, DAVE_IP_V4_ADDR_LEN, ip_str, sizeof(ip_str));
		}
		else
		{
			dave_strcpy(ip_str, t_gp_localhost(), sizeof(ip_str));
		}
	}
	else
	{
		real_cfg = dave_true;
	}

	if(strip(ip_str, dave_strlen(ip_str), ip, DAVE_IP_V4_ADDR_LEN) < DAVE_IP_V4_ADDR_LEN)
	{
		base_restart("system core error! invalid local ip:%s", ip_str);
	}

	return real_cfg;
}

#endif

