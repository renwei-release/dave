/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_CLIENT_MSG_BUFFER_H__
#define __SYNC_CLIENT_MSG_BUFFER_H__

typedef dave_bool (* sync_client_run_thread_fun)(
	SyncServer *pServer,
	void *msg_chain, void *msg_router,
	s8 *src, ThreadId route_src, s8 *dst, ThreadId route_dst,
	ub msg_id, BaseMsgType msg_type, ub msg_len, void *msg_body,
	dave_bool buffer_pop);

void sync_client_msg_buffer_init(void);

void sync_client_msg_buffer_exit(void);

void sync_client_msg_buffer_push(
	SyncServer *pServer,
	void *pChainBson, void *pRouterBson,
	s8 *src, ThreadId route_src,
	s8 *dst, ThreadId route_dst,
	ub msg_id, BaseMsgType msg_type,
	ub msg_len, void *msg_body,
	sync_client_run_thread_fun fun);

ub sync_client_msg_buffer_info(s8 *info, ub info_len);

#endif

