/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_GO_H__
#define __THREAD_GO_H__
#include "base_macro.h"
#include "dave_base.h"
#include "thread_struct.h"

void * thread_go_id(
	ThreadStruct *pSrcThread,
	ThreadId dst_id,
	ub req_id, ub req_len, u8 *req_body,
	ub rsp_id,
	s8 *fun, ub line);

void * thread_go_name(
	ThreadStruct *pSrcThread,
	s8 *dst_thread,
	ub req_id, ub req_len, u8 *req_body,
	ub rsp_id,
	s8 *fun, ub line);

void * thread_go_gid(
	ThreadStruct *pSrcThread,
	s8 *gid, s8 *dst_thread,
	ub req_id, ub req_len, u8 *req_body,
	ub rsp_id,
	s8 *fun, ub line);

void * thread_go_uid(
	ThreadStruct *pSrcThread,
	s8 *uid,
	ub req_id, ub req_len, u8 *req_body,
	ub rsp_id,
	s8 *fun, ub line);

#endif

