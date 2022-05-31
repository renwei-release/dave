/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_PRODUCT_BASE__
#include "dave_base.h"
#include "dave_os.h"
#include "dave_tools.h"
#include "dave_verno.h"
#include "base_test.h"
#include "base_log.h"

#define RET_DEBUG_VALUE RetCode_OK
#define S8_DEBUG_VALUE -12
#define U8_DEBUG_VALUE 12
#define S16_DEBUG_VALUE -1234
#define U16_DEBUG_VALUE 1234
#define S32_DEBUG_VALUE -6462522
#define U32_DEBUG_VALUE 35554553
#define S64_DEBUG_VALUE -8376462522
#define U64_DEBUG_VALUE 83635554553
#define FLOAT_DEBUG_VALUE 12.34
#define DOUBLE_DEBUG_VALUE 123.123
#define VOID_DEBUG_VALUE (void *)739848572524

static ThreadId _base_thread = INVALID_THREAD_ID;

static void
_base_thread_rpc_debug_rsp(ThreadId src, RPCDebugRsp *pRsp)
{
	BASELOG("from:%s 8:%d/%d 16:%d/%d 32:%d/%d 64:%d/%d",
		thread_name(src),
		pRsp->s8_debug, pRsp->u8_debug,
		pRsp->s16_debug, pRsp->u16_debug,
		pRsp->s32_debug, pRsp->u32_debug,
		pRsp->s64_debug, pRsp->u64_debug);
}

static void
_base_thread_rpc_debug_req(ThreadId remote_thread_id)
{
	RPCDebugReq *pReq = thread_reset_msg(pReq);
	RPCDebugRsp rsp;

	pReq->ret_debug = RET_DEBUG_VALUE;
	pReq->s8_debug = S8_DEBUG_VALUE;
	pReq->u8_debug = U8_DEBUG_VALUE;
	pReq->s16_debug = S16_DEBUG_VALUE;
	pReq->u16_debug = U16_DEBUG_VALUE;
	pReq->s32_debug = S32_DEBUG_VALUE;
	pReq->u32_debug = U32_DEBUG_VALUE;
	pReq->s64_debug = S64_DEBUG_VALUE;
	pReq->u64_debug = U64_DEBUG_VALUE;
	pReq->float_debug = FLOAT_DEBUG_VALUE;
	pReq->double_debug = DOUBLE_DEBUG_VALUE;
	pReq->void_debug = VOID_DEBUG_VALUE;

	if(go(remote_thread_id, MSGID_RPC_DEBUG_REQ, pReq, MSGID_RPC_DEBUG_RSP, &rsp) != NULL)
	{
		BASELOG("Runs successfully!");
		_base_thread_rpc_debug_rsp(remote_thread_id, &rsp);
	}
}

static void
_base_thread_remote_id_ready(ThreadRemoteIDReadyMsg *pReady)
{
	BASELOG("%lx/%s/%s/%s",
		pReady->remote_thread_id, thread_name(pReady->remote_thread_id),
		pReady->remote_thread_name, pReady->globally_identifier);

	if(dave_strcmp(pReady->remote_thread_name, "main_aib") == dave_true)
	{
		_base_thread_rpc_debug_req(pReady->remote_thread_id);
	}
}

static void
_base_thread_remote_id_remove(ThreadRemoteIDRemoveMsg *pReady)
{
	BASELOG("%lx/%s/%s",
		pReady->remote_thread_id,
		pReady->remote_thread_name, pReady->globally_identifier);
}

static void
_base_thread_init(MSGBODY *msg)
{

}

static void
_base_thread_main(MSGBODY *msg)
{
	switch((sb)(msg->msg_id))
	{
		case MSGID_DEBUG_REQ:
				base_debug(msg->msg_src, (DebugReq *)(msg->msg_body));
			break;
		case MSGID_ECHO:
				base_echo(msg->msg_src, (MsgIdEcho *)(msg->msg_body));
			break;
		case MSGID_REMOTE_THREAD_ID_READY:
				_base_thread_remote_id_ready((ThreadRemoteIDReadyMsg *)(msg->msg_body));
			break;
		case MSGID_REMOTE_THREAD_ID_REMOVE:
				_base_thread_remote_id_remove((ThreadRemoteIDRemoveMsg *)(msg->msg_body));
			break;
		case MSGID_RPC_DEBUG_RSP:
				_base_thread_rpc_debug_rsp(msg->msg_src, (RPCDebugRsp *)(msg->msg_body));
			break;
		default:
			break;
	}
}

static void
_base_thread_exit(MSGBODY *msg)
{

}

// =====================================================================

void
dave_product_init(void)
{
	ub thread_number = dave_os_cpu_process_number();

	_base_thread = base_thread_creat(dave_verno_my_product(), thread_number, THREAD_THREAD_FLAG|THREAD_COROUTINE_FLAG, _base_thread_init, _base_thread_main, _base_thread_exit);
	if(_base_thread == INVALID_THREAD_ID)
		base_restart(dave_verno_my_product());
}

void
dave_product_exit(void)
{
	if(_base_thread != INVALID_THREAD_ID)
		base_thread_del(_base_thread);
	_base_thread = INVALID_THREAD_ID;
}

#endif

