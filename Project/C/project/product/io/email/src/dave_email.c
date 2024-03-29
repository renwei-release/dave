/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_email.h"
#include "dave_os.h"
#include "dave_echo.h"
#include "dave_3rdparty.h"
#include "email_send.h"
#include "email_log.h"

#define EMAIL_THREAD_MAX 16

static ThreadId _email_thread = INVALID_THREAD_ID;

static ub
_email_thread_number(void)
{
	ub thread_number = dave_os_cpu_process_number();

	if(thread_number > EMAIL_THREAD_MAX)
	{
		thread_number = EMAIL_THREAD_MAX;
	}

	return thread_number;
}

static void
_email_send(ThreadId src, EmailSendReq *pReq)
{
	EmailSendRsp *pRsp = thread_msg(pRsp);

	pRsp->ret = email_send(pReq->subject, ms8(pReq->content));
	pRsp->ptr = pReq->ptr;

	id_msg(src, EMAIL_SEND_RSP, pRsp);

	dave_mfree(pReq->content);
}

static void
_email_init(MSGBODY *msg)
{

}

static void
_email_main(MSGBODY *msg)
{
	switch((ub)msg->msg_id)
	{
		case MSGID_ECHO_REQ:
		case MSGID_ECHO_RSP:
				dave_echo(msg->msg_src, msg->msg_dst, msg->msg_id, msg->msg_body);
			break;
		case EMAIL_SEND_REQ:
				_email_send(msg->msg_src, (EmailSendReq *)(msg->msg_body));
			break;
		default:
			break;
	}
}

static void
_email_exit(MSGBODY *msg)
{

}

// =====================================================================

void
dave_email_init(void)
{
	ub thread_number = _email_thread_number();

	_email_thread = base_thread_creat(EMAIL_THREAD_NAME, thread_number, THREAD_THREAD_FLAG|THREAD_dCOROUTINE_FLAG, _email_init, _email_main, _email_exit);
	if(_email_thread == INVALID_THREAD_ID)
		base_restart(EMAIL_THREAD_NAME);
}

void
dave_email_exit(void)
{
	if(_email_thread != INVALID_THREAD_ID)
		base_thread_del(_email_thread);
	_email_thread = INVALID_THREAD_ID;
}

