/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_CLIENT)
#include "dave_verno.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "thread_tools.h"
#include "sync_base_package.h"
#include "sync_param.h"
#include "sync_client_tools.h"
#include "sync_client_link.h"
#include "sync_client_data.h"
#include "sync_client_run.h"
#include "sync_client_tx.h"
#include "sync_client_msg_buffer.h"
#include "sync_test.h"
#include "sync_lock.h"
#include "sync_log.h"

// #define SYNC_RUN_THREAD_ON_EVENTS

typedef struct {
	SyncServer *pServer;
	MBUF *frame_mbuf;
} SyncClientRunThreadEvents;

static ThreadId _sync_client_thread = INVALID_THREAD_ID;

static inline dave_bool
_sync_client_run_thread_msg(
	SyncServer *pServer,
	s8 *src, ThreadId route_src,
	s8 *dst, ThreadId route_dst,
	ThreadId src_thread, ThreadId dst_thread,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	ub msg_id,
	ub msg_len, u8 *msg_body)
{
	ub net_index;

	route_src = thread_set_local(route_src, src_thread);
	route_dst = thread_set_local(route_dst, dst_thread);

	if(thread_is_sync(route_src) == dave_true)
	{
		/*
		 * 来自SYNC服务器的消息，参数沿用SYNC服务器本地值。
		 */
		route_src = thread_set_remote(0, route_src, thread_get_thread(route_src), thread_get_net(route_src));
		route_src = thread_set_sync(route_src);
		route_dst = thread_set_remote(0, route_dst, thread_get_thread(route_dst), thread_get_net(route_dst));
		route_dst = thread_set_sync(route_dst);
	}
	else
	{
		if(pServer->shadow_index >= SERVER_DATA_MAX)
		{
			net_index = pServer->server_index;
			SYNCLTRACE(60, 1,
				"The shadow index should be used at this time, but the shadow index has not been established! %s->%s:%d",
				src, dst, msg_id);
		}
		else
		{
			net_index = pServer->shadow_index;
		}

		/*
		 * 来自LINK链路的消息，参数按本地实际值计算。
		 */
		route_src = thread_set_remote(0, route_src, sync_client_data_thread_index_on_name(src), net_index);
		route_dst = thread_set_remote(0, route_dst, INVALID_THREAD_ID, net_index);
	}

	SYNCDEBUG("%s/%lx->%s/%lx:%d", src, route_src, dst, route_dst, msg_id);

	return snd_from_msg(route_src, route_dst, msg_id, msg_len, msg_body);
}

static inline dave_bool
_sync_client_run_thread(
	SyncServer *pServer,
	s8 *src, ThreadId route_src,
	s8 *dst, ThreadId route_dst,
	ub msg_id,
	BaseMsgType msg_type,
	ub msg_len, void *msg_body,
	dave_bool buffer_pop)
{
	ThreadId src_thread, dst_thread;
	TaskAttribute src_attrib, dst_attrib;
	dave_bool ret = dave_false;

	src_thread = thread_id(src);
	dst_thread = thread_id(dst);

	/*
	 * 如果影子索引没准备好，
	 * 或者本地线程还未就绪，
	 * 或者是广播到线程的消息，为了稍微做些延迟，
	 * 需要用sync_client_msg_buffer_push到来缓存后发送。
	 */
	if((pServer->server_ready == dave_false)
		|| (pServer->shadow_index >= SERVER_DATA_MAX)
		|| (src_thread == INVALID_THREAD_ID)
		|| (dst_thread == INVALID_THREAD_ID))
	{
		if(buffer_pop == dave_false)
		{
			sync_client_msg_buffer_push(
				pServer,
				src, route_src,
				dst, route_dst,
				msg_id, msg_type,
				msg_len, msg_body,
				_sync_client_run_thread);
			return dave_true;
		}
		return dave_false;
	}

	src_attrib = thread_attrib(src_thread);
	dst_attrib = thread_attrib(dst_thread);

	if(dst_attrib == LOCAL_TASK_ATTRIB)
	{
		ret = _sync_client_run_thread_msg(
			pServer,
			src, route_src,
			dst, route_dst,
			src_thread, dst_thread,
			src_attrib, dst_attrib,
			msg_id,
			msg_len, msg_body);
	}
	else
	{
		SYNCABNOR("get %s invalid attrib:%d->%d or thread:%s<%d>->%s<%d>:%d buffer_pop:%d",
			pServer->verno, src_attrib, dst_attrib, src, src_thread, dst, dst_thread, msg_id,
			buffer_pop);

		ret = dave_false;
	}

	return ret;
}

static void
_sync_client_run_internal(
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
		dst_thread = thread_id(SYNC_CLIENT_THREAD_NAME);
	}

	SYNCTRACE("%s->%s:%d msg_len:%d", src, dst, msg_id, msg_len);

	if(snd_from_msg(src_thread, dst_thread, msg_id, msg_len, msg_body) == dave_false)
	{
		SYNCABNOR("%s->%s msg_id:%d msg_len:%d failed!", src, dst, msg_id, msg_len);
	}
}

static inline void
_sync_client_run_thread_frame(SyncServer *pServer, ub frame_len, u8 *frame)
{
	ThreadId route_src, route_dst;
	s8 src[SYNC_THREAD_NAME_LEN];
	s8 dst[SYNC_THREAD_NAME_LEN];
	ub msg_id;
	BaseMsgType msg_type;
	u8 *package_ptr = NULL;
	ub package_len = 0;
	void *msg_body = NULL;
	ub msg_len = 0;
	ErrCode ret = ERRCODE_OK;

	sync_msg_unpacket(
		frame, frame_len,
		&route_src, &route_dst, src, dst, &msg_id,
		&msg_type, NULL, NULL,
		&package_len, &package_ptr);

	if(t_rpc_unzip(&msg_body, &msg_len, msg_id, (s8 *)package_ptr, package_len) == dave_false)
	{
		SYNCLTRACE(60,1,"%s/%lx/%d/%d->%s/%lx/%d/%d msg_type:%d msg_id:%d packet_len:%d",
			src, route_src, thread_get_thread(route_src), thread_get_net(route_src),
			dst, route_dst, thread_get_thread(route_dst), thread_get_net(route_dst),
			msg_type, msg_id, package_len);

		dave_memset(msg_body, 0x00, msg_len);
	}

	sync_client_detected_rpc_efficiency(msg_len, package_len, msg_id);

	if((src[0] != '\0') && (dst[0] != '\0') && (msg_id != MSGID_RESERVED) && (msg_len > 0))
	{
		if(_sync_client_run_thread(
			pServer,
			src, route_src,
			dst, route_dst,
			msg_id,
			msg_type,
			msg_len, msg_body,
			dave_false) == dave_false)
		{
			ret = ERRCODE_msg_queue_is_full;
		}
	}
	else
	{
		ret = ERRCODE_Invalid_parameter;
	}

	sync_client_tx_run_thread_msg_rsp(pServer, src, dst, msg_id, ret);

	if(ret != ERRCODE_OK)
	{
		SYNCLTRACE(60,1,"%s, %s->%s:%d/%d", errorstr(ret), src, dst, msg_id, msg_len);

		thread_msg_release(msg_body);
	}
}

// =====================================================================

void
sync_client_run_init(void)
{
	_sync_client_thread = thread_id(SYNC_CLIENT_THREAD_NAME);
}

void
sync_client_run_exit(void)
{

}

void
sync_client_run_thread(SyncServer *pServer, ub frame_len, u8 *frame)
{
#ifdef SYNC_RUN_THREAD_ON_EVENTS

	SyncClientRunThreadEvents *pRunThread;
	InternalEvents *pEvents;

	pRunThread = dave_malloc(sizeof(SyncClientRunThreadEvents));
	pRunThread->pServer = pServer;
	pRunThread->frame_mbuf = dave_mmalloc(frame_len);
	dave_memcpy(dave_mptr(pRunThread->frame_mbuf), frame, frame_len);

	pEvents = thread_msg(pEvents);
	pEvents->event_id = 0;
	pEvents->ptr = pRunThread;

	if(write_msg(_sync_client_thread, MSGID_INTERNAL_EVENTS, pEvents) == dave_false)
	{
		SYNCLTRACE(60,1,"MSGID_INTERNAL_EVENTS failed! _sync_client_thread:%lx", _sync_client_thread);
		sync_client_run_thread_events(pRunThread);
	}

#else

	_sync_client_run_thread_frame(pServer, frame_len, frame);

#endif
}

void
sync_client_run_thread_events(void *ptr)
{
	SyncClientRunThreadEvents *pRunThread = (SyncClientRunThreadEvents *)ptr;

	_sync_client_run_thread_frame(pRunThread->pServer, (ub)(pRunThread->frame_mbuf->len), (u8 *)(pRunThread->frame_mbuf->payload));

	dave_mfree(pRunThread->frame_mbuf);

	dave_free(pRunThread);
}

void
sync_client_run_internal(
	s8 *src, s8 *dst,
	ub msg_id,
	ub msg_len, u8 *msg_body)
{
	_sync_client_run_internal(
		src, dst,
		msg_id,
		msg_len, msg_body);
}

#endif

