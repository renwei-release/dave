/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_SERVER_BROADCADT_TX_H__
#define __SYNC_SERVER_BROADCADT_TX_H__

ErrCode sync_server_broadcadt_tx_the_msg_to_all_client(
	SyncClient *pSrcClient,
	ThreadId src_id, TaskAttribute src_attrib, s8 *src_name,
	ub msg_id,
	BaseMsgType msg_type,
	u8 *msg_body, ub msg_len);

void sync_server_broadcadt_tx_the_thread_all_client(
	SyncClient *pSrcClient,
	SyncThread *pSrcThread, SyncThread *pDstThread,
	ThreadId src_id, ThreadId dst_id,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	s8 *src_name, s8 *dst_name,
	ub msg_id,
	BaseMsgType msg_type,
	u8 *msg_body, ub msg_len);

void sync_server_broadcadt_tx_the_msg_to_client(
	SyncThread *pSrcThread, SyncThread *pDstThread,
	SyncClient *pSrcClient, SyncClient *pDstClient,
	ThreadId src_id, ThreadId dst_id,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	s8 *src_name, s8 *dst_name,
	ub msg_id,
	BaseMsgType msg_type,
	u8 *msg_body, ub msg_len);

#endif

