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
#include "dos_show.h"
#include "dos_cmd.h"
#include "dos_tools.h"
#include "dos_log.h"

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

static ErrCode
_dos_debug_req(s8 *cmd_ptr, ub cmd_len)
{
	ub cmd_index;
	s8 thread_name[128];
	DebugReq *pReq;
	ThreadId debug_thread;

	pReq = thread_reset_msg(pReq);

	cmd_index = 0;

	cmd_index += dos_get_str(&cmd_ptr[cmd_index], cmd_len-cmd_index, thread_name, sizeof(thread_name));
	dos_get_last_parameters(&cmd_ptr[cmd_index], cmd_len-cmd_index, pReq->msg, sizeof(pReq->msg));

	debug_thread = thread_id(thread_name);
	if(debug_thread == INVALID_THREAD_ID)
	{
		if(thread_name[0] != '\0')
		{
			dos_print("invalid thread:%s", thread_name);
			thread_msg_release(pReq);
			return ERRCODE_can_not_find_thread;
		}
		else
		{
			dave_strcpy(thread_name, GUARDIAN_THREAD_NAME, sizeof(thread_name));
		}
	}

	remote_event(thread_name, MSGID_DEBUG_REQ, pReq, MSGID_DEBUG_RSP, _dos_debug_rsp);

	return ERRCODE_OK;
}

// =====================================================================

void
dos_debug_reset(void)
{
	dos_cmd_register("debug", _dos_debug_req, NULL);
}

#endif

