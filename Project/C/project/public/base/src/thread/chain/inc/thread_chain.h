/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_CHAIN_H__
#define __THREAD_CHAIN_H__
#include "base_define.h"

#define DAVE_CHAIN_ID_LEN (56)
#define DAVE_CHAIN_THREAD_NAME_LEN (13)

typedef enum {
	ChainType_none = 0,
	ChainType_calling,
	ChainType_called,
	ChainType_execution,
	ChainType_coroutine,
	ChainType_max = 0xff
} ChainType;

typedef struct {
	ChainType type;

	s8 chain_id[DAVE_CHAIN_ID_LEN];
	ub call_id;
	ub chain_counter;
	ub generation;

	ub send_time;
	ub recv_time;

	s8 src_gid[DAVE_GLOBALLY_IDENTIFIER_LEN];
	s8 dst_gid[DAVE_GLOBALLY_IDENTIFIER_LEN];

	s8 src_thread[DAVE_CHAIN_THREAD_NAME_LEN];
	s8 dst_thread[DAVE_CHAIN_THREAD_NAME_LEN];

	ThreadId msg_src;
	ThreadId msg_dst;
	ub msg_id;
} ThreadChain;

void thread_chain_init(void);

void thread_chain_exit(void);

void thread_chain_reset(ThreadChain *pChain);

void thread_chain_fill_msg(MSGBODY *msg, void *msg_chain);

ThreadChain * thread_chain_build_msg(ThreadId msg_src, ThreadId msg_dst, ub msg_id);

ThreadChain * thread_chain_run_msg(MSGBODY *msg);

void thread_chain_run_clean(ThreadChain *pChain, MSGBODY *msg);

void thread_chain_coroutine_msg(
	ThreadChain *pMsgChain,
	ThreadId src_id, ThreadId dst_id,
	ub msg_id, ub msg_len, u8 *msg_body);

void thread_chain_clean_msg(MSGBODY *msg);

void thread_chain_insert(
	ChainType type,
	ThreadChain *pChain,
	s8 *src_gid, s8 *dst_gid,
	ThreadId msg_src, ThreadId msg_dst,
	ub msg_id, ub msg_len, void *msg_body);

MBUF * thread_chain_report(void);

void * thread_chain_to_bson(ThreadChain *pChain);

ThreadChain * thread_bson_to_chain(void *pBson);

#endif

