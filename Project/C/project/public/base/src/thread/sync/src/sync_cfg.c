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
#include "sync_cfg.h"
#include "sync_log.h"

#define SYNC_SERVICE_PORT 6004
#define SYNC_SERVER_DOMAIN "localhost:6004"

static void
_sync_cfg_get_syncs_ip(u8 ip[DAVE_IP_V4_ADDR_LEN])
{
	s8 *ip_str = (s8 *)t_gp_localhost();

	if(strip(ip_str, dave_strlen(ip_str), ip, DAVE_IP_V4_ADDR_LEN) != DAVE_IP_V4_ADDR_LEN)
	{
		ip[0] = 127;
		ip[1] = 0;
		ip[2] = 0;
		ip[3] = 1;
	}
}

static u16
_sync_cfg_get_syncs_port(void)
{
	return SYNC_SERVICE_PORT;
}

// =====================================================================

void
sync_cfg_external_incoming_sync_domain(s8 *sync_domain)
{
	u8 ip[DAVE_IP_V4_ADDR_LEN];
	u16 port;

	if((sync_domain != NULL) && (t_is_all_show_char((u8 *)sync_domain, dave_strlen(sync_domain)) == dave_true))
	{
		if(domainip(ip, &port, sync_domain) == dave_false)
		{
			SYNCLOG("invalid sync_domain:%s", sync_domain);
		}
		else
		{
			cfg_set(CFG_SYNC_SERVER_DOMAIN, sync_domain, dave_strlen(sync_domain));
		}
	}
}

dave_bool
sync_cfg_get_syncs_ip_and_port(u8 ip[DAVE_IP_V4_ADDR_LEN], u16 *port)
{
	s8 domain[1024];

	if(cfg_get(CFG_SYNC_SERVER_DOMAIN, domain, sizeof(domain)) == dave_false)
	{
		_sync_cfg_get_syncs_ip(ip);
		*port = _sync_cfg_get_syncs_port();

		dave_snprintf(domain, sizeof(domain),
			"%d.%d.%d.%d:%d",
			ip[0], ip[1], ip[2], ip[3], *port);
		cfg_set(CFG_SYNC_SERVER_DOMAIN, domain, dave_strlen(domain));
	}
	else
	{
		if(domainip(ip, port, domain) == dave_false)
		{
			domainip(ip, port, SYNC_SERVER_DOMAIN);
		}
	}

	return dave_true;
}

u16
sync_cfg_get_syncs_port(void)
{
	s8 port_str[64];
	u16 port;

	if(cfg_get(CFG_SYNC_SERVER_PORT, port_str, sizeof(port_str)) == dave_false)
	{
		port = _sync_cfg_get_syncs_port();
		dave_snprintf(port_str, sizeof(port_str), "%d", port);
		cfg_set(CFG_SYNC_SERVER_PORT, port_str, dave_strlen(port_str));
	}
	else
	{
		port = stringdigital(port_str);
	}

	if(port == 0)
	{
		port = SYNC_SERVICE_PORT;
	}

	return port;
}

dave_bool
sync_cfg_get_local_ip(u8 ip[DAVE_IP_V4_ADDR_LEN])
{
	s8 domain[1024];
	u16 port;
	dave_bool real_cfg = dave_false;

	cfg_get_by_default(CFG_SYNC_CLIENT_ADDRESS, domain, sizeof(domain), "0.0.0.0");

	real_cfg = domainip(ip, &port, domain);
	if((real_cfg == dave_true)
		&& (ip[0] == 0)
		&& (ip[1] == 0)
		&& (ip[2] == 0)
		&& (ip[3] == 0))
	{
		real_cfg = dave_false;
	}

	if(real_cfg == dave_false)
	{
		dave_memset(ip, 0x00, DAVE_IP_V4_ADDR_LEN);
	}

	return real_cfg;
}

#endif

