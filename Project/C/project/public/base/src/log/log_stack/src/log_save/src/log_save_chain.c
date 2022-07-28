/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "log_stack.h"
#ifdef LOG_STACK_SERVER
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_3rdparty.h"
#include "thread_chain.h"
#include "log_tracing.h"
#include "log_lock.h"
#include "log_log.h"

static inline s8 *
_log_save_chain_time_str(s8 *time_ptr, ub time_len, ub microseconds)
{
	DateStruct date = t_time_second_struct(microseconds/1000000);

	dave_snprintf(time_ptr, time_len, "%04d-%02d-%02d %02d:%02d:%02d",
		date.year, date.month, date.day,
		date.hour, date.minute, date.second);

	return time_ptr;
}

static inline void *
_log_save_chain_to_json(s8 *device_info, ThreadChain *pChain, ub msg_id, ub msg_len, void *msg_body)
{
	void *pJson = dave_json_malloc();
	s8 time_str[128], id_str[64];

	dave_json_add_str(pJson, "type", t_auto_ChainType_str(pChain->type));

	dave_json_add_str(pJson, "service_info", device_info);
	dave_json_add_str(pJson, "chain_id", pChain->chain_id);
	dave_json_add_ub(pJson, "call_id", pChain->call_id);
	dave_json_add_ub(pJson, "chain_counter", pChain->chain_counter);
	dave_json_add_ub(pJson, "generation", pChain->generation);

	dave_json_add_str(pJson, "send_date", _log_save_chain_time_str(time_str, sizeof(time_str), pChain->send_time));
	dave_json_add_str(pJson, "recv_date", _log_save_chain_time_str(time_str, sizeof(time_str), pChain->recv_time));
	dave_json_add_ub(pJson, "send_time", pChain->send_time);
	dave_json_add_ub(pJson, "recv_time", pChain->recv_time);
	dave_json_add_ub(pJson, "execution_time-us", pChain->recv_time - pChain->send_time);

	dave_json_add_str(pJson, "src_gid", pChain->src_gid);
	dave_json_add_str(pJson, "dst_gid", pChain->dst_gid);

	dave_json_add_str(pJson, "src_thread", pChain->src_thread);
	dave_json_add_str(pJson, "dst_thread", pChain->dst_thread);

	dave_snprintf(id_str, sizeof(id_str), "%lx", pChain->msg_src);
	dave_json_add_str(pJson, "msg_src", id_str);
	dave_snprintf(id_str, sizeof(id_str), "%lx", pChain->msg_dst);
	dave_json_add_str(pJson, "msg_dst", id_str);
	dave_json_add_str(pJson, "msg_id", msgstr(pChain->msg_id));
	if(msg_len > 0)
	{
		dave_json_add_object(pJson, "msg_body", t_rpc_rebuild_to_json(msg_id, msg_len, msg_body));
	}

	return pJson;
}

static inline void
_log_save_chain(sb file_id, s8 *device_info, ThreadChain *pChain, ub msg_id, ub msg_len, void *msg_body)
{
	void *pJson;
	s8 *json_str;
	ub json_len, file_len;

	LOGDEBUG("type:%s chain:%s generation:%d %lx->%lx %s->%s %s->%s->%s msg_len:%d",
		t_auto_ChainType_str(pChain->type),
		pChain->chain_id, pChain->generation,
		pChain->send_time, pChain->recv_time,
		pChain->src_gid, pChain->dst_gid,
		pChain->src_thread, pChain->dst_thread, msgstr(pChain->msg_id),
		msg_len);

	pJson = _log_save_chain_to_json(device_info, pChain, msg_id, msg_len, msg_body);

	json_str = dave_json_to_string(pJson, &json_len);

	file_len = dave_os_file_len(NULL, file_id);
	if(file_len < 0)
	{
		file_len = 0;
	}
	file_len += dave_os_file_save(file_id, (ub)file_len, json_len, (u8 *)json_str);
	dave_os_file_save(file_id, (ub)file_len, 1, (u8 *)"\n");

	if(log_tracing(pChain, pJson) == dave_false)
	{
		dave_json_free(pJson);
	}
}

// =====================================================================

void
log_save_chain_init(void)
{
	log_tracing_init();
}

void
log_save_chain_exit(void)
{
	log_tracing_exit();
}

void
log_save_chain(sb file_id, s8 *device_info, s8 *content_ptr, ub content_len)
{
	ub content_index = 0;
	u16 chain_version;
	ub chain_len;
	ThreadChain *pChain;
	ub msg_id, msg_len;
	void *msg_body;

	dave_byte_16(chain_version, content_ptr[content_index++], content_ptr[content_index++]);
	dave_byte_8_32(chain_len, content_ptr[content_index++], content_ptr[content_index++], content_ptr[content_index++], content_ptr[content_index++]);
	pChain = (ThreadChain *)(&content_ptr[content_index]);
	content_index += chain_len;

	dave_byte_8_32(msg_id, content_ptr[content_index++], content_ptr[content_index++], content_ptr[content_index++], content_ptr[content_index++]);
	dave_byte_8_32(msg_len, content_ptr[content_index++], content_ptr[content_index++], content_ptr[content_index++], content_ptr[content_index++]);
	if(msg_len > 0)
		msg_body = (void *)(&content_ptr[content_index]);
	else
		msg_body = NULL;

	if(chain_len != sizeof(ThreadChain))
	{
		LOGLOG("chain_len mismatch:%d/%d chain_version:%d",
			chain_len, sizeof(ThreadChain), chain_version);
		return;
	}

	if(msg_len > (content_len - content_index))
	{
		LOGLOG("content_len mismatch:%d/%d/%d msg_id:%d",
			content_index, msg_len, content_len,
			msg_id);
		return;
	}

	if(pChain->msg_id != msg_id)
	{
		LOGLOG("msg_id mismatch:%d/%d", pChain->msg_id, msg_id);
		return;
	}

	_log_save_chain(file_id, device_info, pChain, msg_id, msg_len, msg_body);
}

#endif

