/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "log_stack.h"
#ifdef LOG_STACK_SERVER
#include <dlfcn.h>
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_3rdparty.h"
#include "thread_chain.h"
#include "log_tracing.h"
#include "log_save_json.h"
#include "log_save_dll.h"
#include "log_save_cfg.h"
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
_log_save_router_to_json(ThreadRouter *pRouter)
{
	ub router_index;
	void *pJson = dave_json_malloc();
	void *pArray = dave_json_array_malloc();

	dave_json_add_str(pJson, "uid", pRouter->uid);
	dave_json_add_ub(pJson, "router_number", pRouter->router_number);
	dave_json_add_ub(pJson, "current_router_index", pRouter->current_router_index);

	for(router_index=0; router_index<pRouter->router_number; router_index++)
	{
		void *pSubRouter = dave_json_malloc();

		dave_json_add_str(pSubRouter, "thread", pRouter->sub_router[router_index].thread);
		if(pRouter->sub_router[router_index].gid[0] != '\0')
		{
			dave_json_add_str(pSubRouter, "gid", pRouter->sub_router[router_index].gid);
		}

		dave_json_array_add_object(pArray, pSubRouter);
	}

	dave_json_add_array(pJson, "sub_router", pArray);

	return pJson;
}

static inline void *
_log_save_chain_to_json(s8 *device_info, s8 *service_verno, ThreadChain *pChain, ThreadRouter *pRouter, ub msg_id, ub msg_len, void *msg_body)
{
	void *pJson = dave_json_malloc();
	s8 time_str[128], ub_str[64];

	dave_json_add_str(pJson, "type", t_auto_ChainType_str(pChain->type));

	dave_json_add_str(pJson, "device_info", device_info);
	dave_json_add_str(pJson, "service_verno", service_verno);
	dave_json_add_str(pJson, JSON_LOG_chain_id, pChain->chain_id);
	dave_json_add_ub(pJson, JSON_LOG_call_id, pChain->call_id);
	dave_json_add_ub(pJson, JSON_LOG_generation, pChain->generation);
	dave_json_add_ub(pJson, "chain_counter", pChain->chain_counter);

	dave_json_add_str(pJson, "send_date", _log_save_chain_time_str(time_str, sizeof(time_str), pChain->send_time));
	dave_json_add_str(pJson, "recv_date", _log_save_chain_time_str(time_str, sizeof(time_str), pChain->recv_time));
	dave_json_add_ub(pJson, "send_time", pChain->send_time);
	dave_json_add_ub(pJson, "recv_time", pChain->recv_time);
	dave_json_add_ub(pJson, "execution_time-us", pChain->recv_time - pChain->send_time);

	dave_json_add_str(pJson, "src_gid", pChain->src_gid);
	dave_json_add_str(pJson, "dst_gid", pChain->dst_gid);

	dave_json_add_str(pJson, JSON_LOG_src_thread, pChain->src_thread);
	dave_json_add_str(pJson, JSON_LOG_dst_thread, pChain->dst_thread);

	dave_json_add_str(pJson, JSON_LOG_action, pChain->request==dave_true?JSON_LOG_action_request:JSON_LOG_action_answer);
	dave_snprintf(ub_str, sizeof(ub_str), "%lx", pChain->msg_src);
	dave_json_add_str(pJson, "msg_src", ub_str);
	dave_snprintf(ub_str, sizeof(ub_str), "%lx", pChain->msg_dst);
	dave_json_add_str(pJson, "msg_dst", ub_str);
	dave_json_add_str(pJson, "msg_id", log_save_RPCMSG_str(pChain->msg_id));
	if((pRouter != NULL) && (pRouter->uid[0] != '\0'))
	{
		dave_json_add_object(pJson, "router", _log_save_router_to_json(pRouter));
	}
	if(msg_len > 0)
	{
		dave_json_add_object(pJson, "msg_body", log_save_msg_to_json(msg_id, msg_len, msg_body));
	}
	dave_json_add_str(pJson, "fun", pChain->fun);
	dave_json_add_ub(pJson, "line", pChain->line);

	return pJson;
}

static inline void
_log_save_chain(sb file_id, s8 *device_info, s8 *service_verno, ThreadChain *pChain, ThreadRouter *pRouter, ub msg_id, ub msg_len, void *msg_body)
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

	pJson = _log_save_chain_to_json(device_info, service_verno, pChain, pRouter, msg_id, msg_len, msg_body);

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

static inline ub
_log_save_chain_load_chain(ThreadChain **ppChain, ub *chain_len, s8 *content_ptr, ub content_len)
{
	ub content_index = 0;

	dave_byte_8_32(*chain_len, content_ptr[content_index++], content_ptr[content_index++], content_ptr[content_index++], content_ptr[content_index++]);
	*ppChain = (ThreadChain *)(&content_ptr[content_index]);
	content_index += *chain_len;

	return content_index;
}

static inline ub
_log_save_chain_load_router(ThreadRouter *pRouter, s8 *content_ptr, ub content_len)
{
	ub content_index = 0;
	ub uid_len, thread_len, gid_len;
	ub router_index;

	dave_byte_16(uid_len, content_ptr[content_index++], content_ptr[content_index++]);
	if(uid_len == 0)
	{
		pRouter->uid[0] = '\0';
		return content_index;
	}

	content_index += dave_memcpy(pRouter->uid, &content_ptr[content_index], uid_len);

	dave_byte_8_32(pRouter->router_number, content_ptr[content_index++], content_ptr[content_index++], content_ptr[content_index++], content_ptr[content_index++]);
	dave_byte_8_32(pRouter->current_router_index, content_ptr[content_index++], content_ptr[content_index++], content_ptr[content_index++], content_ptr[content_index++]);

	for(router_index=0; router_index<DAVE_ROUTER_SUB_MAX; router_index++)
	{
		if(router_index >= pRouter->router_number)
			break;

		dave_byte_16(thread_len, content_ptr[content_index++], content_ptr[content_index++]);
		if(thread_len == 0)
			pRouter->sub_router[router_index].thread[0] = '\0';
		else
			content_index += dave_memcpy(pRouter->sub_router[router_index].thread, &content_ptr[content_index], thread_len);		
		dave_byte_16(gid_len, content_ptr[content_index++], content_ptr[content_index++]);
		if(gid_len == 0)
			pRouter->sub_router[router_index].gid[0] = '\0';
		else
			content_index += dave_memcpy(pRouter->sub_router[router_index].gid, &content_ptr[content_index], gid_len);
	}

	return content_index;
}

static inline ub
_log_save_chain_load_msg(ub *msg_id, ub *msg_len, void **msg_body, s8 *content_ptr, ub content_len)
{
	ub content_index = 0;

	dave_byte_8_32(*msg_id, content_ptr[content_index++], content_ptr[content_index++], content_ptr[content_index++], content_ptr[content_index++]);
	dave_byte_8_32(*msg_len, content_ptr[content_index++], content_ptr[content_index++], content_ptr[content_index++], content_ptr[content_index++]);
	if(*msg_len > 0)
	{
		*msg_body = (void *)(&content_ptr[content_index]);
		content_index += *msg_len;
	}
	else
	{
		*msg_body = NULL;
	}

	return content_index;
}

static inline void
_log_save_chain_load_version_1(sb file_id, s8 *device_info, s8 *service_verno, u16 chain_version, s8 *content_ptr, ub content_len)
{
	ub content_index = 0;
	ub chain_len;
	ThreadChain *pChain;
	ub msg_id, msg_len;
	void *msg_body;

	content_index += _log_save_chain_load_chain(&pChain, &chain_len, &content_ptr[content_index], content_len-content_index);
	if(log_save_type_enable(pChain->type) == dave_false)
	{
		return;
	}
	content_index += _log_save_chain_load_msg(&msg_id, &msg_len, &msg_body, &content_ptr[content_index], content_len-content_index);

	if(chain_len != sizeof(ThreadChain))
	{
		LOGLOG("chain_len mismatch:%d/%d chain_version:%d",
			chain_len, sizeof(ThreadChain), chain_version);
	}
	if(msg_len > (content_len - content_index))
	{
		LOGLOG("content_len mismatch:%d/%d/%d msg_id:%d",
			content_index, msg_len, content_len,
			msg_id);
	}
	if(pChain->msg_id != msg_id)
	{
		LOGLOG("msg_id mismatch:%d/%d", pChain->msg_id, msg_id);
	}

	_log_save_chain(file_id, device_info, service_verno, pChain, NULL, msg_id, msg_len, msg_body);
}

static inline void
_log_save_chain_load_version_2(sb file_id, s8 *device_info, s8 *service_verno, u16 chain_version, s8 *content_ptr, ub content_len)
{
	ub content_index = 0;
	ub chain_len;
	ThreadChain *pChain;
	ThreadRouter *pRouter;
	ub msg_id, msg_len;
	void *msg_body;

	content_index += _log_save_chain_load_chain(&pChain, &chain_len, &content_ptr[content_index], content_len-content_index);
	if(log_save_type_enable(pChain->type) == dave_false)
	{
		return;
	}
	pRouter = dave_malloc(sizeof(ThreadRouter));
	content_index += _log_save_chain_load_router(pRouter, &content_ptr[content_index], content_len-content_index);
	content_index += _log_save_chain_load_msg(&msg_id, &msg_len, &msg_body, &content_ptr[content_index], content_len-content_index);

	if(chain_len != sizeof(ThreadChain))
	{
		LOGLOG("chain_len mismatch:%d/%d chain_version:%d",
			chain_len, sizeof(ThreadChain), chain_version);
	}
	if(pChain->msg_id != msg_id)
	{
		LOGLOG("msg_id mismatch:%d/%d", pChain->msg_id, msg_id);
	}

	_log_save_chain(file_id, device_info, service_verno, pChain, pRouter, msg_id, msg_len, msg_body);

	dave_free(pRouter);
}

// =====================================================================

void
log_save_chain_init(void)
{
	log_tracing_init();

	log_save_dll_init();
}

void
log_save_chain_exit(void)
{
	log_save_dll_exit();

	log_tracing_exit();
}

void
log_save_chain(sb file_id, s8 *device_info, s8 *service_verno, s8 *content_ptr, ub content_len)
{
	u16 chain_version;

	dave_byte_16(chain_version, content_ptr[0], content_ptr[1]);

	if(chain_version == 1)
		_log_save_chain_load_version_1(file_id, device_info, service_verno, chain_version, &content_ptr[2], content_len-2);
	else
		_log_save_chain_load_version_2(file_id, device_info, service_verno, chain_version, &content_ptr[2], content_len-2);
}

#endif

