/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_SERVER_TX_H__
#define __SYNC_SERVER_TX_H__

void sync_server_tx_init(void);

void sync_server_tx_exit(void);

RetCode sync_server_tx_run_thread_msg_req(
	SyncThread *pSrcThread, SyncThread *pDstThread,
	SyncClient *pSrcClient, SyncClient *pDstClient,
	ThreadId route_src, ThreadId route_dst,
	s8 *src, s8 *dst,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	ub msg_id,
	BaseMsgType msg_type,
	u8 *msg_body, ub msg_len);

dave_bool sync_server_tx_run_internal_msg_v2_req(
	SyncClient *pClient,
	ub msg_id, ub msg_len, void *msg_body);

dave_bool sync_server_tx_test_run_thread_msg_req(
	SyncClient *pClient,
	ThreadId route_src, ThreadId route_dst,
	s8 *my_name, s8 *other_name,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	ub msg_id,
	BaseMsgType msg_type,
	u8 *msg_body, ub msg_len);

void sync_server_tx_my_verno(SyncClient *pClient);

void sync_server_tx_module_verno(SyncClient *pClient, s8 *verno);

void sync_server_tx_heartbeat(SyncClient *pClient, dave_bool req_flag);

void sync_server_tx_sync_thread_name_rsp(SyncClient *pClient, s8 *thread_name, sb thread_index);

dave_bool sync_server_tx_add_remote_thread_req(SyncClient *pClient, s8 *thread_name, sb thread_index);

dave_bool sync_server_tx_del_remote_thread_req(SyncClient *pClient, s8 *thread_name, sb thread_index);

dave_bool sync_server_tx_link_up_req(SyncClient *pClient, s8 *verno, s8 *globally_identifier, u8 *link_ip, u16 link_port);

dave_bool sync_server_tx_link_up_rsp(SyncClient *pClient);

dave_bool sync_server_tx_link_down_req(SyncClient *pClient, s8 *verno, u8 *link_ip, u16 link_port);

dave_bool sync_server_tx_link_down_rsp(SyncClient *pClient);

dave_bool sync_server_tx_rpcver_req(SyncClient *pClient);

dave_bool sync_server_tx_rpcver_rsp(SyncClient *pClient);

#endif

