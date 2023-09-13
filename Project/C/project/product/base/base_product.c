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
#include "dave_3rdparty.h"
#include "dave_bdata.h"
#include "dave_store.h"
#include "dave_echo.h"
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
	BASETRACE("from:%s str:%s/%s 8:%d/%d 16:%d/%d 32:%d/%d 64:%ld/%ld ptr:%lx",
		thread_name(src),
		pRsp->str_debug, pRsp->str_debug,
		pRsp->s8_debug, pRsp->u8_debug,
		pRsp->s16_debug, pRsp->u16_debug,
		pRsp->s32_debug, pRsp->u32_debug,
		pRsp->s64_debug, pRsp->u64_debug,
		pRsp->ptr);
}

static void
_base_thread_rpc_debug_req(ThreadId src, RPCDebugReq *pReq)
{
	RPCDebugRsp *pRsp = thread_msg(pRsp);

	*pRsp = (*(RPCDebugRsp *)(pReq));

	BASEDEBUG("from:%lx/%s 8:%d/%d 16:%d/%d 32:%d/%d 64:%ld/%ld ptr:%lx",
		src, thread_name(src),
		pRsp->s8_debug, pRsp->u8_debug,
		pRsp->s16_debug, pRsp->u16_debug,
		pRsp->s32_debug, pRsp->u32_debug,
		pRsp->s64_debug, pRsp->u64_debug,
		pRsp->ptr);

	dave_strcpy(pRsp->rsp_thread, thread_name(_base_thread), sizeof(pRsp->rsp_thread));

	id_msg(src, MSGID_RPC_DEBUG_RSP, pRsp);
}

static void
_base_thread_rpc_debug(ThreadRemoteIDReadyMsg *pReady)
{
	RPCDebugReq req;
	RPCDebugRsp *pRsp;

	dave_memset(&req, 0x00, sizeof(RPCDebugReq));

	req.ret_debug = RET_DEBUG_VALUE;
	dave_strcpy(req.req_thread, thread_name(_base_thread), sizeof(req.req_thread));
	dave_strcpy(req.str_debug, thread_name(pReady->remote_thread_id), sizeof(req.str_debug));
	req.s8_debug = S8_DEBUG_VALUE;
	req.u8_debug = U8_DEBUG_VALUE;
	req.s16_debug = S16_DEBUG_VALUE;
	req.u16_debug = U16_DEBUG_VALUE;
	req.s32_debug = S32_DEBUG_VALUE;
	req.u32_debug = U32_DEBUG_VALUE;
	req.s64_debug = S64_DEBUG_VALUE;
	req.u64_debug = U64_DEBUG_VALUE;
	req.float_debug = FLOAT_DEBUG_VALUE;
	req.double_debug = DOUBLE_DEBUG_VALUE;
	req.void_debug = VOID_DEBUG_VALUE;
	req.ptr = &req;

	pRsp = id_co(pReady->remote_thread_id, MSGID_RPC_DEBUG_REQ, &req, MSGID_RPC_DEBUG_RSP);
	if(pRsp != NULL)
	{
		BASELOG("id_co(%lx) successfully! ptr:%lx/%lx", pReady->remote_thread_id, &req, pRsp->ptr);
		_base_thread_rpc_debug_rsp(pReady->remote_thread_id, pRsp);
	}
	pRsp = name_co(pReady->remote_thread_name, MSGID_RPC_DEBUG_REQ, &req, MSGID_RPC_DEBUG_RSP);
	if(pRsp != NULL)
	{
		BASELOG("name_co(%s) successfully! ptr:%lx/%lx", pReady->remote_thread_name, &req, pRsp->ptr);
		_base_thread_rpc_debug_rsp(pReady->remote_thread_id, pRsp);
	}
	pRsp = gid_co(pReady->globally_identifier, pReady->remote_thread_name, MSGID_RPC_DEBUG_REQ, &req, MSGID_RPC_DEBUG_RSP);
	if(pRsp != NULL)
	{
		BASELOG("gid_co(%s/%s) successfully! ptr:%lx/%lx", pReady->globally_identifier, pReady->remote_thread_name, &req, pRsp->ptr);
		_base_thread_rpc_debug_rsp(pReady->remote_thread_id, pRsp);
	}
}

static void
_base_msg_timer_out(ProcessMsgTimerOutMsg *pTimerOut)
{
	BASELOG("msg_id:%s", msgstr(pTimerOut->msg_id));
}

static void
_base_thread_remote_id_ready(ThreadRemoteIDReadyMsg *pReady)
{
	BASEDEBUG("%lx/%s/%s/%s",
		pReady->remote_thread_id, thread_name(pReady->remote_thread_id),
		pReady->remote_thread_name, pReady->globally_identifier);

	if(dave_strcmp(pReady->remote_thread_name, "bbs") == dave_true)
	{
		rcfg_set("base_product_ttl_debug", "asdffffffffffffffffffffff", 60);
		rcfg_set("/set/base_product_ttl_debug", "asdffffffffffffffffffffff", 0);
		rcfg_set("/set/base_product_ttl_debug", "asdffffffffffffffffffffffmodify", 0);
		rcfg_set("base_product_ttl_debug", "debug", 60);
	}

	if((dave_strcmp(pReady->remote_thread_name, "bbs") == dave_true)
		|| (dave_strcmp(pReady->remote_thread_name, "main_aib") == dave_true))
	{
		_base_thread_rpc_debug(pReady);
	}
}

static void
_base_thread_remote_id_remove(ThreadRemoteIDRemoveMsg *pReady)
{
	BASEDEBUG("%lx/%s/%s",
		pReady->remote_thread_id,
		pReady->remote_thread_name, pReady->globally_identifier);
}

static void
_base_cfg_remote_update(CFGRemoteUpdate *pUpdate)
{
	BASETRACE("%s (%d)%s : (%d)%s ttl:%d",
		pUpdate->put_flag==dave_true?"PUT":"DELETE",
		dave_strlen(pUpdate->cfg_name), pUpdate->cfg_name,
		dave_strlen(pUpdate->cfg_value), pUpdate->cfg_value,
		pUpdate->ttl);
}

static void
_base_thread_init(MSGBODY *msg)
{
	BDATALOG("INIT", "%s booting!", dave_verno());
}

static void
_base_thread_main(MSGBODY *msg)
{
	switch((sb)(msg->msg_id))
	{
		case MSGID_DEBUG_REQ:
				base_debug(msg->msg_src, (DebugReq *)(msg->msg_body));
			break;
		case MSGID_ECHO_REQ:
		case MSGID_ECHO_RSP:
				dave_echo(msg->msg_src, msg->msg_dst, msg->msg_id, msg->msg_body);
			break;
		case MSGID_PROCESS_MSG_TIMER_OUT:
				_base_msg_timer_out((ProcessMsgTimerOutMsg *)(msg->msg_body));
			break;
		case MSGID_REMOTE_THREAD_ID_READY:
				_base_thread_remote_id_ready((ThreadRemoteIDReadyMsg *)(msg->msg_body));
			break;
		case MSGID_REMOTE_THREAD_ID_REMOVE:
				_base_thread_remote_id_remove((ThreadRemoteIDRemoveMsg *)(msg->msg_body));
			break;
		case MSGID_RPC_DEBUG_REQ:
				_base_thread_rpc_debug_req(msg->msg_src, (RPCDebugReq *)(msg->msg_body));
			break;
		case MSGID_RPC_DEBUG_RSP:
				_base_thread_rpc_debug_rsp(msg->msg_src, (RPCDebugRsp *)(msg->msg_body));
			break;
		case MSGID_CFG_REMOTE_UPDATE:
				_base_cfg_remote_update((CFGRemoteUpdate *)(msg->msg_body));
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

	_base_thread = base_thread_creat(t_gp_product_name(), thread_number, THREAD_THREAD_FLAG, _base_thread_init, _base_thread_main, _base_thread_exit);
	if(_base_thread == INVALID_THREAD_ID)
		base_restart(t_gp_product_name());
}

void
dave_product_exit(void)
{
	if(_base_thread != INVALID_THREAD_ID)
		base_thread_del(_base_thread);
	_base_thread = INVALID_THREAD_ID;
}

#endif

