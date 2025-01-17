/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_echo.h"
#include "dave_sip.h"
#include "dave_uac.h"
#include "dave_rtp.h"
#include "sip_signal.h"
#include "dave_osip.h"
#include "uac_call.h"
#include "uac_main.h"
#include "uac_log.h"

static ThreadId _uac_thread = INVALID_THREAD_ID;

static void
_uac_read(SocketRead *pRead)
{
	if(dave_rtp_recv(pRead) == dave_false)
	{
		sip_signal_recv(pRead);
	}

	dave_mfree(pRead->data);
}

static void
_uac_rtp(ThreadId src, RTPDataRsp *pRsp)
{
	uac_rtp(
		pRsp->call_id, pRsp->call_from, pRsp->call_to,
		pRsp->payload_type, pRsp->sequence_number, pRsp->timestamp, pRsp->ssrc,
		ms8(pRsp->payload_data), mlen(pRsp->payload_data));

	dave_mfree(pRsp->payload_data);
}

static void
_uac_call(ThreadId src, SIPCallReq *pReq)
{
	SIPCallRsp *pRsp = thread_msg(pRsp);

	UACLOG("%s phone_number:%s", thread_name(src), pReq->phone_number);

	pRsp->ret = uac_call(pRsp->call_id, sizeof(pRsp->call_id), src, pReq->phone_number);
	dave_strcpy(pRsp->phone_number, pReq->phone_number, sizeof(pRsp->phone_number));
	pRsp->ptr = pReq->ptr;

	id_msg(src, SIP_CALL_RSP, pRsp);
}

static void
_uac_bye(ThreadId src, SIPByeReq *pReq)
{
	SIPByeRsp *pRsp = thread_msg(pRsp);

	UACLOG("%s phone_number:%s", thread_name(src), pReq->phone_number);

	pRsp->ret = uac_bye(pRsp->call_id, sizeof(pRsp->call_id), src, pReq->phone_number);
	dave_strcpy(pRsp->phone_number, pReq->phone_number, sizeof(pRsp->phone_number));
	pRsp->ptr = pReq->ptr;

	id_msg(src, SIP_BYE_RSP, pRsp);
}

static void
_uac_all_bye(ThreadRemoteIDRemoveMsg *pRemove)
{
	UACLOG("%lx/%s/%s",
		pRemove->remote_thread_id, pRemove->remote_thread_name,
		pRemove->globally_identifier);

	uac_main_del_owner_id_all_call(pRemove->remote_thread_id);
}

static void
_uac_main(MSGBODY *pMsg)
{
	switch((sb)(pMsg->msg_id))
	{
		case MSGID_ECHO_REQ:
		case MSGID_ECHO_RSP:
				dave_echo(pMsg->msg_src, pMsg->msg_dst, pMsg->msg_id, pMsg->msg_body);
			break;
		case MSGID_RESTART_REQ:
		case MSGID_LOCAL_THREAD_READY:
		case MSGID_LOCAL_THREAD_REMOVE:
		case MSGID_REMOTE_THREAD_READY:
		case MSGID_REMOTE_THREAD_REMOVE:
		case MSGID_REMOTE_THREAD_ID_READY:
			break;
		case MSGID_REMOTE_THREAD_ID_REMOVE:
				_uac_all_bye((ThreadRemoteIDRemoveMsg *)(pMsg->msg_body));
			break;
		case SOCKET_DISCONNECT_RSP:
		case SOCKET_PLUGIN:
		case SOCKET_PLUGOUT:
		case MSGID_CFG_UPDATE:
			break;
		case SOCKET_READ:
				_uac_read((SocketRead *)(pMsg->msg_body));
			break;
		case RTP_DATA_RSP:
				_uac_rtp(pMsg->msg_src, (RTPDataRsp *)(pMsg->msg_body));
			break;
		case SIP_CALL_REQ:
				_uac_call(pMsg->msg_src, (SIPCallReq *)(pMsg->msg_body));
			break;
		case SIP_BYE_REQ:
				_uac_bye(pMsg->msg_src, (SIPByeReq *)(pMsg->msg_body));
			break;
		default:
				UACLOG("unprocess msg-id:%s", msgstr(pMsg->msg_id));
			break;
	}
}

static void
_uac_init(MSGBODY *pMsg)
{
	dave_osip_init();
	sip_signal_init();
	dave_rtp_init();

	uac_main_init();
}

static void
_uac_exit(MSGBODY *pMsg)
{
	uac_main_exit();

	dave_rtp_exit();
	sip_signal_exit();
	dave_osip_exit();
}

// =====================================================================

void
dave_uac_init(void)
{
	ub thread_number = dave_os_cpu_process_number();

	_uac_thread = base_thread_creat(
		UAC_THREAD_NAME, thread_number, THREAD_THREAD_FLAG,
		_uac_init, _uac_main, _uac_exit);

	if(_uac_thread == INVALID_THREAD_ID)
		base_restart(UAC_THREAD_NAME);
}

void
dave_uac_exit(void)
{
	if(_uac_thread != INVALID_THREAD_ID)
		base_thread_del(_uac_thread);
	_uac_thread = INVALID_THREAD_ID;
}

