/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_CLIENT) || defined(SYNC_STACK_SERVER)
#include "dave_os.h"
#include "dave_tools.h"
#include "dave_base.h"
#include "sync_lock.h"
#include "sync_log.h"

#define GLOBALLY_IDENTIFIER_CFG_NAME (s8 *)"GLOBALLYIDENTIFIER"

static s8 _sync_globally_identifier[DAVE_GLOBALLY_IDENTIFIER_LEN] = { '\0', '\0'};

static dave_bool
_sync_globally_identifier_check(s8 *globally_identifier_ptr, ub globally_identifier_len)
{
	u8 mac[DAVE_MAC_ADDR_LEN];
	s8 mac_str[DAVE_MAC_ADDR_LEN * 2 + 1];

	if(globally_identifier_ptr[0] == '\0')
	{
		return dave_false;
	}

	if((dave_strlen(globally_identifier_ptr) / 2) != DAVE_MD5_HASH_LEN)
	{
		return dave_false;
	}

	dave_os_load_mac(mac);
	dave_snprintf(mac_str, sizeof(mac_str),
		"%02X%02X%02X%02X%02X%02X",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	if(dave_memcmp(mac_str, globally_identifier_ptr, 12) == dave_false)
	{
		return dave_false;
	}

	return dave_true;
}

static void
_sync_globally_identifier_build(s8 *globally_identifier_ptr, ub globally_identifier_len)
{
	u8 mac[DAVE_MAC_ADDR_LEN];
	s8 hostname[256];
	u8 ip_v4[DAVE_IP_V4_ADDR_LEN];
	u8 ip_v6[DAVE_IP_V6_ADDR_LEN];
	s8 md5_str[DAVE_GLOBALLY_IDENTIFIER_LEN];
	s8 *encode_ptr;
	sb encode_len = sizeof(mac) + sizeof(hostname) + sizeof(ip_v4) + sizeof(ip_v6) + 256;
	sb encode_index;

	dave_os_load_mac(mac);
	dave_os_load_host_name(hostname, sizeof(hostname));
	dave_os_load_ip(ip_v4, ip_v6);

	encode_ptr = dave_malloc(encode_len);
	encode_index = 0;
	encode_index += dave_memcpy(&encode_ptr[encode_index], mac, DAVE_MAC_ADDR_LEN);
	encode_index += dave_strcpy(&encode_ptr[encode_index], hostname, encode_len-encode_index);
	encode_index += dave_memcpy(&encode_ptr[encode_index], ip_v4, DAVE_IP_V4_ADDR_LEN);
	encode_index += dave_memcpy(&encode_ptr[encode_index], ip_v6, DAVE_IP_V6_ADDR_LEN);
	encode_index += dave_snprintf(
		&encode_ptr[encode_index], encode_len-encode_index,
		"%lx%lx%lx%lx%lx%lx",
		dave_rand(), dave_rand(), dave_rand(),
		dave_rand(), dave_rand(), dave_rand());

	if(t_crypto_md5_str(md5_str, (u8 *)encode_ptr, encode_index) == dave_false)
	{
		base_restart("build globally identifier failed!");
	}

	dave_snprintf(globally_identifier_ptr, globally_identifier_len,
		"%02X%02X%02X%02X%02X%02X%s",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
		md5_str);

	dave_free(encode_ptr);
}

static void
_sync_globally_identifier_save(s8 *globally_identifier_ptr, ub globally_identifier_len)
{
	cfg_set(GLOBALLY_IDENTIFIER_CFG_NAME, (u8 *)globally_identifier_ptr, globally_identifier_len);
}

static dave_bool
_sync_globally_identifier_load(s8 *globally_identifier_ptr, ub globally_identifier_len)
{
	return cfg_get(GLOBALLY_IDENTIFIER_CFG_NAME, (u8 *)globally_identifier_ptr, globally_identifier_len);
}

// =====================================================================

s8 *
sync_globally_identifier(void)
{
	dave_bool build_new = dave_false;
	s8 *id_ptr = _sync_globally_identifier;
	ub id_len = sizeof(_sync_globally_identifier);

	sync_lock();
	if(_sync_globally_identifier_check(id_ptr, id_len) == dave_false)
	{
		if((_sync_globally_identifier_load(id_ptr, id_len) == dave_false)
			|| (_sync_globally_identifier_check(id_ptr, id_len) == dave_false))
		{
			build_new = dave_true;

			_sync_globally_identifier_build(id_ptr, id_len);

			_sync_globally_identifier_save(id_ptr, id_len);
		}

		SYNCTRACE("%s my globally identifier:%s", build_new==dave_true?"Build":"Load", id_ptr);
	}
	sync_unlock();

	return id_ptr;
}

#endif

