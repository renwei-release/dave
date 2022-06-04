/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __BASE_MSG_H__
#define __BASE_MSG_H__

/* for MSGID_TEST message */
typedef struct {
	s8 test_msg[4096];
} TESTMSG;

/* for MSGID_TIMER message */
typedef struct {
	sb timer_id;
} TIMERMSG;

/* for MSGID_WAKEUP message */
typedef struct {
	void *null_msg;
	u32 some_msg;
} WAKEUPMSG;

/* for MSGID_RUN_FUNCTION message */
typedef struct {
	void *thread_fun;
	void *last_fun;
	void *param;
	ThreadId thread_dst;
	dave_bool initialization_flag;
} RUNFUNCTIONMSG;

/* for MSGID_DEBUG_REQ message */
typedef struct {
	s8 msg[4096];
	void *ptr;
} DebugReq;

/* for MSGID_DEBUG_RSP message */
typedef struct {
	s8 msg[1048576];
	void *ptr;
} DebugRsp;

/* for MSGID_RESTART_REQ message */
typedef struct {
	s8 reason[128];
	ub times;
} RESTARTREQMSG;

/* for MSGID_RESTART_RSP message */
typedef struct {
	dave_bool wait_flag;
} RESTARTRSPMSG;

/* for MSGID_POWER_OFF message */
typedef struct {
	s8 reason[128];
} POWEROFFMSG;

/* for MSGID_REMOTE_THREAD_READY message */
typedef struct {
	ThreadId remote_thread_id;
	s8 remote_thread_name[128];
} ThreadRemoteReadyMsg;

/* for MSGID_REMOTE_THREAD_REMOVE message */
typedef struct {
	ThreadId remote_thread_id;
	s8 remote_thread_name[128];
} ThreadRemoteRemoveMsg;

/* for MSGID_TRACE_SWITCH message */
typedef struct {
	ThreadId thread_id;
	dave_bool trace_on;
} TraceSwitchMsg;

/* for MSGID_PROCESS_MSG_TIMER_OUT message */
typedef struct {
	ub msg_id;
	ub msg_len;
	void *msg_body;
} ProcessMsgTimerOutMsg;

/* for MSGID_TEMPORARILY_DEFINE_MESSAGE message */
typedef struct {
	void *parameter;
} TemporarilyDefineMessageMsg;

/* for MSGID_SYSTEM_MOUNT message */
typedef struct {
	s32 socket;
	s8 verno[DAVE_VERNO_STR_LEN];
	SocNetInfo NetInfo;
} SystemMount;

/* for MSGID_SYSTEM_DECOUPLING message */
typedef struct {
	s32 socket;
	s8 verno[DAVE_VERNO_STR_LEN];
	SocNetInfo NetInfo;
} SystemDecoupling;

/* for MSGID_MEMORY_WARNING message */
typedef struct {
	ub used_percentage;
} MemoryWarning;

/* for MSGID_ECHO message */
typedef struct {
	ub echo_counter;
	ub echo_time;
	dave_bool echo_multiple;
	dave_bool concurrency_flag;
	s8 msg[256];
} MsgIdEcho;

/* for MSGID_INTERNAL_EVENTS message */
typedef struct {
	ub event_id;
	void *ptr;
} InternalEvents;

/* for MSGID_THREAD_BUSY message */
typedef struct {
	ThreadId thread_id;
	s8 thread_name[DAVE_THREAD_NAME_LEN];
	ub msg_id;
	ub msg_number;
} ThreadBusy;

/* for MSGID_THREAD_IDLE message */
typedef struct {
	ThreadId thread_id;
	s8 thread_name[DAVE_THREAD_NAME_LEN];
} ThreadIdle;

/* for MSGID_CLIENT_BUSY message */
typedef struct {
	s8 verno[DAVE_VERNO_STR_LEN];
	void *ptr;
} ClientBusy;

/* for MSGID_CLIENT_IDLE message */
typedef struct {
	s8 verno[DAVE_VERNO_STR_LEN];
	void *ptr;
} ClientIdle;

/* for MSGID_REMOTE_THREAD_ID_READY message */
typedef struct {
	ThreadId remote_thread_id;
	s8 remote_thread_name[128];
	s8 globally_identifier[DAVE_GLOBALLY_IDENTIFIER_LEN];
} ThreadRemoteIDReadyMsg;

/* for MSGID_REMOTE_THREAD_ID_REMOVE message */
typedef struct {
	ThreadId remote_thread_id;
	s8 remote_thread_name[128];
	s8 globally_identifier[DAVE_GLOBALLY_IDENTIFIER_LEN];
} ThreadRemoteIDRemoveMsg;

/* for MSGID_LOCAL_THREAD_READY message */
typedef struct {
	ThreadId local_thread_id;
	s8 local_thread_name[128];
} ThreadLocalReadyMsg;

/* for MSGID_LOCAL_THREAD_REMOVE message */
typedef struct {
	ThreadId local_thread_id;
	s8 local_thread_name[128];
} ThreadLocalRemoveMsg;

/* for MSGID_INNER_LOOP message */
typedef struct {
	void *ptr;
} MsgInnerLoop;

/* for MSGID_RPC_DEBUG_REQ message */
typedef struct {
	RetCode ret_debug;
	s8 s8_debug;
	u8 u8_debug;
	s16 s16_debug;
	u16 u16_debug;
	s32 s32_debug;
	u32 u32_debug;
	s64 s64_debug;
	u64 u64_debug;
	float float_debug;
	double double_debug;
	void *void_debug;
	DateStruct date_debug;
	MBUF *mbuf_debug;
	void *ptr;
} RPCDebugReq;

/* for MSGID_RPC_DEBUG_RSP message */
typedef struct {
	RetCode ret_debug;
	s8 s8_debug;
	u8 u8_debug;
	s16 s16_debug;
	u16 u16_debug;
	s32 s32_debug;
	u32 u32_debug;
	s64 s64_debug;
	u64 u64_debug;
	float float_debug;
	double double_debug;
	void *void_debug;
	DateStruct date_debug;
	MBUF *mbuf_debug;
	void *ptr;
} RPCDebugRsp;

/* for MSGID_CFG_UPDATE message */
typedef struct {
	s8 cfg_name[DAVE_NORMAL_NAME_LEN];
	ub cfg_length;
	u8 cfg_value[8196];
} CFGUpdate;

/* for MSGID_BLOCKS_REQ message */
typedef struct {
	BuildingBlocksOpt opt;
	ub blocks_id_1;
	ub blocks_id_2;
	void *ptr;
} MsgBlocksReq;

/* for MSGID_BLOCKS_RSP message */
typedef struct {
	RetCode ret;
	BuildingBlocksOpt opt;
	BuildingBlocks blocks[DAVE_BUILDING_BLOCKS_MAX];
	void *ptr;
} MsgBlocksRsp;

/* for MSGID_OS_NOTIFY message */
typedef struct {
	ub notify_info;
} MsgOSNotify;

/* for MSGID_INTERNAL_LOOP message */
typedef struct {
	ub event_id;
	void *ptr;
} InternalLoop;

/* for MSGID_COROUTINE_WAKEUP message */
typedef struct {
	ub wakeup_id;

	ub thread_index;
	ub wakeup_index;
	s8 some_string[256];
	void *ptr;
} CoroutineWakeup;

#endif

