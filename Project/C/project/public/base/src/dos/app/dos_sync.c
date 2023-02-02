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
#include "dave_verno.h"
#include "dos_show.h"
#include "dos_cmd.h"
#include "dos_tools.h"
#include "dos_log.h"

static BuildingBlocksOpt
_dos_sync_opt_str_to_opt(s8 *opt_str)
{
	if(dave_strcmp("i", opt_str) == dave_true)
	{
		return BuildingBlocksOpt_inq;
	}
	if(dave_strcmp("mount", opt_str) == dave_true)
	{
		return BuildingBlocksOpt_mount;
	}
	else if(dave_strcmp("decoupling", opt_str) == dave_true)
	{
		return BuildingBlocksOpt_decoupling;
	}
	else if(dave_strcmp("exchange", opt_str) == dave_true)
	{
		return BuildingBlocksOpt_State_exchange;
	}
	else if(dave_strcmp("valve", opt_str) == dave_true)
	{
		return BuildingBlocksOpt_valve;
	}
	else
	{
		return BuildingBlocksOpt_none;
	}
}

static RetCode
_dos_sync_blocks_help(void)
{
	dos_print("Please enter the command in the following format:\n"\
			"blocks i\n"\
			"blocks mount [blocks id]\n"\
			"blocks decoupling [blocks id]\n"\
			"blocks exchange [blocks id 1] [blocks id 2]\n"\
			"blocks valve [blocks id] [release quantity]");

	return RetCode_OK;
}

static void
_dos_sync_blocks_rsp(MSGBODY *msg)
{
	MsgBlocksRsp *pRsp = (MsgBlocksRsp *)(msg->msg_body);
	s8 *print_buf;
	ub print_buf_len, print_buf_index;
	ub blocks_index;

	print_buf_len = 1024 * 64;
	print_buf_index = 0;
	print_buf = dave_malloc(print_buf_len);

	if(pRsp->ret != RetCode_OK)
	{
		print_buf_index += dave_snprintf(&print_buf[print_buf_index], print_buf_len-print_buf_index,
			"blocks option error:%s\n", retstr(pRsp->ret));
	}

	for(blocks_index=0; blocks_index<DAVE_BUILDING_BLOCKS_MAX; blocks_index++)
	{
		if(pRsp->blocks[blocks_index].blocks_id != 0)
		{
			print_buf_index += dave_snprintf(&print_buf[print_buf_index], print_buf_len-print_buf_index,
				"BLOCKS:%d verno:%s ip:%s state:%s/%s/%s quantity:%d\n",
				pRsp->blocks[blocks_index].blocks_id,
				pRsp->blocks[blocks_index].verno,
				ipv4str(pRsp->blocks[blocks_index].ip_addr, pRsp->blocks[blocks_index].port),
				pRsp->blocks[blocks_index].ready_flag==dave_true?"ready":"not ready",
				pRsp->blocks[blocks_index].blocks_flag==dave_true?"mount":"decoupling",
				pRsp->blocks[blocks_index].client_flag==dave_true?"idle":"busy",
				pRsp->blocks[blocks_index].release_quantity);
		}
	}

	if(print_buf_index == 0)
	{
		dos_print("[EMPTY]");
	}
	else
	{
		dos_print("%s", print_buf);
	}

	dave_free(print_buf);
}

static RetCode
_dos_sync_blocks_req(s8 *cmd_ptr, ub cmd_len)
{
	ub cmd_index;
	s8 opt_str[64];
	BuildingBlocksOpt opt;
	ub blocks_id_1, blocks_id_2;
	MsgBlocksReq *pReq;
	RetCode ret = RetCode_Invalid_parameter;

	cmd_index = 0;
	cmd_index += dos_get_one_parameters(&cmd_ptr[cmd_index], cmd_len-cmd_index, opt_str, sizeof(opt_str));
	opt = _dos_sync_opt_str_to_opt(opt_str);
	cmd_index += dos_load_ub(&cmd_ptr[cmd_index], cmd_len-cmd_index, &blocks_id_1);
	cmd_index += dos_load_ub(&cmd_ptr[cmd_index], cmd_len-cmd_index, &blocks_id_2);

	if(opt != BuildingBlocksOpt_none)
	{
		pReq = thread_msg(pReq);

		pReq->opt = opt;
		pReq->blocks_id_1 = blocks_id_1;
		pReq->blocks_id_2 = blocks_id_2;
		pReq->ptr = NULL;

		id_event(thread_id(SYNC_SERVER_THREAD_NAME), MSGID_BLOCKS_REQ, pReq, MSGID_BLOCKS_RSP, _dos_sync_blocks_rsp);

		ret = RetCode_OK;
	}

	return ret;
}

static void
_dos_sync_info_rsp(MSGBODY *ptr)
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
_dos_sync_info_req(s8 *cmd_ptr, ub cmd_len)
{
	dave_bool is_sync_server = dave_strcmp(dave_verno_product(NULL, NULL, 0), "SYNC");
	s8 *sync_server = SYNC_SERVER_THREAD_NAME;
	DebugReq *pReq;

	if(is_sync_server == dave_true)
		sync_server = SYNC_SERVER_THREAD_NAME;
	else
		sync_server = SYNC_CLIENT_THREAD_NAME;

	pReq = thread_reset_msg(pReq);

	pReq->msg[0] = 'i';

	name_event(sync_server, MSGID_DEBUG_REQ, pReq, MSGID_DEBUG_RSP, _dos_sync_info_rsp);

	return RetCode_OK;
}

// =====================================================================

void
dos_sync_reset(void)
{
	dave_bool is_sync_server = dave_strcmp(dave_verno_product(NULL, NULL, 0), "SYNC");

	if(is_sync_server == dave_true)
	{
		dos_cmd_reg("blocks", _dos_sync_blocks_req, _dos_sync_blocks_help);
	}

	dos_cmd_reg("sync", _dos_sync_info_req, NULL);
}

#endif

