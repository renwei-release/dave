/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __BASE_THREAD_H__
#define __BASE_THREAD_H__

#define SYNC_CLIENT_THREAD_NAME "syncc"
#define SYNC_SERVER_THREAD_NAME "syncs"
#define LOG_CLIENT_THREAD_NAME "logc"
#define LOG_SERVER_THREAD_NAME "logs"
#define TIMER_THREAD_NAME "timer"
#define GUARDIAN_THREAD_NAME "g"

#define INVALID_THREAD_ID (0xffffffffffffffff)

typedef u64 ThreadId;

#define THREAD_MSG_WAKEUP (0x00000001)
#define THREAD_TICK_WAKEUP (0x00000002)
#define THREAD_THREAD_FLAG (0x00000004)
#define THREAD_REMOTE_FLAG (0x00000010)
#define THREAD_TRACE_FLAG (0x00000020)
#define THREAD_PRIVATE_FLAG (0x00000040)
#define THREAD_COROUTINE_FLAG (0x00000080)
#define THREAD_dCOROUTINE_FLAG (0x00000100)
#define THREAD_CORE_FLAG (0x00000200)

typedef enum {
	RESERVED_TASK_ATTRIB,
	LOCAL_TASK_ATTRIB,
	REMOTE_TASK_ATTRIB,
	EMPTY_TASK_ATTRIB,
} TaskAttribute;

typedef enum {
	BaseMsgType_Unicast = 0,
	BaseMsgType_Broadcast_thread,
	BaseMsgType_Broadcast_local,
	BaseMsgType_Broadcast_remote,
	BaseMsgType_Broadcast_total,
	BaseMsgType_Broadcast_dismiss,
} BaseMsgType;

typedef enum {
	MsgMemState_uncaptured,
	MsgMemState_captured,
} MsgMemState;

typedef struct {
	ThreadId msg_src;
	ThreadId msg_dst;
	ub msg_id;
	BaseMsgType msg_type;
	TaskAttribute src_attrib;
	TaskAttribute dst_attrib;

	ub msg_len;
	void *msg_body;

	MsgMemState mem_state;

	ub thread_wakeup_index;

	ub msg_build_time;
	ub msg_build_serial;

	void *user_ptr;

	void *msg_chain;
	void *msg_router;

	s8 src_gid[DAVE_GLOBALLY_IDENTIFIER_LEN];
	s8 src_name[DAVE_THREAD_NAME_LEN];
} MSGBODY;

typedef void (*base_thread_fun)(MSGBODY *msg);

void base_thread_init(void *main_thread_id, s8 *sync_domain);
void base_thread_exit(void);
void base_thread_schedule(void);

ThreadId base_thread_creat(char *name, ub level_number, ub thread_flag, base_thread_fun thread_init, base_thread_fun thread_main, base_thread_fun thread_exit);
dave_bool base_thread_del(ThreadId thread_id);
ThreadId base_thread_get_self(s8 *fun, ub line);
ub base_thread_name_array(s8 thread_name[][64], ub thread_number);
dave_bool __base_thread_trace_state__(s8 *fun, ub line);
RetCode base_thread_msg_register(ThreadId thread_id, ub msg_id, base_thread_fun msg_fun, void *user_ptr);
void base_thread_msg_unregister(ThreadId thread_id, ub msg_id);

ThreadId base_thread_get_local(ThreadId thread_id);
ThreadId base_thread_get_id(const s8 *name, s8 *fun, ub line);
TaskAttribute base_thread_attrib(ThreadId thread_id);
s8 *base_thread_get_name(ThreadId thread_id, s8 *fun, ub line);
void * base_thread_msg(ub msg_len, dave_bool reset, s8 *fun, ub line);
void base_thread_msg_release(void *ptr, s8 *fun, ub line);

dave_bool base_thread_id_msg(void *msg_chain, void *msg_router, s8 *src_gid, s8 *src_name, ThreadId src_id, ThreadId dst_id, BaseMsgType msg_type, ub msg_id, ub msg_len, u8 *msg_body, ub msg_number, s8 *fun, ub line);
dave_bool base_thread_id_event(ThreadId src_id, ThreadId dst_id, BaseMsgType msg_type, ub req_id, ub req_len, u8 *req_body, ub rsp_id, base_thread_fun rsp_fun, s8 *fun, ub line);
void * base_thread_id_co(ThreadId src_id, ThreadId dst_id, ub req_id, ub req_len, u8 *req_body, ub rsp_id, s8 *fun, ub line);

dave_bool base_thread_name_msg(ThreadId src_id, s8 *dst_thread, ub msg_id, ub msg_len, u8 *msg_body, s8 *fun, ub line);
dave_bool base_thread_name_event(ThreadId src_id, s8 *dst_thread, ub req_id, ub req_len, u8 *req_body, ub rsp_id, base_thread_fun rsp_fun, s8 *fun, ub line);
void * base_thread_name_co(ThreadId src_id, s8 *dst_thread, ub req_id, ub req_len, u8 *req_body, ub rsp_id, s8 *fun, ub line);

dave_bool base_thread_gid_msg(ThreadId src_id, s8 *gid, s8 *dst_thread, ub msg_id, ub msg_len, u8 *msg_body, s8 *fun, ub line);
dave_bool base_thread_gid_event(ThreadId src_id, s8 *gid, s8 *dst_thread, ub req_id, ub req_len, u8 *req_body, ub rsp_id, base_thread_fun rsp_fun, s8 *fun, ub line);
void * base_thread_gid_co(ThreadId src_id, s8 *gid, s8 *dst_thread, ub req_id, ub req_len, u8 *req_body, ub rsp_id, s8 *fun, ub line);

dave_bool base_thread_uid_msg(ThreadId src_id, s8 *uid, ub msg_id, ub msg_len, u8 *msg_body, s8 *fun, ub line);
dave_bool base_thread_uid_event(ThreadId src_id, s8 *uid, ub req_id, ub req_len, u8 *req_body, ub rsp_id, base_thread_fun rsp_fun, s8 *fun, ub line);
void * base_thread_uid_co(ThreadId src_id, s8 *uid, ub req_id, ub req_len, u8 *req_body, ub rsp_id, s8 *fun, ub line);

void * base_thread_sync_msg(ThreadId src_id, ThreadId dst_id, ub req_id, ub req_len, u8 *req_body, ub rsp_id, ub rsp_len, u8 *rsp_body, s8 *fun, ub line);

dave_bool base_thread_broadcast_msg(BaseMsgType type, s8 *dst_name, ub msg_id, ub msg_len, u8 *msg_body, s8 *fun, ub line);

#define base_thread_trace_state() __base_thread_trace_state__((s8 *)__func__, (ub)__LINE__)

#define get_self() base_thread_get_self((s8 *)__func__, (ub)__LINE__)
#define self() base_thread_get_self((s8 *)__func__, (ub)__LINE__)
#define get_thread_id(name) base_thread_get_id((const s8 *)name, (s8 *)__func__, (ub)__LINE__)

#define thread_id(name) base_thread_get_id((const s8 *)name, (s8 *)__func__, (ub)__LINE__)
#define thread_attrib(thread_id) base_thread_attrib(thread_id)

#define get_thread_name(thread_id) base_thread_get_name(thread_id, (s8 *)__func__, (ub)__LINE__)
#define thread_name(thread_id) base_thread_get_name(thread_id, (s8 *)__func__, (ub)__LINE__)

#define thread_reset_msg(msg_body) base_thread_msg(sizeof(*msg_body), dave_true, (s8 *)__func__, (ub)__LINE__)
#define thread_msg(msg_body) base_thread_msg(sizeof(*msg_body), dave_false, (s8 *)__func__, (ub)__LINE__)
#define thread_msg_release(msg_body) base_thread_msg_release(msg_body, (s8 *)__func__, (ub)__LINE__)

#define snd_msg(dst_id, msg_id, msg_len, msg_body) base_thread_id_msg(NULL, NULL, NULL, NULL, INVALID_THREAD_ID, dst_id, BaseMsgType_Unicast, (ub)msg_id, (ub)msg_len, (u8 *)(msg_body), 0, (s8 *)__func__, (ub)__LINE__)
#define snd_from_msg(src_id, dst_id, msg_id, msg_len, msg_body) base_thread_id_msg(NULL, NULL, NULL, NULL, src_id, dst_id, BaseMsgType_Unicast, (ub)msg_id, (ub)msg_len, (u8 *)(msg_body), 0, (s8 *)__func__, (ub)__LINE__)

#define id_msg(dst_id, msg_id, msg_body) base_thread_id_msg(NULL, NULL, NULL, NULL, INVALID_THREAD_ID, dst_id, BaseMsgType_Unicast, (ub)msg_id, sizeof(*msg_body), (u8 *)(msg_body), 0, (s8 *)__func__, (ub)__LINE__)
#define id_nmsg(dst_id, msg_id, msg_body, msg_number) base_thread_id_msg(NULL, NULL, NULL, NULL, INVALID_THREAD_ID, dst_id, BaseMsgType_Unicast, (ub)msg_id, sizeof(*msg_body), (u8 *)(msg_body), msg_number, (s8 *)__func__, (ub)__LINE__)
#define id_event(dst_id, req_id, req_body, rsp_id, rsp_fun) base_thread_id_event(INVALID_THREAD_ID, dst_id, BaseMsgType_Unicast, (ub)req_id, sizeof(*req_body), (u8 *)(req_body), (ub)rsp_id, rsp_fun, (s8 *)__func__, (ub)__LINE__)
#define id_go(dst_id, req_id, req_body, rsp_id) base_thread_id_co(INVALID_THREAD_ID, dst_id, (ub)req_id, sizeof(*req_body), (u8 *)(req_body), (ub)rsp_id, (s8 *)__func__, (ub)__LINE__)

#define name_msg(dst_thread, msg_id, msg_body) base_thread_name_msg(INVALID_THREAD_ID, (s8 *)dst_thread, (ub)msg_id, sizeof(*msg_body), (u8 *)(msg_body), (s8 *)__func__, (ub)__LINE__)
#define name_event(dst_thread, req_id, req_body, rsp_id, rsp_fun) base_thread_name_event(INVALID_THREAD_ID, (s8 *)dst_thread, (ub)req_id, sizeof(*req_body), (u8 *)(req_body), (ub)rsp_id, rsp_fun, (s8 *)__func__, (ub)__LINE__)
#define name_go(dst_thread, req_id, req_body, rsp_id) base_thread_name_co(INVALID_THREAD_ID, (s8 *)dst_thread, (ub)req_id, sizeof(*req_body), (u8 *)req_body, (ub)rsp_id, (s8 *)__func__, (ub)__LINE__)

#define gid_msg(gid, dst_thread, msg_id, msg_body) base_thread_gid_msg(INVALID_THREAD_ID, gid, (s8 *)dst_thread, (ub)msg_id, sizeof(*msg_body), (u8 *)(msg_body), (s8 *)__func__, (ub)__LINE__)
#define gid_event(gid, dst_thread, req_id, req_body, rsp_id, rsp_fun) base_thread_gid_event(INVALID_THREAD_ID, gid, (s8 *)dst_thread, (ub)req_id, sizeof(*req_body), (u8 *)(req_body), (ub)rsp_id, rsp_fun, (s8 *)__func__, (ub)__LINE__)
#define gid_go(gid, dst_thread, req_id, req_body, rsp_id) base_thread_gid_co(INVALID_THREAD_ID, gid, (s8 *)dst_thread, (ub)req_id, sizeof(*req_body), (u8 *)req_body, (ub)rsp_id, (s8 *)__func__, (ub)__LINE__)

#define uid_msg(uid, msg_id, msg_body) base_thread_uid_msg(INVALID_THREAD_ID, uid, (ub)msg_id, sizeof(*msg_body), (u8 *)(msg_body), (s8 *)__func__, (ub)__LINE__)
#define uid_event(uid, req_id, req_body, rsp_id, rsp_fun) base_thread_uid_event(INVALID_THREAD_ID, uid, (ub)req_id, sizeof(*req_body), (u8 *)(req_body), (ub)rsp_id, rsp_fun, (s8 *)__func__, (ub)__LINE__)
#define uid_go(uid, req_id, req_body, rsp_id) base_thread_uid_co(INVALID_THREAD_ID, uid, (ub)req_id, sizeof(*req_body), (u8 *)req_body, (ub)rsp_id, (s8 *)__func__, (ub)__LINE__)

#define sync_msg(dst_id, req_id, req_body, rsp_id, rsp_body) base_thread_sync_msg(INVALID_THREAD_ID, dst_id, (ub)req_id, sizeof(*req_body), (u8 *)(req_body), (ub)rsp_id, sizeof(*rsp_body), (u8 *)(rsp_body), (s8 *)__func__, (ub)__LINE__)

#define broadcast_thread(thread_name, msg_id, msg_body) base_thread_broadcast_msg(BaseMsgType_Broadcast_thread, thread_name, (ub)msg_id, sizeof(*msg_body), (u8 *)(msg_body), (s8 *)__func__, (ub)__LINE__)
#define broadcast_local(msg_id, msg_body) base_thread_broadcast_msg(BaseMsgType_Broadcast_local, NULL, (ub)msg_id, sizeof(*msg_body), (u8 *)(msg_body), (s8 *)__func__, (ub)__LINE__)
#define broadcast_remote(msg_id, msg_body) base_thread_broadcast_msg(BaseMsgType_Broadcast_remote, NULL, (ub)msg_id, sizeof(*msg_body), (u8 *)(msg_body), (s8 *)__func__, (ub)__LINE__)
#define broadcast_total(msg_id, msg_body) base_thread_broadcast_msg(BaseMsgType_Broadcast_total, NULL, (ub)msg_id, sizeof(*msg_body), (u8 *)(msg_body), (s8 *)__func__, (ub)__LINE__)
#define broadcast_dismiss(thread_name, msg_id) base_thread_broadcast_msg(BaseMsgType_Broadcast_dismiss, thread_name, (ub)msg_id, 0, NULL, (s8 *)__func__, (ub)__LINE__)

#define reg_msg(msg_id, msg_fun) base_thread_msg_register(INVALID_THREAD_ID, (ub)msg_id, msg_fun, NULL)
#define reg_msgptr(msg_id, msg_fun, user_ptr) base_thread_msg_register(INVALID_THREAD_ID, (ub)msg_id, msg_fun, user_ptr)
#define unreg_msg(msg_id) base_thread_msg_unregister(INVALID_THREAD_ID, (ub)msg_id)

#define inner_loop(fun) { MsgInnerLoop loop; reg_msg(MSGID_INNER_LOOP, fun); id_msg(self(), MSGID_INNER_LOOP, &loop); }

#endif

