/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_tools.h"
#include "dave_base.h"
#include "dave_verno.h"
#include "thread_struct.h"
#include "thread_thread.h"
#include "thread_quit.h"
#include "thread_map.h"
#include "thread_mem.h"
#include "thread_guardian.h"
#include "thread_tools.h"
#include "thread_msg_buffer.h"
#include "thread_call.h"
#include "thread_sync.h"
#include "thread_statistics.h"
#include "thread_log.h"

static ThreadStruct *_thread = NULL;

static dave_bool
_thread_broadcast_thread_msg(BaseMsgType type, ThreadId self_id, s8 *dst_name, ub msg_id, ub msg_len, u8 *msg_body, s8 *fun, ub line)
{
	ThreadId dst_id;
	ub broadcast_len;
	u8 *broadcast_msg;
	dave_bool ret = dave_false;

	if((type != BaseMsgType_Broadcast_thread) || (dst_name == NULL))
	{
		return dave_false;
	}

	if((type == BaseMsgType_Broadcast_thread)
		&& (base_thread_attrib(self_id) == LOCAL_TASK_ATTRIB)
		&& (thread_must_in_local(thread_name(self_id)) == dave_false))
	{
		dst_id = thread_id(dst_name);

		broadcast_len = msg_len;
		broadcast_msg = base_thread_msg(broadcast_len, dave_false, (s8 *)__func__, (ub)__LINE__);
		dave_memcpy(broadcast_msg, msg_body, broadcast_len);

		if(dst_id != INVALID_THREAD_ID)
		{
			ret = base_thread_id_msg(NULL, NULL, NULL, NULL, self_id, dst_id, type, msg_id, broadcast_len, broadcast_msg, 0, fun, line);
		}
		else
		{
			ret = thread_msg_buffer_thread_push(self_id, dst_name, type, msg_id, broadcast_len, broadcast_msg, fun, line);
		}
	}

	THREADDEBUG("%s->%s:%d msg_len:%d ret:%d", thread_name(self_id), thread_name(dst_id), msg_id, msg_len, ret);

	return ret;
}

static dave_bool
_thread_broadcast_remote_msg(BaseMsgType type, ThreadId self_id, ub msg_id, ub msg_len, u8 *msg_body, s8 *fun, ub line)
{
	ThreadId syncc_id;
	ub broadcast_len;
	u8 *broadcast_msg;

	if((type != BaseMsgType_Broadcast_remote) && (type != BaseMsgType_Broadcast_total))
	{
		return dave_false;
	}

	syncc_id = thread_id(SYNC_CLIENT_THREAD_NAME);
	if(syncc_id == INVALID_THREAD_ID)
	{
		return dave_false;
	}

	if(((type == BaseMsgType_Broadcast_remote) || (type == BaseMsgType_Broadcast_total))
		&& (base_thread_attrib(self_id) == LOCAL_TASK_ATTRIB)
		&& (thread_must_in_local(thread_name(self_id)) == dave_false))
	{
		type = BaseMsgType_Broadcast_remote;

		broadcast_len = msg_len;
		broadcast_msg = base_thread_msg(msg_len, dave_false, (s8 *)__func__, (ub)__LINE__);
		dave_memcpy(broadcast_msg, msg_body, broadcast_len);

		base_thread_id_msg(NULL, NULL, NULL, NULL, self_id, syncc_id, type, msg_id, broadcast_len, broadcast_msg, 0, fun, line);
	}

	return dave_true;
}

static dave_bool
_thread_broadcast_local_msg(BaseMsgType type, ThreadId self_id, ub msg_id, ub msg_len, u8 *msg_body, s8 *fun, ub line)
{
	ub thread_index;
	ub broadcast_len;
	u8 *broadcast_msg;

	if((type != BaseMsgType_Broadcast_local) && (type != BaseMsgType_Broadcast_total))
	{
		return dave_false;
	}

	if((type == BaseMsgType_Broadcast_local) || (type == BaseMsgType_Broadcast_total))
	{
		for(thread_index=0; thread_index<THREAD_MAX; thread_index++)
		{
			if((_thread[thread_index].thread_id != INVALID_THREAD_ID)
				&& (_thread[thread_index].has_initialization == dave_true)
				&& (_thread[thread_index].thread_id != self_id)
				&& (base_thread_attrib(_thread[thread_index].thread_id) == LOCAL_TASK_ATTRIB))
			{
				broadcast_len = msg_len;
				broadcast_msg = base_thread_msg(broadcast_len, dave_false, (s8 *)__func__, (ub)__LINE__);
				dave_memcpy(broadcast_msg, msg_body, broadcast_len);

				THREADDEBUG("%s>%s:%s",
					thread_name(self_id), thread_name(_thread[thread_index].thread_id),
					msgstr(msg_id));

				/*
				 * Local broadcast message unified priority message queue processing.
				 */
				base_thread_id_msg(NULL, NULL, NULL, NULL, self_id, _thread[thread_index].thread_id, BaseMsgType_pre_msg, msg_id, broadcast_len, broadcast_msg, 0, fun, line);
			}
		}
	}

	return dave_true;
}

// =====================================================================

dave_bool
thread_broadcast_msg(
	ThreadStruct *thread_struct,
	BaseMsgType type,
	s8 *dst_name,
	ub msg_id,
	ub msg_len, u8 *msg_body,
	s8 *fun, ub line)
{
	ThreadId self_id = self();

	if(self_id == INVALID_THREAD_ID)
	{
		THREADDEBUG("who is me? type:%s msg_id:%s msg_len:%d <%s:%d>",
			t_auto_BaseMsgType_str(type),
			msgstr(msg_id), msg_len,
			fun, line);
	}

	_thread = thread_struct;

	if(msg_len == 0)
	{
		msg_len = 1;
	}

	if(msg_body == NULL)
	{
		msg_body = base_thread_msg(msg_len, dave_false, fun, line);
	}

	_thread_broadcast_thread_msg(type, self_id, dst_name, msg_id, msg_len, msg_body, fun, line);

	_thread_broadcast_remote_msg(type, self_id, msg_id, msg_len, msg_body, fun, line);

	_thread_broadcast_local_msg(type, self_id, msg_id, msg_len, msg_body, fun, line);

	THREADDEBUG("type:%d dst:%s msg_id:%d <%s:%d>", type, dst_name, msg_id, fun, line);

	thread_clean_user_input_data(msg_body, msg_id);

	return dave_true;
}

#endif

