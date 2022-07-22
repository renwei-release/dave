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
	ub generation;

	ub send_time;
	ub recv_time;

	s8 src_gid[DAVE_GLOBALLY_IDENTIFIER_LEN];
	s8 dst_gid[DAVE_GLOBALLY_IDENTIFIER_LEN];

	s8 src_thread[DAVE_THREAD_NAME_LEN];
	s8 dst_thread[DAVE_THREAD_NAME_LEN];

	ub msg_id;
} ThreadChain;

void thread_chain_reset(ThreadChain *pChain);

void thread_chain_clean(ThreadChain *pChain);

void thread_chain_build_msg(ThreadChain *pMsgChain, ThreadId src_id, ThreadId dst_id, ub msg_id);

ThreadChain * thread_chain_run_msg(MSGBODY *msg);

#endif

