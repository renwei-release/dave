/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_base.h"
#include "dave_tools.h"
#include "base_tools.h"
#include "dos_show.h"
#include "dos_cmd.h"
#include "dos_tools.h"
#include "dos_log.h"

static ThreadId
_dos_debug_loose_match(s8 *thread_name)
{
	s8 thread_detect_name[128];
	ThreadId debug_thread;

	dave_strcpy(thread_detect_name, thread_name, sizeof(thread_detect_name));

	debug_thread = thread_id(thread_detect_name);
	if(debug_thread == INVALID_THREAD_ID)
	{
		debug_thread = thread_id(lower(thread_detect_name));
	}
	if(debug_thread == INVALID_THREAD_ID)
	{
		debug_thread = thread_id(upper(thread_detect_name));
	}

	return debug_thread;
}

static void
_dos_debug_rsp(MSGBODY *ptr)
{
	DebugRsp *pRsp = (DebugRsp *)(ptr->msg_body);

	if(dave_strlen(pRsp->msg) == 0)
	{
		dos_print("the empty message from %s!", thread_name(ptr->msg_src));
	}
	else
	{
		dos_print("%s", pRsp->msg);
	}
}

static RetCode
_dos_debug_req(s8 *cmd_ptr, ub cmd_len)
{
	ub cmd_index;
	s8 thread_name[128];
	DebugReq *pReq;
	ThreadId debug_thread;

	pReq = thread_reset_msg(pReq);

	cmd_index = 0;

	cmd_index += dos_load_string(&cmd_ptr[cmd_index], cmd_len-cmd_index, thread_name, sizeof(thread_name));
	dos_get_last_parameters(&cmd_ptr[cmd_index], cmd_len-cmd_index, pReq->msg, sizeof(pReq->msg));

	debug_thread = _dos_debug_loose_match(thread_name);
	if(debug_thread == INVALID_THREAD_ID)
	{
		if(thread_name[0] != '\0')
		{
			dos_print("invalid thread:%s", thread_name);
			thread_msg_release(pReq);
			return RetCode_can_not_find_thread;
		}

		name_event(GUARDIAN_THREAD_NAME, MSGID_DEBUG_REQ, pReq, MSGID_DEBUG_RSP, _dos_debug_rsp);
	}
	else
	{
		id_event(debug_thread, MSGID_DEBUG_REQ, pReq, MSGID_DEBUG_RSP, _dos_debug_rsp);
	}

	return RetCode_OK;
}

static RetCode
_dos_main_req(s8 *cmd_ptr, ub cmd_len)
{
	DebugReq *pReq;
	ThreadId main_thread;

	main_thread = main_thread_id_get();
	if(main_thread == INVALID_THREAD_ID)
	{
		return RetCode_can_not_find_thread;		
	}

	pReq = thread_reset_msg(pReq);

	dos_get_last_parameters(cmd_ptr, cmd_len, pReq->msg, sizeof(pReq->msg));

	id_event(main_thread, MSGID_DEBUG_REQ, pReq, MSGID_DEBUG_RSP, _dos_debug_rsp);

	return RetCode_OK;
}

// =====================================================================

void
dos_debug_reset(void)
{
	dos_cmd_reg("debug", _dos_debug_req, NULL);
	dos_cmd_reg("m", _dos_main_req, NULL);
}

#endif

