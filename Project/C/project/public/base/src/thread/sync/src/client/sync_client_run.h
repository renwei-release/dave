/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_CLIENT_RUN_H__
#define __SYNC_CLIENT_RUN_H__

void sync_client_run_init(void);

void sync_client_run_exit(void);

void sync_client_run_thread(
	SyncServer *pServer,
	ub frame_len, u8 *frame_ptr);

void sync_client_run_internal(
	SyncServer *pServer,
	s8 *src, s8 *dst,
	ub msg_id,
	ub msg_len, u8 *msg_body);

#endif

