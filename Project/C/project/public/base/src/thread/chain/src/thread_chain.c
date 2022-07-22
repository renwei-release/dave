/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "base_define.h"
#include "base_tools.h"
#include "thread_chain.h"
#include "thread_tools.h"
#include "thread_log.h"
#include "chain_id.h"

static inline void
_thread_chain_new_msg(
	ThreadChain *pMsgChain,
	ThreadId src_id, ThreadId dst_id,
	ub msg_id)
{
	chain_id(pMsgChain->chain_id, sizeof(pMsgChain->chain_id));
	pMsgChain->generation = 0;

	pMsgChain->send_time = dave_os_time_ns();
	pMsgChain->recv_time = dave_os_time_ns();

	dave_memset(pMsgChain->src_gid, 0x00, sizeof(pMsgChain->src_gid));
	dave_strcpy(pMsgChain->dst_gid, globally_identifier(), sizeof(pMsgChain->dst_gid));

	dave_strcpy(pMsgChain->src_thread, thread_id_to_name(src_id), sizeof(pMsgChain->src_thread));
	dave_strcpy(pMsgChain->dst_thread, thread_id_to_name(dst_id), sizeof(pMsgChain->dst_thread));

	pMsgChain->msg_id = msg_id;
}

static inline void
_thread_chain_copy_msg(
	ThreadChain *pMsgChain, ThreadChain *pThreadChain,
	ThreadId src_id, ThreadId dst_id,
	ub msg_id)
{
	dave_strcpy(pMsgChain->chain_id, pThreadChain->chain_id, sizeof(pMsgChain->chain_id));
	pMsgChain->generation = pThreadChain->generation;

	pMsgChain->send_time = dave_os_time_ns();
	pMsgChain->recv_time = dave_os_time_ns();

	dave_strcpy(pMsgChain->src_gid, globally_identifier(), sizeof(pMsgChain->src_gid));
	dave_memset(pMsgChain->dst_gid, 0x00, sizeof(pMsgChain->dst_gid));

	dave_strcpy(pMsgChain->src_thread, thread_id_to_name(src_id), sizeof(pMsgChain->src_thread));
	dave_strcpy(pMsgChain->dst_thread, thread_id_to_name(dst_id), sizeof(pMsgChain->dst_thread));

	pMsgChain->msg_id = msg_id;
}

// =====================================================================

void
thread_chain_reset(ThreadChain *pChain)
{
	dave_memset(pChain, 0x00, sizeof(ThreadChain));

	pChain->valid = dave_false;
}

void
thread_chain_clean(ThreadChain *pChain)
{
	pChain->valid = dave_false;
}

void
thread_chain_build_msg(
	ThreadChain *pMsgChain,
	ThreadId src_id, ThreadId dst_id,
	ub msg_id)
{
	ThreadChain *pThreadChain;

	if((thread_id_to_attrib(src_id) == LOCAL_TASK_ATTRIB)
		&& (thread_id_to_attrib(dst_id) == LOCAL_TASK_ATTRIB))
	{
		return;
	}

	pThreadChain = thread_current_chain();
	if(pThreadChain == NULL)
	{
		THREADABNOR("pThreadChain is NULL! %x/%s->%x/%s:%s",
			src_id, thread_id_to_name(src_id), dst_id, thread_id_to_name(dst_id),
			msgstr(msg_id));
		return;
	}

	if(pThreadChain->valid == dave_false)
		_thread_chain_new_msg(pMsgChain, src_id, dst_id, msg_id);
	else
		_thread_chain_copy_msg(pMsgChain, pThreadChain, src_id, dst_id, msg_id);
}

ThreadChain *
thread_chain_run_msg(MSGBODY *msg)
{
	return NULL;
}

#endif

