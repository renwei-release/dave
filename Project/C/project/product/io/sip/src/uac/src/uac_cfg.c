/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "uac_log.h"


// =====================================================================

s8 *
uac_cfg_sbc_server(void)
{
    static s8 sbc_server[32] = { '\0' };

	if(sbc_server[0] == '\0')
	{
    	cfg_get_str("SBCServer", sbc_server, sizeof(sbc_server), "0.0.0.0");
	}

	if(dave_strcmp(sbc_server, "0.0.0.0") == dave_true)
	{
		UACLOG("sbc_server:%s is empty!", sbc_server);
		return NULL;
	}

    return sbc_server;
}

s8 *
uac_cfg_sbc_port(void)
{
	static s8 sbc_port[16] = { '\0' };

	if(sbc_port[0] == '\0')
	{
    	cfg_get_str("SBCPort", sbc_port, sizeof(sbc_port), "5080");
	}

	return sbc_port;
}

s8 *
uac_cfg_username(void)
{
	static s8 username[64] = { '\0' };

	if(username[0] == '\0')
	{
		cfg_get_str("SIPUserName", username, sizeof(username), "+85200000000");
	}

	return username;
}

s8 *
uac_cfg_password(void)
{
	static s8 password[64] = { '\0' };

	if(password[0] == '\0')
	{
		cfg_get_str("SIPPassword", password, sizeof(password), "******");
	}

	return password;
}

s8 *
uac_cfg_local_ip(void)
{
	static s8 local_ip[64] = { '\0' };
	u8 ip_v4[DAVE_IP_V4_ADDR_LEN];

	if(local_ip[0] == '\0')
	{
		cfg_get_str("SIPLocalIP", local_ip, sizeof(local_ip), "0");

		if(local_ip[0] == '0')
		{
			dave_os_load_ip(ip_v4, NULL);
			ipstr(ip_v4, sizeof(ip_v4), local_ip, sizeof(local_ip));

			cfg_set_str("SIPLocalIP", local_ip);
		}
	}

	return local_ip;
}

s8 *
uac_cfg_local_port(void)
{
	static s8 local_port[64] = { '\0' };

	if(local_port[0] == '\0')
	{
		cfg_get_str("SIPLocalPort", local_port, sizeof(local_port), "5080");
	}

	return local_port;
}

s8 *
uac_cfg_rtp_ip(void)
{
	static s8 rtp_ip[64] = { '\0' };
	u8 ip_v4[DAVE_IP_V4_ADDR_LEN];

	if(rtp_ip[0] == '\0')
	{
		cfg_get_str("RTPLocalIP", rtp_ip, sizeof(rtp_ip), "0");

		if(rtp_ip[0] == '0')
		{
			dave_os_load_ip(ip_v4, NULL);
			ipstr(ip_v4, sizeof(ip_v4), rtp_ip, sizeof(rtp_ip));

			cfg_set_str("RTPLocalIP", rtp_ip);
		}
	}

	return rtp_ip;
}

s8 *
uac_cfg_rtp_port(void)
{
	static s8 rtp_port[64] = { '\0' };

	if(rtp_port[0] == '\0')
	{
		cfg_get_str("RTPLocalPort", rtp_port, sizeof(rtp_port), "30000");
	}

	return rtp_port;
}

u8
uac_cfg_media_format(void)
{
	return (u8)cfg_get_ub("RTPMediaFormat", 8);
}

