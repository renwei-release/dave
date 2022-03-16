/*
 * ================================================================================
 * (c) Copyright 2020 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.09.04.
 * ================================================================================
 */

#ifndef __SYNC_CLIENT_DATA_H__
#define __SYNC_CLIENT_DATA_H__
#include "dave_base.h"

void sync_client_data_init(void);

void sync_client_data_exit(void);

void sync_client_data_reset_sync_server(void);

SyncServer * sync_client_data_sync_server(void);

SyncServer * sync_client_data_head_server(void);

LinkThread * sync_client_data_head_thread(void);

dave_bool sync_client_data_all_server_is_disconnect(void);

SyncServer * sync_client_data_server_inq_on_socket(s32 socket);

SyncServer * sync_client_data_server_inq_on_net(u8 *ip, u16 port);

SyncServer * sync_client_data_server_inq_on_index(ub server_index);

LinkThread * sync_client_data_thread_on_name(s8 *thread_name, ub thread_index);

ub sync_client_data_thread_index_on_name(s8 *thread_name);

ub sync_client_data_thread_local_reset(void);

s8 * sync_client_data_thread_local_name(ub thread_index);

ub sync_client_data_thread_local_index(s8 *thread_name);

void sync_client_data_del_server_on_all_thread(SyncServer *pServer);

void sync_client_data_reset_server(SyncServer *pServer, dave_bool clean_flag);

SyncServer * sync_client_data_server_add_client(s8 *verno, s8 *globally_identifier, u8 *ip, u16 port);

SyncServer * sync_client_data_server_del_client(s8 *verno, s8 *globally_identifier, u8 *ip, u16 port);

SyncServer * sync_client_data_server_add_child(s32 socket, u8 *ip, u16 port);

SyncServer * sync_client_data_server_del_child(s32 socket);

LinkThread * sync_client_data_thread_add(SyncServer *pServer, s8 *thread_name);

LinkThread * sync_client_data_thread_del(SyncServer *pServer, s8 *thread_name);

ub sync_client_data_info(s8 *info, ub info_len);

SyncServer * sync_client_server(ub server_index);

LinkThread * sync_client_thread(ub thread_index);

#endif

