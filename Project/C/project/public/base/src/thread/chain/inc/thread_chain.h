/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_CHAIN_H__
#define __THREAD_CHAIN_H__
#include "base_define.h"
#include "thread_router.h"

#define DAVE_CHAIN_ID_LEN (56)
#define DAVE_CHAIN_THREAD_NAME_LEN (13)
#define DAVE_CHAIN_FUN_LEN (48)

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

	dave_bool request;
	ThreadId msg_src;
	ThreadId msg_dst;
	ub msg_id;

	s8 fun[DAVE_CHAIN_FUN_LEN];
	ub line;
} ThreadChain;

void thread_chain_init(void);

void thread_chain_exit(void);

void thread_chain_reload_cfg(CFGUpdate *pUpdate);

void thread_chain_reset(ThreadChain *pChain);

void thread_chain_free(ThreadChain *pChain);

void thread_chain_fill_msg(MSGBODY *msg, void *msg_chain);

ThreadChain * thread_chain_build_msg(
	void *msg_chain,
	ThreadId msg_src, ThreadId msg_dst, ub msg_id,
	s8 *fun, ub line);

void thread_chain_clean_msg(MSGBODY *msg);

ThreadChain * thread_chain_run_msg(MSGBODY *msg);

void thread_chain_run_clean(ThreadChain *pThreadChain, MSGBODY *msg);

void thread_chain_coroutine_msg(
	ThreadChain *pMsgChain, ThreadRouter *pMsgRouter,
	ThreadId msg_src, ThreadId msg_dst,
	ub msg_id, ub msg_len, u8 *msg_body);

void thread_chain_insert(
	ChainType type,
	ThreadChain *pChain, ThreadRouter *pRouter,
	s8 *src_gid, s8 *dst_gid,
	ThreadId msg_src, ThreadId msg_dst,
	ub msg_id, ub msg_len, void *msg_body);

MBUF * thread_chain_report(void);

void * thread_chain_to_bson(ThreadChain *pChain);

ThreadChain * thread_bson_to_chain(void *pBson);

#endif

