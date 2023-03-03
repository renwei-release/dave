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

static ThreadId
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

// =====================================================================

void *
dave_dll_thread_msg(int msg_len, char *fun, int line)
{
	void *ptr;

	ptr = base_thread_msg_creat((ub)msg_len, dave_true, (s8 *)__func__, (ub)__LINE__);

	DLLDEBUG("msg_len:%d ptr:%lx fun:%s line:%d",
		msg_len, ptr, (s8 *)__func__, (ub)__LINE__);

	return ptr;
}

void
dave_dll_thread_msg_release(void *ptr, char *fun, int line)
{
	base_thread_msg_release(ptr, (s8 *)__func__, (ub)__LINE__);
}

int
dave_dll_thread_id_msg(unsigned long long dst_id, int msg_id, int msg_len, void *msg_body, char *fun, int line)
{
	ThreadId src_id = _dll_thread_src_id(INVALID_THREAD_ID);

	DLLDEBUG("dst_id:%lx msg_id:%d msg_len:%d msg_body:%lx fun:%s line:%d",
		dst_id, msg_id, msg_len, msg_body, (s8 *)__func__, (ub)__LINE__);

	if(base_thread_id_msg(
		NULL, NULL,
		NULL, NULL,
		src_id, (ThreadId)dst_id,
		BaseMsgType_Unicast,
		(ub)msg_id, (ub)msg_len, (u8 *)msg_body,
		0,
		(s8 *)__func__, (ub)__LINE__) == dave_true)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

void *
dave_dll_thread_id_co(unsigned long long dst_id, int req_id, int req_len, void *req_body, int rsp_id, char *fun, int line)
{
	ThreadId src_id = _dll_thread_src_id(INVALID_THREAD_ID);

	DLLDEBUG("dst_id:%lx req_id:%d req_len:%d req_body:%lx rsp_id:%lx fun:%s line:%d",
		dst_id, req_id, req_len, req_body, rsp_id, fun, line);

	return base_thread_id_co(
		src_id, (ThreadId)dst_id,
		req_id, req_len, req_body,
		rsp_id,
		(s8 *)__func__, (ub)__LINE__);
}

int
dave_dll_thread_name_msg(char *dst_thread, int msg_id, int msg_len, void *msg_body, char *fun, int line)
{
	ThreadId src_id = _dll_thread_src_id(INVALID_THREAD_ID);

	DLLDEBUG("dst_thread:%s msg_id:%d msg_len:%d msg_body:%lx fun:%s line:%d",
		dst_thread, msg_id, msg_len, msg_body, fun, line);

	if(base_thread_name_msg(
		src_id, (s8 *)dst_thread,
		(ub)msg_id, (ub)msg_len, (u8 *)msg_body,
		(s8 *)__func__, (ub)__LINE__) == dave_true)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

void *
dave_dll_thread_name_co(char *dst_thread, int req_id, int req_len, void *req_body, int rsp_id, char *fun, int line)
{
	ThreadId src_id = _dll_thread_src_id(INVALID_THREAD_ID);

	DLLDEBUG("dst_thread:%s req_id:%d req_len:%d req_body:%lx rsp_id:%lx fun:%s line:%d",
		dst_thread, req_id, req_len, req_body, rsp_id, fun, line);

	return base_thread_name_co(
		src_id, (s8 *)dst_thread,
		req_id, req_len, req_body,
		rsp_id,
		(s8 *)__func__, (ub)__LINE__);
}

int
dave_dll_thread_gid_msg(char *gid, char *dst_thread, int msg_id, int msg_len, void *msg_body, char *fun, int line)
{
	ThreadId src_id = _dll_thread_src_id(INVALID_THREAD_ID);

	DLLDEBUG("gid:%s dst_thread:%s msg_id:%d msg_len:%d msg_body:%lx fun:%s line:%d",
		gid, dst_thread, msg_id, msg_len, msg_body, fun, line);

	if(base_thread_gid_msg(
		src_id, (s8 *)gid, (s8 *)dst_thread,
		(ub)msg_id, (ub)msg_len, (u8 *)msg_body,
		(s8 *)__func__, (ub)__LINE__) == dave_true)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

void *
dave_dll_thread_gid_co(char *gid, char *dst_thread, int req_id, int req_len, void *req_body, int rsp_id, char *fun, int line)
{
	ThreadId src_id = _dll_thread_src_id(INVALID_THREAD_ID);

	DLLDEBUG("gid:%s dst_thread:%s req_id:%d req_len:%d req_body:%lx rsp_id:%lx fun:%s line:%d",
		gid, dst_thread, req_id, req_len, req_body, rsp_id, fun, line);

	return base_thread_gid_co(
		src_id, (s8 *)gid, (s8 *)dst_thread,
		req_id, req_len, req_body,
		rsp_id,
		(s8 *)__func__, (ub)__LINE__);
}

int
dave_dll_thread_uid_msg(char *uid, int msg_id, int msg_len, void *msg_body, char *fun, int line)
{
	ThreadId src_id = _dll_thread_src_id(INVALID_THREAD_ID);

	DLLDEBUG("uid:%s msg_id:%d msg_len:%d msg_body:%lx fun:%s line:%d",
		uid, msg_id, msg_len, msg_body, fun, line);

	if(base_thread_uid_msg(
		src_id, (s8 *)uid,
		(ub)msg_id, (ub)msg_len, (u8 *)msg_body,
		(s8 *)__func__, (ub)__LINE__) == dave_true)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

void *
dave_dll_thread_uid_co(char *uid, int req_id, int req_len, void *req_body, int rsp_id, char *fun, int line)
{
	ThreadId src_id = _dll_thread_src_id(INVALID_THREAD_ID);

	DLLDEBUG("uid:%s req_id:%d req_len:%d req_body:%lx rsp_id:%lx fun:%s line:%d",
		uid, req_id, req_len, req_body, rsp_id, fun, line);

	return base_thread_uid_co(
		src_id, (s8 *)uid,
		req_id, req_len, req_body,
		rsp_id,
		(s8 *)__func__, (ub)__LINE__);
}

void *
dave_dll_thread_sync_msg(char *dst_thread, int req_id, int req_len, void *req_body, int rsp_id, int rsp_len, void *rsp_body, char *fun, int line)
{
	ThreadId src_id, dst_id;

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
		(s8 *)__func__, (ub)__LINE__);
}

int
dave_dll_thread_broadcast_msg(char *thread_name, int msg_id, int msg_len, void *msg_body, char *fun, int line)
{
	BaseMsgType type;

	DLLDEBUG("thread_name:%s msg_id:%d msg_len:%d msg_body:%lx fun:%s line:%d",
		thread_name, msg_id, msg_len, msg_body, fun, line);

	type = (thread_name == NULL ? BaseMsgType_Broadcast_total : BaseMsgType_Broadcast_thread);

	if(base_thread_broadcast_msg(
		type,
		(s8 *)thread_name, (ub)msg_id, (ub)msg_len, (u8 *)msg_body,
		(s8 *)__func__, (ub)__LINE__) == dave_true)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

#endif

