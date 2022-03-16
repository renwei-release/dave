/*
 * ================================================================================
 * (c) Copyright 2020 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.09.03.
 * ================================================================================
 */

#ifndef __SYNC_CLIENT_TOOLS_H__
#define __SYNC_CLIENT_TOOLS_H__

typedef struct {
	u8 u8_test_data;
	u16 u16_test_data;
	u32 u32_test_data;
	ub ub_test_data;
	double double_test_data;
	s8 str_test_data[64];
	void *ptr;
} SyncTestMsg;

void sync_client_test_data(SyncTestMsg *pTestMsg);

dave_bool sync_client_test_data_valid(SyncTestMsg *pTestMsg);

s8 * sync_client_type_to_str(SyncServerType type);

ub sync_client_data_thread_name_to_index(s8 *thread_name);

void __sync_client_detected_rpc_efficiency__(ub msg_len, ub package_len, ub msg_id, s8 *fun, ub line);
#define sync_client_detected_rpc_efficiency(msg_len, package_len, msg_id) __sync_client_detected_rpc_efficiency__(msg_len, package_len, msg_id, (s8 *)__func__, (ub)__LINE__)

void sync_client_send_statistics(SyncServer *pServer, s8 *thread);

void sync_client_recv_statistics(SyncServer *pServer, s8 *thread);

#endif

