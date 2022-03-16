/*
 * ================================================================================
 * (c) Copyright 2020 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.11.11.
 * The synchronization service in the future may try to connect to all within
 * the framework of the server.
 * ================================================================================
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_SERVER)
#include "dave_verno.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "sync_base_package.h"
#include "sync_param.h"
#include "sync_server_msg_buffer.h"
#include "sync_test.h"
#include "sync_lock.h"
#include "sync_log.h"

static void
_sync_server_run_internal(
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
		case MSGID_CLIENT_BUSY:
				((ClientBusy *)(msg_body))->ptr = pClient;
			break;
		case MSGID_CLIENT_IDLE:
				((ClientIdle *)(msg_body))->ptr = pClient;
			break;
		default:
				ret = dave_false;
			break;
	}

	return ret;
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

	_sync_server_run_internal(
			pClient,
			src, dst,
			msg_id,
			msg_len, msg_body);
}

#endif

