/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_CHAIN_H__
#define __THREAD_CHAIN_H__
#include "base_define.h"

typedef struct {
	dave_bool valid;

	s8 chain_id[DAVE_CHAIN_ID_LEN];
	ub chain_counter;
	ub generation;

	ub send_time;
	ub recv_time;

	s8 src_gid[DAVE_GLOBALLY_IDENTIFIER_LEN];
	s8 dst_gid[DAVE_GLOBALLY_IDENTIFIER_LEN];

	s8 src_thread[DAVE_THREAD_NAME_LEN];
	s8 dst_thread[DAVE_THREAD_NAME_LEN];

	ub msg_id;

	dave_bool called;
	ub call_id;
} ThreadChain;

void thread_chain_init(void);

void thread_chain_exit(void);

ThreadChain * thread_chain_malloc(void);

void thread_chain_free(ThreadChain *pChain);

void thread_chain_reset(ThreadChain *pChain);

dave_bool thread_chain_enable(ThreadId msg_src, ThreadId msg_dst, ub msg_id);

void thread_chain_clean(ThreadChain *pChain);

void thread_chain_build_msg(ThreadChain *pMsgChain, ThreadId msg_src, ThreadId msg_dst, ub msg_id);

ThreadChain * thread_chain_run_msg(MSGBODY *msg);

void thread_chain_insert(
	dave_bool called,
	ThreadChain *pChain,
	s8 *src_gid, s8 *dst_gid,
	ThreadId msg_src, ThreadId msg_dst,
	ub msg_id, ub msg_len, void *msg_body);

MBUF * thread_chain_report(void);

void * thread_chain_to_bson(ThreadChain *pChain);

ThreadChain * thread_bson_to_chain(void *pBson);

#endif

