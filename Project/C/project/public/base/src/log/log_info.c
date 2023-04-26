/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_verno.h"
#include "dave_os.h"

static s8 _log_product_name[2048] = { '\0' };
static s8 _log_chain_name[128] = { '\0' };
static s8 _log_device_info[2048] = { '\0' };

static void
_log_info_get_device_info(s8 *info_ptr, ub info_len)
{
	s8 host_name[128];
	u8 mac[DAVE_MAC_ADDR_LEN];

	dave_os_load_host_name(host_name, sizeof(host_name));

	dave_os_load_mac(mac);

	dave_snprintf(info_ptr, info_len, "%s-%02X%02X%02X%02X%02X%02X",
		host_name,
		mac[0], mac[1], mac[2],
		mac[3], mac[4], mac[5]);
}

// =====================================================================

s8 *
log_info_product(void)
{
	if(_log_product_name[0] == '\0')
	{
		dave_product(dave_verno(), _log_product_name, sizeof(_log_product_name));
	}
	return _log_product_name;
}

s8 *
log_info_chain(void)
{
	if(_log_chain_name[0] == '\0')
	{
		dave_snprintf(_log_chain_name, sizeof(_log_chain_name), "CHAIN");
	}
	return _log_chain_name;
}

s8 *
log_info_device(void)
{
	if(_log_device_info[0] == '\0')
	{
		_log_info_get_device_info(_log_device_info, sizeof(_log_device_info));
	}
	return _log_device_info;
}

