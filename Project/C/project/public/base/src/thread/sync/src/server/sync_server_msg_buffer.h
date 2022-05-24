/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_SERVER_MSG_BUFFER_H__
#define __SYNC_SERVER_MSG_BUFFER_H__

typedef RetCode (* sync_server_run_thread_fun)(SyncClient *pClient, ub frame_len, u8 *frame, dave_bool buffer_pop);

void sync_server_msg_buffer_init(void);

void sync_server_msg_buffer_exit(void);

void sync_server_msg_buffer_push(
	SyncClient *pClient,
	ub frame_len, u8 *frame,
	sync_server_run_thread_fun fun);

ub sync_server_msg_buffer_info(s8 *info, ub info_len);

#endif

