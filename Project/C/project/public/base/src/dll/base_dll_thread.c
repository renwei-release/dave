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

// =====================================================================

void *
dave_dll_thread_msg(int msg_len, char *fun, int line)
{
	void *ptr;

	ptr = base_thread_msg((ub)msg_len, dave_true, (s8 *)fun, (ub)line);

	DLLDEBUG("msg_len:%d ptr:%lx fun:%s line:%d",
		msg_len, ptr, fun, line);

	return ptr;
}

void
dave_dll_thread_msg_release(void *ptr, char *fun, int line)
{
	base_thread_msg_release(ptr, (s8 *)fun, (ub)line);
}

int
dave_dll_thread_id_msg(unsigned long long dst_id, int msg_id, int msg_len, void *msg_body, char *fun, int line)
{
	DLLDEBUG("thread_name:%s msg_id:%d msg_len:%d msg_body:%lx fun:%s line:%d",
		thread_name, msg_id, msg_len, msg_body, fun, line);

	if(base_thread_id_msg(
		dave_dll_main_thread_id(), (ThreadId)dst_id,
		BaseMsgType_Unicast,
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

int
dave_dll_thread_name_msg(char *thread_name, int msg_id, int msg_len, void *msg_body, char *fun, int line)
{
	DLLDEBUG("thread_name:%s msg_id:%d msg_len:%d msg_body:%lx fun:%s line:%d",
		thread_name, msg_id, msg_len, msg_body, fun, line);

	if(base_thread_name_msg(
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

int
dave_dll_thread_gid_msg(char *gid, char *thread_name, int msg_id, int msg_len, void *msg_body, char *fun, int line)
{
	DLLDEBUG("thread_name:%s msg_id:%d msg_len:%d msg_body:%lx fun:%s line:%d",
		thread_name, msg_id, msg_len, msg_body, fun, line);

	if(base_thread_gid_msg(
		(s8 *)gid, (s8 *)thread_name,
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

void *
dave_dll_thread_sync_msg(char *dst_thread, int req_id, int req_len, void *req_body, int rsp_id, int rsp_len, void *rsp_body, char *fun, int line)
{
	ThreadId src_id, dst_id;

	src_id = self();
	if(src_id == INVALID_THREAD_ID)
	{
		src_id = dave_dll_main_thread_id();
	}
	dst_id = thread_id(dst_thread);
	if(dst_id == INVALID_THREAD_ID)
	{
		DLLLOG("invalid dst_id on thread_name:%s!", dst_thread);
		return NULL;
	}

	return base_thread_sync_msg(src_id, dst_id, (ub)req_id, (ub)req_len, (u8 *)req_body, (ub)rsp_id, (ub)rsp_len, (u8 *)rsp_body, (s8 *)fun, (ub)line);
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

