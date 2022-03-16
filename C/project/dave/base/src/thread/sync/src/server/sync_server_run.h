/*
 * ================================================================================
 * (c) Copyright 2020 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.11.11.
 * ================================================================================
 */

#ifndef __SYNC_SERVER_RUN_H__
#define __SYNC_SERVER_RUN_H__

void sync_server_run_init(void);

void sync_server_run_exit(void);

void sync_server_run_internal(
	SyncClient *pClient,
	s8 *src, s8 *dst,
	ub msg_id,
	ub msg_len, u8 *msg_body);

#endif

