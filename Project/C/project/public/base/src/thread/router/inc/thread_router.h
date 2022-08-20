/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_ROUTER_H__
#define __THREAD_ROUTER_H__
#include "base_define.h"

#define DAVE_ROUTER_UID_LEN DAVE_UID_LEN
#define DAVE_ROUTER_SUB_MAX 32

typedef struct {
	s8 thread[DAVE_THREAD_NAME_LEN];
	s8 gid[DAVE_GLOBALLY_IDENTIFIER_LEN];
} ThreadSubRouter;

typedef struct {
	s8 uid[DAVE_ROUTER_UID_LEN];

	ub router_number;
	ub current_router_index;
	ThreadSubRouter sub_router[DAVE_ROUTER_SUB_MAX];

#ifdef LEVEL_PRODUCT_alpha
	s8 router_info[1024];
#endif
} ThreadRouter;

void thread_router_reset(ThreadRouter *pRouter);

void thread_router_free(ThreadRouter *pRouter);

ThreadRouter * thread_router_build_msg(void *msg_router, ub msg_id);

void thread_router_clean_msg(MSGBODY *msg);

ThreadId thread_router_build_router(ThreadRouter **ppRouter, s8 *uid);

ThreadId thread_router_pop_msg(ThreadRouter **ppRouter, s8 *uid);

ThreadRouter * thread_router_run_msg(MSGBODY *msg);

void thread_router_run_clean(ThreadRouter *pThreadRouter);

ThreadId thread_router_check_uid(s8 *uid);

void thread_router_next_route(ThreadRouter *pRouter);

void * thread_router_to_bson(ThreadRouter *pRouter);

ThreadRouter * thread_bson_to_router(void *pBson);

s8 * thread_router_info(s8 *msg, ThreadRouter *pRouter);

#endif

