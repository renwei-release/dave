/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "tools_log.h"

void * t_rpc_ver3_zip(ub msg_id, void *msg_body, ub msg_len);
dave_bool t_rpc_ver3_unzip(void **msg_body, ub *msg_len, ub msg_id, s8 *packet_ptr, ub packet_len);

static void *
_t_rpc_zip_ver1(ub msg_id, void *msg_body, ub msg_len)
{
	TOOLSLOG("unsupport ver1 msg_id:%s msg_len:%d", msgstr(msg_id), msg_len);
	return NULL;
}

static dave_bool
_t_rpc_unzip_ver1(void **msg_body, ub *msg_len, ub msg_id, s8 *packet_ptr, ub packet_len)
{
	TOOLSLOG("unsupport ver1 msg_id:%s packet_len:%d", msgstr(msg_id), packet_len);
	return dave_false;
}

static inline MBUF *
_t_rpc_zip_ver3(ub msg_id, void *msg_body, ub msg_len)
{
	void *pBson;
	MBUF *mbuf_data;

	pBson = t_rpc_ver3_zip(msg_id, msg_body, msg_len);
	if(pBson == NULL)
		return NULL;

	mbuf_data = t_bson_to_mbuf(pBson);

	t_bson_free_object(pBson);

	return mbuf_data;
}

static inline  dave_bool
_t_rpc_unzip_ver3(void **msg_body, ub *msg_len, ub msg_id, s8 *packet_ptr, ub packet_len)
{
	return t_rpc_ver3_unzip(msg_body, msg_len, msg_id, packet_ptr, packet_len);
}

// =====================================================================

MBUF *
t_rpc_zip(sb ver, ub msg_id, void *msg_body, ub msg_len)
{
	if(ver == 1)
		return _t_rpc_zip_ver1(msg_id, msg_body, msg_len);
	else
		return _t_rpc_zip_ver3(msg_id, msg_body, msg_len);
}

dave_bool
t_rpc_unzip(void **msg_body, ub *msg_len, ub msg_id, s8 *packet_ptr, ub packet_len)
{
	if(packet_ptr[0] == '{')
		return _t_rpc_unzip_ver1(msg_body, msg_len, msg_id, packet_ptr, packet_len);
	else
		return _t_rpc_unzip_ver3(msg_body, msg_len, msg_id, packet_ptr, packet_len);
}

