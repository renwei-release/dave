/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_SERVER)
#include "dave_verno.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "sync_base_package.h"
#include "sync_param.h"
#include "sync_server_data.h"
#include "sync_server_msg_buffer.h"
#include "sync_server_remote_cfg.h"
#include "sync_test.h"
#include "sync_lock.h"
#include "sync_log.h"

static void
_sync_server_run_cfg_remote_update(SyncClient *pClient, CFGRemoteSyncUpdate *pUpdate)
{
	SYNCTRACE("%s %s ttl:%d",
		pClient->globally_identifier,
		ms8(pUpdate->cfg_mbuf_name),
		pUpdate->ttl);

	if(pUpdate->put_flag == dave_true)
	{
		sync_server_remote_cfg_set(pClient, pUpdate);
	}
	else
	{
		sync_server_remote_cfg_del(pClient, pUpdate);
	}

	dave_mfree(pUpdate->cfg_mbuf_name);
	dave_mfree(pUpdate->cfg_mbuf_value);
}

static void
_sync_server_run_client_busy(SyncClient *pClient, SystemBusy *pBusy)
{
	sync_server_client_state(pClient, dave_true);
}

static void
_sync_server_run_client_idle(SyncClient *pClient, SystemIdle *pIdle)
{
	sync_server_client_state(pClient, dave_false);
}

static dave_bool
_sync_server_run_special_internal(
	SyncClient *pClient,
	s8 *src, s8 *dst,
	ub msg_id,
	ub msg_len, u8 *msg_body)
{
	dave_bool ret = dave_true;

	switch(msg_id)
	{
		case MSGID_SYSTEM_BUSY:
				((SystemBusy *)(msg_body))->ptr = pClient;
			break;
		case MSGID_SYSTEM_IDLE:
				((SystemIdle *)(msg_body))->ptr = pClient;
			break;
		default:
				ret = dave_false;
			break;
	}

	return ret;
}

static dave_bool
_sync_server_run_internal(
	SyncClient *pClient,
	s8 *src, s8 *dst,
	ub msg_id,
	ub msg_len, u8 *msg_body)
{
	dave_bool process_flag = dave_true;

	switch(msg_id)
	{
		case MSGID_CFG_REMOTE_SYNC_UPDATE:
				_sync_server_run_cfg_remote_update(pClient, (CFGRemoteSyncUpdate *)(msg_body));
			break;
		case MSGID_THREAD_BUSY:
		case MSGID_THREAD_IDLE:
			break;
		case MSGID_SYSTEM_BUSY:
				_sync_server_run_client_busy(pClient, (SystemBusy *)(msg_body));
			break;
		case MSGID_SYSTEM_IDLE:
				_sync_server_run_client_idle(pClient, (SystemIdle *)(msg_body));
			break;
		default:
				process_flag = dave_false;
			break;
	}

	return process_flag;
}

static void
_sync_server_snd_internal(
	SyncClient *pClient,
	s8 *src, s8 *dst,
	ub msg_id,
	ub msg_len, u8 *msg_body)
{
	ThreadId src_thread, dst_thread;

	src_thread = thread_id(src);
	if(src_thread == INVALID_THREAD_ID)
	{
		src_thread = thread_id(SYNC_CLIENT_THREAD_NAME);
	}
	dst_thread = thread_id(dst);
	if(dst_thread == INVALID_THREAD_ID)
	{
		dst_thread = thread_id(SYNC_SERVER_THREAD_NAME);
	}

	SYNCTRACE("%s/%s->%s/%s:%d msg_len:%d",
		src, thread_name(src_thread),
		dst, thread_name(dst_thread),
		msg_id, msg_len);

	if(snd_from_msg(src_thread, dst_thread, msg_id, msg_len, msg_body) == dave_false)
	{
		SYNCABNOR("%s->%s msg_id:%d msg_len:%d failed!", src, dst, msg_id, msg_len);
	}
}

// =====================================================================

void
sync_server_run_init(void)
{

}

void
sync_server_run_exit(void)
{

}

void
sync_server_run_internal(
	SyncClient *pClient,
	s8 *src, s8 *dst,
	ub msg_id,
	ub msg_len, u8 *msg_body)
{
	_sync_server_run_special_internal(
			pClient,
			src, dst,
			msg_id,
			msg_len, msg_body);

	if(_sync_server_run_internal(
			pClient,
			src, dst,
			msg_id,
			msg_len, msg_body) == dave_false)
	{
		_sync_server_snd_internal(
				pClient,
				src, dst,
				msg_id,
				msg_len, msg_body);
	}
	else
	{
		thread_msg_release(msg_body);
	}
}

#endif

