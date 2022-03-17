/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_CLIENT_TX_H__
#define __SYNC_CLIENT_TX_H__

dave_bool sync_client_tx_run_internal_msg_req(ub msg_id, ub msg_len, void *msg_body);

void sync_client_tx_my_verno(SyncServer *pServer);

void sync_client_tx_heartbeat(SyncServer *pServer, dave_bool req_flag);

dave_bool sync_client_tx_sync_thread_name_req(SyncServer *pServer, s8 *thread_name, ub thread_index);

dave_bool sync_client_tx_sync_thread_name_rsp(SyncServer *pServer, s8 *thread_name, ub thread_index);

void sync_client_tx_run_thread_msg_rsp(SyncServer *pServer, s8 *src, s8 *dst, ub msg_id, ErrCode ret);

dave_bool sync_client_tx_run_thread_msg_req(
	SyncServer *pServer,
	ThreadId route_src, ThreadId route_dst,
	s8 *src, s8 *dst, ub msg_id,
	BaseMsgType msg_type, TaskAttribute src_attrib, TaskAttribute dst_attrib,
	ub msg_len, void *msg_body);

dave_bool sync_client_tx_test_run_thread_msg_req(SyncServer *pServer, s8 *my_name, s8 *other_name);

dave_bool sync_client_tx_add_remote_thread_rsp(SyncServer *pServer, s8 *thread_name, sb thread_index);

dave_bool sync_client_tx_del_remote_thread_rsp(SyncServer *pServer, s8 *thread_name, sb thread_index);

dave_bool sync_client_tx_link_up_req(SyncServer *pServer, u8 ip[DAVE_IP_V6_ADDR_LEN], u16 port);

dave_bool sync_client_tx_link_up_rsp(SyncServer *pServer, s8 *verno, u8 *link_ip, u16 link_port);

dave_bool sync_client_tx_link_down_req(SyncServer *pServer, u8 ip[DAVE_IP_V6_ADDR_LEN], u16 port);

dave_bool sync_client_tx_link_down_rsp(SyncServer *pServer, s8 *verno, u8 *link_ip, u16 link_port);

dave_bool sync_client_tx_rpcver_req(SyncServer *pServer);

dave_bool sync_client_tx_rpcver_rsp(SyncServer *pServer);

dave_bool sync_client_tx_system_state(dave_bool busy);

#endif

