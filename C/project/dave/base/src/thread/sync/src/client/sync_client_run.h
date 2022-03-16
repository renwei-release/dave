/*
 * ================================================================================
 * (c) Copyright 2020 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.09.03.
 * ================================================================================
 */

#ifndef __SYNC_CLIENT_RUN_H__
#define __SYNC_CLIENT_RUN_H__

void sync_client_run_init(void);

void sync_client_run_exit(void);

void sync_client_run_thread(SyncServer *pServer, ub frame_len, u8 *frame);

void sync_client_run_thread_events(void *ptr);

void sync_client_run_internal(
	s8 *src, s8 *dst,
	ub msg_id,
	ub msg_len, u8 *msg_body);

#endif

