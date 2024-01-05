/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_SERVER_DATA_H__
#define __SYNC_SERVER_DATA_H__

void sync_server_data_init(void);

void sync_server_data_exit(void);

SyncClient * sync_server_add_client(s32 socket, SocNetInfo *pNetInfo);

void sync_server_del_client(SyncClient *pClient);

SyncClient * sync_server_find_client(s32 socket);

SyncThread * sync_server_add_thread(SyncClient *pClient, s8 *thread_name);

ub sync_server_del_thread(SyncThread *pThread, SyncClient *pClient);

SyncThread * sync_server_find_thread(s8 *thread_name);

dave_bool sync_server_client_on_the_thread(SyncThread *pThread, SyncClient *pClient);

SyncClient * sync_server_find_effective_client(s8 *thread_name);

SyncClient * sync_server_check_globally_identifier_conflict(SyncClient *pClient);

void sync_server_my_verno_to_all_client(SyncClient *pMyClient);

void sync_server_run_test(SyncClient *pClient,
	ThreadId route_src, ThreadId route_dst,
	s8 *my_name, s8 *other_name,
	ub msg_id,
	BaseMsgType msg_type,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	ub msg_len, u8 *msg_body);

void sync_server_client_state(SyncClient *pClient, dave_bool busy);

SyncClient * sync_server_client(ub client_index);

SyncThread * sync_server_thread(ub thread_index);

ub sync_server_data_info(s8 *info, ub info_len);

#endif

