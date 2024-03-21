/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_base.h"
#include "dave_verno.h"
#include "dave_tools.h"
#include "base_dll_main.h"
#include "dll_log.h"

static inline ThreadId
_dll_thread_src_id(ThreadId src_id)
{
	if(src_id == INVALID_THREAD_ID)
	{
		src_id = self();
	}
	if(src_id == INVALID_THREAD_ID)
	{
		src_id = dave_dll_main_thread_id();
	}

	return src_id;
}

static inline int
_dll_thread_id_msg(unsigned long long dst_id, BaseMsgType msg_type, int msg_id, int msg_len, void *msg_body, char *fun, int line)
{
	ThreadId src_id = _dll_thread_src_id(INVALID_THREAD_ID);

	DLLDEBUG("dst_id:%lx msg_id:%d msg_len:%d msg_body:%lx fun:%s line:%d",
		dst_id, msg_id, msg_len, msg_body, (s8 *)fun, (ub)line);

	if(fun == NULL)
	{
		fun = (char *)__func__;
		line = __LINE__;
	}

	if(base_thread_id_msg(
		NULL, NULL,
		NULL, NULL,
		src_id, (ThreadId)dst_id,
		msg_type,
		(ub)msg_id, (ub)msg_len, (u8 *)msg_body,
		0,
		(s8 *)fun, (ub)line) == dave_true)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

static inline void *
_dll_thread_id_co(unsigned long long dst_id, BaseMsgType msg_type, int req_id, int req_len, void *req_body, int rsp_id, char *fun, int line)
{
	ThreadId src_id = _dll_thread_src_id(INVALID_THREAD_ID);

	DLLDEBUG("dst_id:%lx req_id:%d req_len:%d req_body:%lx rsp_id:%lx fun:%s line:%d",
		dst_id, req_id, req_len, req_body, rsp_id, fun, line);

	if(fun == NULL)
	{
		fun = (char *)__func__;
		line = __LINE__;
	}

	return base_thread_id_co(
		src_id, (ThreadId)dst_id,
		msg_type,
		req_id, req_len, req_body,
		rsp_id,
		(s8 *)fun, (ub)line);
}

static inline int
_dll_thread_name_msg(char *dst_thread, BaseMsgType msg_type, int msg_id, int msg_len, void *msg_body, char *fun, int line)
{
	ThreadId src_id = _dll_thread_src_id(INVALID_THREAD_ID);

	DLLDEBUG("dst_thread:%s msg_id:%d msg_len:%d msg_body:%lx fun:%s line:%d",
		dst_thread, msg_id, msg_len, msg_body, fun, line);

	if(fun == NULL)
	{
		fun = (char *)__func__;
		line = __LINE__;
	}

	if(base_thread_name_msg(
		src_id, (s8 *)dst_thread,
		msg_type,
		(ub)msg_id, (ub)msg_len, (u8 *)msg_body,
		(s8 *)fun, (ub)line) == dave_true)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

static inline void *
_dll_thread_name_co(char *dst_thread, BaseMsgType msg_type, int req_id, int req_len, void *req_body, int rsp_id, char *fun, int line)
{
	ThreadId src_id = _dll_thread_src_id(INVALID_THREAD_ID);

	DLLDEBUG("dst_thread:%s req_id:%d req_len:%d req_body:%lx rsp_id:%lx fun:%s line:%d",
		dst_thread, req_id, req_len, req_body, rsp_id, fun, line);

	if(fun == NULL)
	{
		fun = (char *)__func__;
		line = __LINE__;
	}

	return base_thread_name_co(
		src_id, (s8 *)dst_thread,
		msg_type,
		req_id, req_len, req_body,
		rsp_id,
		(s8 *)fun, (ub)line);
}

static inline int
_dll_thread_gid_msg(char *gid, char *dst_thread, BaseMsgType msg_type, int msg_id, int msg_len, void *msg_body, char *fun, int line)
{
	ThreadId src_id = _dll_thread_src_id(INVALID_THREAD_ID);

	DLLDEBUG("gid:%s dst_thread:%s msg_id:%d msg_len:%d msg_body:%lx fun:%s line:%d",
		gid, dst_thread, msg_id, msg_len, msg_body, fun, line);

	if(fun == NULL)
	{
		fun = (char *)__func__;
		line = __LINE__;
	}

	if(base_thread_gid_msg(
		src_id, (s8 *)gid, (s8 *)dst_thread,
		msg_type,
		(ub)msg_id, (ub)msg_len, (u8 *)msg_body,
		(s8 *)fun, (ub)line) == dave_true)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

static inline void *
_dll_thread_gid_co(char *gid, char *dst_thread, BaseMsgType msg_type, int req_id, int req_len, void *req_body, int rsp_id, char *fun, int line)
{
	ThreadId src_id = _dll_thread_src_id(INVALID_THREAD_ID);

	DLLDEBUG("gid:%s dst_thread:%s req_id:%d req_len:%d req_body:%lx rsp_id:%lx fun:%s line:%d",
		gid, dst_thread, req_id, req_len, req_body, rsp_id, fun, line);

	if(fun == NULL)
	{
		fun = (char *)__func__;
		line = __LINE__;
	}

	return base_thread_gid_co(
		src_id, (s8 *)gid, (s8 *)dst_thread,
		msg_type,
		req_id, req_len, req_body,
		rsp_id,
		(s8 *)fun, (ub)line);
}

static inline int
_dll_thread_uid_msg(char *uid, BaseMsgType msg_type, int msg_id, int msg_len, void *msg_body, char *fun, int line)
{
	ThreadId src_id = _dll_thread_src_id(INVALID_THREAD_ID);

	DLLDEBUG("uid:%s msg_id:%d msg_len:%d msg_body:%lx fun:%s line:%d",
		uid, msg_id, msg_len, msg_body, fun, line);

	if(fun == NULL)
	{
		fun = (char *)__func__;
		line = __LINE__;
	}

	if(base_thread_uid_msg(
		src_id, (s8 *)uid,
		msg_type,
		(ub)msg_id, (ub)msg_len, (u8 *)msg_body,
		(s8 *)fun, (ub)line) == dave_true)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

static inline void *
_dll_thread_uid_co(char *uid, BaseMsgType msg_type, int req_id, int req_len, void *req_body, int rsp_id, char *fun, int line)
{
	ThreadId src_id = _dll_thread_src_id(INVALID_THREAD_ID);

	DLLDEBUG("uid:%s req_id:%d req_len:%d req_body:%lx rsp_id:%lx fun:%s line:%d",
		uid, req_id, req_len, req_body, rsp_id, fun, line);

	if(fun == NULL)
	{
		fun = (char *)__func__;
		line = __LINE__;
	}

	return base_thread_uid_co(
		src_id, (s8 *)uid,
		msg_type,
		req_id, req_len, req_body,
		rsp_id,
		(s8 *)fun, (ub)line);
}

// =====================================================================

int
dave_dll_thread_id(char *thread_name)
{
	ThreadId id;

	id = thread_id(thread_name);

	if((id == INVALID_THREAD_ID) || (id < 0))
		return -1;

	return (int)id;
}

char *
dave_dll_self(void)
{
	ThreadId self_id = self();

	return thread_name(self_id);
}

void *
dave_dll_thread_msg(int msg_len, char *fun, int line)
{
	void *ptr;

	if(fun == NULL)
	{
		fun = (char *)__func__;
		line = __LINE__;
	}

	ptr = base_thread_msg_creat((ub)msg_len, dave_true, (s8 *)fun, (ub)line);

	DLLDEBUG("msg_len:%d ptr:%lx fun:%s line:%d",
		msg_len, ptr, (s8 *)fun, (ub)line);

	return ptr;
}

void
dave_dll_thread_msg_release(void *ptr, char *fun, int line)
{
	if(fun == NULL)
	{
		fun = (char *)__func__;
		line = __LINE__;
	}

	base_thread_msg_release(ptr, (s8 *)fun, (ub)line);
}

int
dave_dll_thread_id_msg(unsigned long long dst_id, int msg_id, int msg_len, void *msg_body, char *fun, int line)
{
	return _dll_thread_id_msg(dst_id, BaseMsgType_Unicast, msg_id, msg_len,msg_body, fun, line);
}

int
dave_dll_thread_id_qmsg(unsigned long long dst_id, int msg_id, int msg_len, void *msg_body, char *fun, int line)
{
	return _dll_thread_id_msg(dst_id, BaseMsgType_Unicast_queue, msg_id, msg_len,msg_body, fun, line);
}

void *
dave_dll_thread_id_co(unsigned long long dst_id, int req_id, int req_len, void *req_body, int rsp_id, char *fun, int line)
{
	return _dll_thread_id_co(dst_id, BaseMsgType_Unicast, req_id, req_len, req_body, rsp_id, fun, line);
}

void *
dave_dll_thread_id_qco(unsigned long long dst_id, int req_id, int req_len, void *req_body, int rsp_id, char *fun, int line)
{
	return _dll_thread_id_co(dst_id, BaseMsgType_Unicast_queue, req_id, req_len, req_body, rsp_id, fun, line);
}

int
dave_dll_thread_name_msg(char *dst_thread, int msg_id, int msg_len, void *msg_body, char *fun, int line)
{
	return _dll_thread_name_msg(dst_thread, BaseMsgType_Unicast, msg_id, msg_len, msg_body, fun, line);
}

int
dave_dll_thread_name_qmsg(char *dst_thread, int msg_id, int msg_len, void *msg_body, char *fun, int line)
{
	return _dll_thread_name_msg(dst_thread, BaseMsgType_Unicast_queue, msg_id, msg_len, msg_body, fun, line);
}

void *
dave_dll_thread_name_co(char *dst_thread, int req_id, int req_len, void *req_body, int rsp_id, char *fun, int line)
{
	return _dll_thread_name_co(dst_thread, BaseMsgType_Unicast, req_id, req_len, req_body, rsp_id, fun, line);
}

void *
dave_dll_thread_name_qco(char *dst_thread, int req_id, int req_len, void *req_body, int rsp_id, char *fun, int line)
{
	return _dll_thread_name_co(dst_thread, BaseMsgType_Unicast_queue, req_id, req_len, req_body, rsp_id, fun, line);
}

int
dave_dll_thread_gid_msg(char *gid, char *dst_thread, int msg_id, int msg_len, void *msg_body, char *fun, int line)
{
	return _dll_thread_gid_msg(gid, dst_thread, BaseMsgType_Unicast, msg_id, msg_len, msg_body, fun, line);
}

int
dave_dll_thread_gid_qmsg(char *gid, char *dst_thread, int msg_id, int msg_len, void *msg_body, char *fun, int line)
{
	return _dll_thread_gid_msg(gid, dst_thread, BaseMsgType_Unicast_queue, msg_id, msg_len, msg_body, fun, line);
}

void *
dave_dll_thread_gid_co(char *gid, char *dst_thread, int req_id, int req_len, void *req_body, int rsp_id, char *fun, int line)
{
	return _dll_thread_gid_co(gid, dst_thread, BaseMsgType_Unicast, req_id, req_len, req_body, rsp_id, fun, line);
}

void *
dave_dll_thread_gid_qco(char *gid, char *dst_thread, int req_id, int req_len, void *req_body, int rsp_id, char *fun, int line)
{
	return _dll_thread_gid_co(gid, dst_thread, BaseMsgType_Unicast_queue, req_id, req_len, req_body, rsp_id, fun, line);
}

int
dave_dll_thread_uid_msg(char *uid, int msg_id, int msg_len, void *msg_body, char *fun, int line)
{
	return _dll_thread_uid_msg(uid, BaseMsgType_Unicast, msg_id, msg_len, msg_body, fun, line);
}

int
dave_dll_thread_uid_qmsg(char *uid, int msg_id, int msg_len, void *msg_body, char *fun, int line)
{
	return _dll_thread_uid_msg(uid, BaseMsgType_Unicast_queue, msg_id, msg_len, msg_body, fun, line);
}

void *
dave_dll_thread_uid_co(char *uid, int req_id, int req_len, void *req_body, int rsp_id, char *fun, int line)
{
	return _dll_thread_uid_co(uid, BaseMsgType_Unicast, req_id, req_len, req_body, rsp_id, fun, line);
}

void *
dave_dll_thread_uid_qco(char *uid, int req_id, int req_len, void *req_body, int rsp_id, char *fun, int line)
{
	return _dll_thread_uid_co(uid, BaseMsgType_Unicast_queue, req_id, req_len, req_body, rsp_id, fun, line);
}

void *
dave_dll_thread_sync_msg(char *dst_thread, int req_id, int req_len, void *req_body, int rsp_id, int rsp_len, void *rsp_body, char *fun, int line)
{
	ThreadId src_id, dst_id;

	if(fun == NULL)
	{
		fun = (char *)__func__;
		line = __LINE__;
	}

	src_id = _dll_thread_src_id(INVALID_THREAD_ID);
	dst_id = thread_id(dst_thread);
	if(dst_id == INVALID_THREAD_ID)
	{
		DLLLOG("invalid dst_id on thread_name:%s!", dst_thread);
		return NULL;
	}

	return base_thread_sync_msg(
		src_id, dst_id,
		(ub)req_id, (ub)req_len, (u8 *)req_body,
		(ub)rsp_id, (ub)rsp_len, (u8 *)rsp_body,
		(s8 *)fun, (ub)line);
}

int
dave_dll_thread_broadcast_msg(char *thread_name, int msg_id, int msg_len, void *msg_body, char *fun, int line)
{
	BaseMsgType type;

	DLLDEBUG("thread_name:%s msg_id:%d msg_len:%d msg_body:%lx fun:%s line:%d",
		thread_name, msg_id, msg_len, msg_body, fun, line);

	if(fun == NULL)
	{
		fun = (char *)__func__;
		line = __LINE__;
	}

	type = (thread_name == NULL ? BaseMsgType_Broadcast_remote : (thread_name[0] == '\0' ? BaseMsgType_Broadcast_remote : BaseMsgType_Broadcast_thread));

	if(base_thread_broadcast_msg(
		type,
		(s8 *)thread_name, (ub)msg_id, (ub)msg_len, (u8 *)msg_body,
		(s8 *)fun, (ub)line) == dave_true)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

#endif

