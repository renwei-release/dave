/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_PARAM_H__
#define __SYNC_PARAM_H__
#include "dave_base.h"
#include "dave_tools.h"
#include "thread_parameter.h"
#include "thread_sync.h"

#define SYNC_THREAD_MAX (THREAD_MAX)					/* 这个参数决定了系统最大支持的线程名数量，最好与 THREAD_MAX 参数保持一致 */

#define SYNC_CLIENT_MAX (THREAD_CLIENT_MAX)				/* 对于某个服务，这个参数决定了系统最大支持的服务的数量 */

#define SYNC_SERSER_MAX (1)								/* 这个参数决定了每个服务能连接的SYNC服务的最大数量 */

#define LINK_CLIENT_MAX (SYNC_CLIENT_MAX)

#define SERVER_DATA_MAX (SYNC_CLIENT_MAX + SYNC_SERSER_MAX + LINK_CLIENT_MAX)

#define SYNC_THREAD_NAME_LEN (64)						/* THREAD_NAME_MAX */

#define SYNC_LOCAL_ID_MAX (0xffff)						/* 参考 LOCAL_ID_MASK */

#define SYNC_THREAD_INDEX_MAX (0xffff)					/* 参考 THREAD_INDEX_MASK */

#define SYNC_NET_INDEX_MAX (0xffff)						/* 参考 NET_INDEX_MASK */

#define SYNC_STACK_HEAD_MAX_LEN (2048)

#define SYNC_RPC_EFFICIENCY_WARNING (8 * 1024)

#define SYNC_RPC_BIG_PACKAGE (4 * 1024 * 1024)

typedef struct {
	dave_bool support_queue_server;
} ServiceStatement;

typedef enum {
	SyncServerType_sync_client,
	SyncServerType_client,
	SyncServerType_child,
	SyncServerType_max
} SyncServerType;

typedef struct {
	s32 client_socket;

	TLock opt_pv;

	sb left_timer;
	sb sync_timer;

	ub recv_data_counter;
	ub send_data_counter;
	ub recv_msg_counter;
	ub send_msg_counter;

	ub sync_resynchronization_counter;
	ub sync_thread_index;

	s8 verno[DAVE_VERNO_STR_LEN];
	s8 globally_identifier[DAVE_GLOBALLY_IDENTIFIER_LEN];
	s8 host_name[DAVE_NORMAL_NAME_LEN];
	sb rpc_version;
	ub work_start_second;
	SocNetInfo NetInfo;
	dave_bool link_up_flag;
	u8 link_ip[16];
	u16 link_port;

	dave_bool receive_thread_done;
	dave_bool sync_thread_flag;
	dave_bool send_down_and_up_flag[SYNC_CLIENT_MAX];

	dave_bool ready_flag;	// 每个客户端上的线程名都同步到服务端后，标记置为True。
	dave_bool blocks_flag;	// 标记客户端是被挂载还是脱钩状态，如果被挂载才能对外提供服务，但不管挂载还是脱钩，都可请求外界服务。
	dave_bool client_flag;	// 由客户端提供，其是否能对外提供服务的标记。

	ub notify_blocks_flag;
	ub release_quantity;

	ub client_index;
} SyncClient;

typedef struct {
	SyncServerType server_type;
	s32 server_socket;

	TLock rxtx_pv;

	dave_bool server_connecting;
	dave_bool server_cnt;
	dave_bool server_booting;
	dave_bool server_ready;
	dave_bool server_busy;

	sb left_timer;
	sb reconnect_times;
	sb booting_reciprocal;

	ub recv_data_counter;
	ub send_data_counter;

	ub sync_thread_name_number;
	ub sync_thread_name_index;

	ub server_send_message_counter;
	ub server_recv_message_counter;

	s8 verno[DAVE_VERNO_STR_LEN];
	s8 globally_identifier[DAVE_GLOBALLY_IDENTIFIER_LEN];
	s8 host_name[DAVE_NORMAL_NAME_LEN];
	sb rpc_version;
	ub work_start_second;

	u8 cfg_server_ip[16];
	u16 cfg_server_port;
	u8 child_ip[16];
	u16 child_port;

	ub server_index;
	/*
	 * 考虑到目前sync client之间会建立两条链接，
	 * 通过globally_identifier来识别是否来自同一client端，
	 * 两个链路会有两个thread_id路由编号，
	 * 为方便应用层，这里通过shadow_index的方式
	 * 把两条链路中的某一条映射到另外一条上，
	 * 至于具体那条映射那条，要看sync_client_globally_identifier_add
	 * 处理的时间顺序。
	 */
	ub shadow_index;

	ServiceStatement service_statement;
} SyncServer;

typedef struct {
	s8 thread_name[SYNC_THREAD_NAME_LEN];

	SyncClient *pClient[SYNC_CLIENT_MAX];

	TLock chose_client_pv;
	ub chose_client_index;

	ub thread_send_message_counter;
	ub thread_recv_message_counter;

	ub thread_index;
} SyncThread;

typedef struct {
	s8 thread_name[SYNC_THREAD_NAME_LEN];
	ThreadId thread_id;

	TLock chose_server_pv;
	ub chose_server_index;

	SyncServer *pServer[SERVER_DATA_MAX];
	dave_bool shadow_index_ready_remove_flag[SERVER_DATA_MAX];
	sb shadow_index_ready_remove_counter[SERVER_DATA_MAX];

	ub thread_send_message_counter;
	ub thread_recv_message_counter;

	ub thread_index;
} LinkThread;

#endif

