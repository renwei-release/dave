/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_SERVER)
#include "dave_tools.h"
#include "dave_verno.h"
#include "dave_base.h"
#include "sync_param.h"
#include "sync_base_package.h"
#include "sync_server_param.h"
#include "sync_server_data.h"
#include "sync_server_tx.h"
#include "sync_server_broadcadt.h"
#include "sync_server_broadcadt_tx.h"
#include "sync_server_tools.h"
#include "sync_lock.h"
#include "sync_test.h"
#include "sync_log.h"

static TLock _broadcadt_pv;

static RetCode
_sync_server_broadcadt_to_thread_self_client(
	SyncClient *pSrcClient,
	ThreadId src_id, ThreadId dst_id,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	s8 *src_name, s8 *dst_name,
	ub msg_id,
	BaseMsgType msg_type,
	u8 *msg_body, ub msg_len)
{
	SyncThread *pSrcThread, *pDstThread;

	pSrcThread = sync_server_find_thread(src_name);
	if(pSrcThread == NULL)
	{
		SYNCLOG("can't find src! %s->%s:%d", src_name, dst_name, msg_id);
		return RetCode_can_not_find_thread;
	}
	pDstThread = sync_server_find_thread(dst_name);
	if(pDstThread == NULL)
	{
		SYNCLOG("can't find dst! %s->%s:%d", src_name, dst_name, msg_id);
		return RetCode_can_not_find_thread;
	}

	sync_server_broadcadt_tx_the_msg_to_thread_self_client(
		pSrcClient,
		pSrcThread, pDstThread,
		src_id, dst_id,
		src_attrib, dst_attrib,
		src_name, dst_name,
		msg_id,
		msg_type,
		msg_body, msg_len);

	SYNCTRACE("%s->%s:%s", src_name, dst_name, msgstr(msg_id));

	return RetCode_OK;
}

static RetCode
_sync_server_broadcadt_to_all_client(
	SyncClient *pSrcClient,
	ThreadId route_src, s8 *src,
	ThreadId route_dst, s8 *dst,
	ub msg_id,
	BaseMsgType msg_type,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	u8 *msg_body, ub msg_len)
{
	return sync_server_broadcadt_tx_the_msg_to_all_client(
		pSrcClient,
		route_src, src_attrib, src,
		msg_id,
		msg_type,
		msg_body, msg_len);
}

static RetCode
_sync_server_broadcadt(
	SyncClient *pSrcClient,
	ThreadId route_src, s8 *src,
	ThreadId route_dst, s8 *dst,
	ub msg_id,
	BaseMsgType msg_type,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	u8 *msg_body, ub msg_len)
{
	RetCode ret = RetCode_OK;

	switch(msg_type)
	{
		case BaseMsgType_Broadcast_thread:
				ret = _sync_server_broadcadt_to_thread_self_client(
					pSrcClient,
					route_src, route_dst,
					src_attrib, dst_attrib,
					src, dst,
					msg_id, msg_type,
					msg_body, msg_len);
			break;
		case BaseMsgType_Broadcast_dismiss:
			break;
		case BaseMsgType_Broadcast_remote:
		case BaseMsgType_Broadcast_total:
				ret = _sync_server_broadcadt_to_all_client(
					pSrcClient,
					route_src, src,
					route_dst, dst,
					msg_id,
					msg_type,
					src_attrib, dst_attrib,
					msg_body, msg_len);
			break;
		default:
				SYNCLOG("invalid msg_type:%d", msg_type);
			break;
	}

	return ret;
}

// =====================================================================

void
sync_server_broadcadt_init(void)
{
	t_lock_reset(&_broadcadt_pv);
}

void
sync_server_broadcadt_exit(void)
{

}

RetCode
sync_server_broadcadt(
	SyncClient *pSrcClient,
	ThreadId route_src, s8 *src,
	ThreadId route_dst, s8 *dst,
	ub msg_id,
	BaseMsgType msg_type,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	u8 *msg_body, ub msg_len)
{
	RetCode ret = RetCode_Resource_conflicts;

	SYNCTRACE("%s->%s:%d", src, dst, msg_id);

	SAFECODEv2W(_broadcadt_pv, ret = _sync_server_broadcadt(
			pSrcClient,
			route_src, src,
			route_dst, dst,
			msg_id,
			msg_type,
			src_attrib, dst_attrib,
			msg_body, msg_len			
		);
	);

	return ret;
}

#endif

