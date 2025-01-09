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
	ThreadId run_thread_id;
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

/* for MSGID_ECHO_REQ message */
typedef struct {
	MsgIdEcho echo;
	void *ptr;
} MsgIdEchoReq;

/* for MSGID_ECHO_RSP message */
typedef struct {
	MsgIdEcho echo;
	void *ptr;
} MsgIdEchoRsp;

/* for MSGID_INTERNAL_EVENTS message */
typedef struct {
	ub event_id;
	void *ptr;
} InternalEvents;

/* for MSGID_THREAD_BUSY message */
typedef struct {
	ThreadId thread_id;
	s8 thread_name[DAVE_THREAD_NAME_LEN];
	ub msg_number;
} ThreadBusy;

/* for MSGID_THREAD_IDLE message */
typedef struct {
	ThreadId thread_id;
	s8 thread_name[DAVE_THREAD_NAME_LEN];
} ThreadIdle;

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
	ub thread_flag;
} ThreadLocalReadyMsg;

/* for MSGID_LOCAL_THREAD_REMOVE message */
typedef struct {
	ThreadId local_thread_id;
	s8 local_thread_name[128];
	ub thread_flag;
} ThreadLocalRemoveMsg;

/* for MSGID_INNER_LOOP message */
typedef struct {
	MBUF *param;
	void *ptr;
} MsgInnerLoop;

/* for MSGID_CFG_UPDATE message */
typedef struct {
	s8 cfg_name[DAVE_NORMAL_NAME_LEN];
	ub cfg_length;
	u8 cfg_value[8196];
} CFGUpdate;

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
	void *ptr;
} CoroutineWakeup;

/* for MSGID_RPC_DEBUG_REQ message */
typedef struct {
	RetCode ret_debug;
	s8 req_thread[64];
	s8 str_debug[16];
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
	ub req_time;
	ub rsp_time;
	void *ptr;
} RPCDebugReq;

/* for MSGID_RPC_DEBUG_RSP message */
typedef struct {
	RetCode ret_debug;
	s8 rsp_thread[64];
	s8 str_debug[16];
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
	ub req_time;
	ub rsp_time;
	void *ptr;
} RPCDebugRsp;

/* for MSGID_CFG_REMOTE_UPDATE message */
typedef struct {
	dave_bool put_flag;
	s8 cfg_name[1024];
	s8 cfg_value[262144];
	sb ttl;
} CFGRemoteUpdate;

/* for MSGID_DOS_FORWARD message */
typedef struct {
	MBUF *cmd;
	MBUF *param;
	void *ptr;
} DosForward;

/* for MSGID_CFG_REMOTE_SYNC_UPDATE message */
typedef struct {
	dave_bool put_flag;
	MBUF *cfg_mbuf_name;
	MBUF *cfg_mbuf_value;
	sb ttl;
} CFGRemoteSyncUpdate;

/* for MSGID_QUEUE_UPLOAD_MESSAGE_REQ message */
typedef struct {
	s8 src_name[DAVE_THREAD_NAME_LEN];
	s8 dst_name[DAVE_THREAD_NAME_LEN];
	s8 src_gid[DAVE_GLOBALLY_IDENTIFIER_LEN];
	s8 dst_gid[DAVE_GLOBALLY_IDENTIFIER_LEN];
	ub msg_id;
	MBUF *msg;
	void *ptr;
} QueueUploadMsgReq;

/* for MSGID_QUEUE_UPLOAD_MESSAGE_RSP message */
typedef struct {
	RetCode ret;
	void *ptr;
} QueueUploadMsgRsp;

/* for MSGID_QUEUE_DOWNLOAD_MESSAGE_REQ message */
typedef struct {
	s8 name[DAVE_THREAD_NAME_LEN];
	s8 gid[DAVE_GLOBALLY_IDENTIFIER_LEN];
	void *ptr;
} QueueDownloadMsgReq;

/* for MSGID_QUEUE_DOWNLOAD_MESSAGE_RSP message */
typedef struct {
	RetCode ret;
	s8 src_name[DAVE_THREAD_NAME_LEN];
	s8 dst_name[DAVE_THREAD_NAME_LEN];
	s8 src_gid[DAVE_GLOBALLY_IDENTIFIER_LEN];
	s8 dst_gid[DAVE_GLOBALLY_IDENTIFIER_LEN];
	MBUF *msg;
	void *ptr;
} QueueDownloadMsgRsp;

/* for MSGID_QUEUE_UPDATE_STATE_REQ message */
typedef struct {
	s8 src_name[DAVE_THREAD_NAME_LEN];
	s8 dst_name[DAVE_THREAD_NAME_LEN];
	s8 src_gid[DAVE_GLOBALLY_IDENTIFIER_LEN];
	s8 dst_gid[DAVE_GLOBALLY_IDENTIFIER_LEN];
	ub msg_number;
	MBUF *msg;
	s8 queue_gid[DAVE_GLOBALLY_IDENTIFIER_LEN];
	void *ptr;
} QueueUpdateStateReq;

/* for MSGID_QUEUE_UPDATE_STATE_RSP message */
typedef struct {
	RetCode ret;
	s8 src_name[DAVE_THREAD_NAME_LEN];
	s8 dst_name[DAVE_THREAD_NAME_LEN];
	s8 src_gid[DAVE_GLOBALLY_IDENTIFIER_LEN];
	s8 dst_gid[DAVE_GLOBALLY_IDENTIFIER_LEN];
	MBUF *msg;
	void *ptr;
} QueueUpdateStateRsp;

/* for MSGID_QUEUE_RUN_MESSAGE_REQ message */
typedef struct {
	s8 src_name[DAVE_THREAD_NAME_LEN];
	s8 dst_name[DAVE_THREAD_NAME_LEN];
	s8 src_gid[DAVE_GLOBALLY_IDENTIFIER_LEN];
	s8 dst_gid[DAVE_GLOBALLY_IDENTIFIER_LEN];
	MBUF *msg;
	void *ptr;
} QueueRunMsgReq;

/* for MSGID_QUEUE_RUN_MESSAGE_RSP message */
typedef struct {
	RetCode ret;
	s8 name[DAVE_THREAD_NAME_LEN];
	ub msg_number;
	ub thread_number;
	void *ptr;
} QueueRunMsgRsp;

/* for MSGID_SYSTEM_BUSY message */
typedef struct {
	s8 gid[DAVE_GLOBALLY_IDENTIFIER_LEN];
	s8 verno[DAVE_VERNO_STR_LEN];
	void *ptr;
} SystemBusy;

/* for MSGID_SYSTEM_IDLE message */
typedef struct {
	s8 gid[DAVE_GLOBALLY_IDENTIFIER_LEN];
	s8 verno[DAVE_VERNO_STR_LEN];
	void *ptr;
} SystemIdle;

/* for MSGID_GENERAL_REQ message */
typedef struct {
	s8 general_type[256];
	MBUF *general_data;
	MBUF *general_bin;
	ub send_req_us_time;
	void *ptr;
} GeneralReq;

/* for MSGID_GENERAL_RSP message */
typedef struct {
	s8 general_type[256];
	MBUF *general_data;
	MBUF *general_bin;
	ub send_req_us_time;
	ub recv_req_us_time;
	ub send_rsp_us_time;
	void *ptr;
} GeneralRsp;

/* for MSGID_APPLICATION_BUSY message */
typedef struct {
	dave_bool cfg_flag;
	void *ptr;
} ApplicationBusy;

/* for MSGID_APPLICATION_IDLE message */
typedef struct {
	dave_bool cfg_flag;
	void *ptr;
} ApplicationIdle;

/* for MSGID_PROTECTOR_REG message */
typedef struct {
	void *ptr;
} ProtectorReg;

/* for MSGID_PROTECTOR_UNREG message */
typedef struct {
	void *ptr;
} ProtectorUnreg;

/* for MSGID_PROTECTOR_WAKEUP message */
typedef struct {
	void *ptr;
} ProtectorWakeup;

/* for FREE_MESSAGE_AREA_1 message */
typedef struct {
	s8 general_type[256];
	MBUF *general_data;
	ub send_req_us_time;
	ub recv_req_us_time;
	ub send_rsp_us_time;
	void *ptr;
} FreeMessageArea1;

/* for FREE_MESSAGE_AREA_2 message */
typedef struct {
	s8 general_type[256];
	MBUF *general_data;
	ub send_req_us_time;
	ub recv_req_us_time;
	ub send_rsp_us_time;
	void *ptr;
} FreeMessageArea2;

/* for FREE_MESSAGE_AREA_3 message */
typedef struct {
	s8 general_type[256];
	MBUF *general_data;
	ub send_req_us_time;
	ub recv_req_us_time;
	ub send_rsp_us_time;
	void *ptr;
} FreeMessageArea3;

/* for FREE_MESSAGE_AREA_4 message */
typedef struct {
	s8 general_type[256];
	MBUF *general_data;
	ub send_req_us_time;
	ub recv_req_us_time;
	ub send_rsp_us_time;
	void *ptr;
} FreeMessageArea4;

/* for FREE_MESSAGE_AREA_5 message */
typedef struct {
	s8 general_type[256];
	MBUF *general_data;
	ub send_req_us_time;
	ub recv_req_us_time;
	ub send_rsp_us_time;
	void *ptr;
} FreeMessageArea5;

#endif

