/*
 * ================================================================================
 * (c) Copyright 2020 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.08.10.
 * The synchronization service in the future may try to connect to all within
 * the framework of the server.
 * ================================================================================
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_SERVER)
#include "dave_tools.h"
#include "dave_verno.h"
#include "dave_base.h"
#include "sync_param.h"
#include "sync_tools.h"
#include "sync_base_package.h"
#include "sync_server_param.h"
#include "sync_server_tx.h"
#include "sync_server_rx.h"
#include "sync_server_data.h"
#include "sync_server_tools.h"
#include "sync_server_sync.h"
#include "sync_lock.h"
#include "sync_test.h"
#include "sync_log.h"

static ub
_sync_server_client_index_to_blocks_id(ub client_index)
{
	return client_index + 1;
}

static ub
_sync_server_blocks_id_to_client_index(ub blocks_id)
{
	ub client_index;

	if(blocks_id == 0)
	{
		return 0;
	}

	client_index = blocks_id - 1;

	if(client_index >= SYNC_CLIENT_MAX)
	{
		client_index = 0;
	}

	return client_index;
}

static ErrCode
_sync_server_opt_blocks_mount_or_decoupling(BuildingBlocksOpt opt, ub blocks_id)
{
	ErrCode ret = ERRCODE_invalid_option;
	ub client_index;
	SyncClient *pClient = NULL;

	if((blocks_id > 0) && (blocks_id <= SYNC_CLIENT_MAX))
	{
		client_index = _sync_server_blocks_id_to_client_index(blocks_id);
		pClient = sync_server_client(client_index);

		if(pClient->client_socket != INVALID_SOCKET_ID)
		{
			if(opt == BuildingBlocksOpt_mount)
			{
				pClient->blocks_flag = dave_true;
				pClient->release_quantity = 0;
				ret = ERRCODE_OK;
			}
			else if(opt == BuildingBlocksOpt_decoupling)
			{
				if(sync_server_still_have_ready_brothers(pClient) == dave_true)
				{
					pClient->blocks_flag = dave_false;
					pClient->release_quantity = 0;
					ret = ERRCODE_OK;
				}
			}
		}

		SYNCLOG("%s blocks:%d", pClient->verno, pClient->blocks_flag);
	}

	if(ret == ERRCODE_OK)
	{
		sync_server_tx_blocks_state(pClient);

		sync_server_sync_auto_link(pClient);
	}

	return ret;
}

static ErrCode
_sync_server_opt_blocks_exchange(ub blocks_id_1, ub blocks_id_2)
{
	ub client1_index, client2_index;
	SyncClient *pClient1 = NULL;
	SyncClient *pClient2 = NULL;
	dave_bool back_flag;
	ErrCode ret = ERRCODE_invalid_option;

	if((blocks_id_1 > 0) && (blocks_id_1 <= SYNC_CLIENT_MAX)
		&& (blocks_id_2 > 0) && (blocks_id_2 <= SYNC_CLIENT_MAX))
	{
		client1_index = _sync_server_blocks_id_to_client_index(blocks_id_1);
		client2_index = _sync_server_blocks_id_to_client_index(blocks_id_2);

		pClient1 = sync_server_client(client1_index);
		pClient2 = sync_server_client(client2_index);
	
		if((pClient1->client_socket != INVALID_SOCKET_ID)
			&& (pClient2->client_socket != INVALID_SOCKET_ID)
			&& (sync_server_are_they_brothers(pClient1, pClient2) == dave_true)
			&& (pClient1->ready_flag == dave_true)
			&& (pClient2->ready_flag == dave_true)
			&& (pClient1->blocks_flag != pClient2->blocks_flag))
		{
			back_flag = pClient1->blocks_flag;
			pClient1->blocks_flag = pClient2->blocks_flag;
			pClient2->blocks_flag = back_flag;
			SYNCLOG("%s blocks:%d, %s blocks:%d",
				pClient1->verno, pClient1->blocks_flag,
				pClient2->verno, pClient2->blocks_flag);
			ret = ERRCODE_OK;
		}
	}

	if(ret == ERRCODE_OK)
	{
		sync_server_tx_blocks_state(pClient1);
		sync_server_tx_blocks_state(pClient2);

		sync_server_sync_auto_link(pClient1);
		sync_server_sync_auto_link(pClient2);
	}

	return ret;
}

static ErrCode
_sync_server_opt_blocks_valve(ub blocks_id, ub release_quantity)
{
	ErrCode ret = ERRCODE_invalid_option;
	ub client_index;
	SyncClient *pClient = NULL;

	if((blocks_id > 0) && (blocks_id <= SYNC_CLIENT_MAX))
	{
		client_index = _sync_server_blocks_id_to_client_index(blocks_id);
		pClient = sync_server_client(client_index);

		if(pClient->client_socket != INVALID_SOCKET_ID)
		{
			if((pClient->ready_flag == dave_true) && (pClient->blocks_flag == dave_false))
			{
				if((pClient->release_quantity + release_quantity) <= SYNC_MAX_RELEASE_QUANTITY)
				{
					sync_lock();
					pClient->release_quantity += release_quantity;
					sync_unlock();
					ret = ERRCODE_OK;
				}
				else
				{
					SYNCLOG("the release quantity:%d/%d/%d is overflow!",
						pClient->release_quantity, release_quantity,
						SYNC_MAX_RELEASE_QUANTITY);
					ret = ERRCODE_data_overflow;
				}
			}
		}
	}

	return ret;
}

static ErrCode
_sync_server_opt_blocks(BuildingBlocksOpt opt, ub blocks_id_1, ub blocks_id_2)
{
	ErrCode ret = ERRCODE_invalid_option;

	if(opt == BuildingBlocksOpt_inq)
	{
		ret = ERRCODE_OK;
	}
	else if((opt == BuildingBlocksOpt_mount) || (opt == BuildingBlocksOpt_decoupling))
	{
		ret = _sync_server_opt_blocks_mount_or_decoupling(opt, blocks_id_1);
	}
	else if(opt == BuildingBlocksOpt_State_exchange)
	{
		ret = _sync_server_opt_blocks_exchange(blocks_id_1, blocks_id_2);
	}
	else if(opt == BuildingBlocksOpt_valve)
	{
		ret = _sync_server_opt_blocks_valve(blocks_id_1, blocks_id_2);
	}

	return ret;
}

static void
_sync_server_load_blocks(BuildingBlocks *pBlocks)
{
	ub client_index, blocks_index;
	SyncClient *pClient;

	for(client_index=0,blocks_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		pClient = sync_server_client(client_index);

		if(pClient->client_socket != INVALID_SOCKET_ID)
		{
			pBlocks[blocks_index].blocks_id = _sync_server_client_index_to_blocks_id(pClient->client_index);
			dave_strcpy(pBlocks[blocks_index].verno, pClient->verno, DAVE_VERNO_STR_LEN);
			dave_memcpy(pBlocks[blocks_index].ip_addr, pClient->NetInfo.addr.ip.ip_addr, sizeof(pClient->NetInfo.addr.ip.ip_addr));
			pBlocks[blocks_index].port = pClient->NetInfo.port;
			pBlocks[blocks_index].ready_flag = pClient->ready_flag;
			pBlocks[blocks_index].blocks_flag = pClient->blocks_flag;
			pBlocks[blocks_index].client_flag = pClient->client_flag;
			pBlocks[blocks_index].release_quantity = pClient->release_quantity;

			blocks_index ++;
		}
	}
}

static void
_sync_server_blocks_req(ThreadId src, MsgBlocksReq *pReq)
{
	MsgBlocksRsp *pRsp = thread_reset_msg(pRsp);

	pRsp->ret = _sync_server_opt_blocks(pReq->opt, pReq->blocks_id_1, pReq->blocks_id_2);
	pRsp->opt = pReq->opt;
	_sync_server_load_blocks(pRsp->blocks);
	pRsp->ptr = pReq->ptr;

	write_msg(src, MSGID_BLOCKS_RSP, pRsp);
}

// =====================================================================

void
sync_server_blocks_command(ThreadId src, MsgBlocksReq *pReq)
{
	_sync_server_blocks_req(src, pReq);
}

ub
sync_server_blocks_index_to_blocks_id(ub client_index)
{
	return _sync_server_client_index_to_blocks_id(client_index);
}

#endif

