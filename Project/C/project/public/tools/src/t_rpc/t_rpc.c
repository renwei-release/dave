/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "tools_log.h"

void * t_rpc_ver3_zip(void *pChainBson, ub msg_id, void *msg_body, ub msg_len);
dave_bool t_rpc_ver3_unzip(void **ppChainBson, void **msg_body, ub *msg_len, ub msg_id, s8 *packet_ptr, ub packet_len);
void * t_rpc_ver3_ptr(ub msg_id, void *msg_body, void *new_ptr);
ub t_rpc_ver3_sizeof(ub msg_id);

static void *
_t_rpc_zip_ver1(ub msg_id, void *msg_body, ub msg_len)
{
	TOOLSLOG("unsupport ver1 msg_id:%d msg_len:%d", msg_id, msg_len);
	return NULL;
}

static dave_bool
_t_rpc_unzip_ver1(void **msg_body, ub *msg_len, ub msg_id, s8 *packet_ptr, ub packet_len)
{
	TOOLSLOG("unsupport ver1 msg_id:%d packet_len:%d", msg_id, packet_len);
	return dave_false;
}

static inline MBUF *
_t_rpc_zip_ver3(void *pChainBson, ub msg_id, void *msg_body, ub msg_len)
{
	void *pBson;
	MBUF *mbuf_data;

	pBson = t_rpc_ver3_zip(pChainBson, msg_id, msg_body, msg_len);
	if(pBson == NULL)
		return NULL;

	mbuf_data = t_bson_to_mbuf(pBson);

	t_bson_free_object(pBson);

	return mbuf_data;
}

static inline  dave_bool
_t_rpc_unzip_ver3(void **ppChainBson, void **msg_body, ub *msg_len, ub msg_id, s8 *packet_ptr, ub packet_len)
{
	return t_rpc_ver3_unzip(ppChainBson, msg_body, msg_len, msg_id, packet_ptr, packet_len);
}

// =====================================================================

MBUF *
t_rpc_zip(sb ver, void *pChainBson, ub msg_id, void *msg_body, ub msg_len)
{
	if(ver == 1)
		return _t_rpc_zip_ver1(msg_id, msg_body, msg_len);
	else
		return _t_rpc_zip_ver3(pChainBson, msg_id, msg_body, msg_len);
}

dave_bool
t_rpc_unzip(void **ppChainBson, void **msg_body, ub *msg_len, ub msg_id, s8 *packet_ptr, ub packet_len)
{
	if(packet_ptr[0] == '{')
		return _t_rpc_unzip_ver1(msg_body, msg_len, msg_id, packet_ptr, packet_len);
	else
		return _t_rpc_unzip_ver3(ppChainBson, msg_body, msg_len, msg_id, packet_ptr, packet_len);
}

void *
t_rpc_ptr(ub msg_id, void *msg_body, void *new_ptr)
{
	return t_rpc_ver3_ptr(msg_id, msg_body, new_ptr);
}

void *
t_rpc_rebuild_to_json(ub msg_id, ub msg_len, void *msg_body)
{
	ub msg_real_len = t_rpc_ver3_sizeof(msg_id);
	void *msg_real_body = NULL;
	void *pBson, *pJson;

	if(msg_real_len == 0)
	{
		TOOLSLOG("wath is msg_id:%s msg_len:%d", msgstr(msg_id), msg_len);
		return NULL;
	}

	if(msg_real_len > msg_len)
	{
		msg_real_body = dave_malloc(msg_real_len);
		dave_memcpy(msg_real_body, msg_body, msg_len);
		dave_memset(&(((s8 *)msg_real_body)[msg_len]), 0x00, msg_real_len-msg_len);
		msg_body = msg_real_body;
		msg_len = msg_real_len;
	}

	pBson = t_rpc_ver3_zip(NULL, msg_id, msg_body, msg_len);

	if(msg_real_body != NULL)
	{
		dave_free(msg_real_body);
	}

	pJson = t_bson_to_json(pBson);

	t_bson_free_object(pBson);

	return pJson;
}

