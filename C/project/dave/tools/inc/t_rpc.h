/*
 * ================================================================================
 * (c) Copyright 2022 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2022.03.13.
 *
 * ############################# IMPORTANT INFORMATION ############################
 * The code of this file is automatically generated by tools(Tools/rpc),
 * please do not modify it manually!
 * ############################# IMPORTANT INFORMATION ############################
 * ================================================================================
 */

#ifndef __T_RPC_H__
#define __T_RPC_H__

typedef enum {
	MSGID_RESERVED = 0x0000000000000000,

	SOCKET_BIND_REQ = 1000,
	SOCKET_BIND_RSP = 1001,
	SOCKET_CONNECT_REQ = 1002,
	SOCKET_CONNECT_RSP = 1003,
	SOCKET_DISCONNECT_REQ = 1004,
	SOCKET_DISCONNECT_RSP = 1005,
	SOCKET_PLUGIN = 1006,
	SOCKET_PLUGOUT = 1007,
	SOCKET_READ = 1008,
	SOCKET_WRITE = 1009,
	SOCKET_NOTIFY = 1010,
	SOCKET_RAW_EVENT = 1011,
	MSGID_TEST = 1,
	MSGID_TIMER = 2,
	MSGID_WAKEUP = 3,
	MSGID_RUN_FUNCTION = 4,
	MSGID_DEBUG_REQ = 5,
	MSGID_DEBUG_RSP = 6,
	MSGID_RESTART_REQ = 7,
	MSGID_RESTART_RSP = 8,
	MSGID_POWER_OFF = 9,
	MSGID_REMOTE_THREAD_READY = 10,
	MSGID_REMOTE_THREAD_REMOVE = 11,
	MSGID_TRACE_SWITCH = 12,
	MSGID_REMOTE_MSG_TIMER_OUT = 13,
	MSGID_TEMPORARILY_DEFINE_MESSAGE = 14,
	MSGID_SYSTEM_MOUNT = 15,
	MSGID_SYSTEM_DECOUPLING = 16,
	MSGID_MEMORY_WARNING = 17,
	MSGID_ECHO = 18,
	MSGID_INTERNAL_EVENTS = 19,
	MSGID_THREAD_BUSY = 20,
	MSGID_THREAD_IDLE = 21,
	MSGID_CLIENT_BUSY = 22,
	MSGID_CLIENT_IDLE = 23,
	MSGID_REMOTE_THREAD_ID_READY = 24,
	MSGID_REMOTE_THREAD_ID_REMOVE = 25,
	MSGID_LOCAL_THREAD_READY = 26,
	MSGID_LOCAL_THREAD_REMOVE = 27,
	MSGID_RPC_DEBUG_MSG = 28,
	MSGID_CFG_UPDATE = 29,
	MSGID_BLOCKS_REQ = 30,
	MSGID_BLOCKS_RSP = 31,
	MSGID_OS_NOTIFY = 32,

	MSGID_INVALID = 0xffffffffffffffff
} RPCMSG;

MBUF * t_rpc_zip(sb ver, ub msg_id, void *msg_body, ub msg_len);
dave_bool t_rpc_unzip(void **msg_body, ub *msg_len, ub msg_id, s8 *packet_ptr, ub packet_len);

#endif
