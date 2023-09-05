/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_verno.h"
#include "dos_show.h"
#include "dos_cmd.h"
#include "dos_tools.h"
#include "dos_log.h"

static void
_dos_queue_info_rsp(MSGBODY *ptr)
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
_dos_queue_info_req(s8 *cmd_ptr, ub cmd_len)
{
	dave_bool is_queue_server = dave_strcmp(dave_verno_my_product(), "QUEUE");
	s8 *queue_server = QUEUE_SERVER_THREAD_NAME;
	DebugReq *pReq;

	if(is_queue_server == dave_true)
		queue_server = QUEUE_SERVER_THREAD_NAME;
	else
		queue_server = QUEUE_CLIENT_THREAD_NAME;

	pReq = thread_reset_msg(pReq);

	pReq->msg[0] = 'i';

	name_event(queue_server, MSGID_DEBUG_REQ, pReq, MSGID_DEBUG_RSP, _dos_queue_info_rsp);

	return RetCode_OK;
}

// =====================================================================

void
dos_queue_reset(void)
{
	dos_cmd_reg("queue", _dos_queue_info_req, NULL);
}

#endif

