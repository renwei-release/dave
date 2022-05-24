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
#include "base_rxtx.h"
#include "thread_tools.h"
#include "sync_param.h"
#include "sync_globally_identifier.h"
#include "sync_base_package.h"
#include "sync_server_param.h"
#include "sync_server_tx.h"
#include "sync_lock.h"
#include "sync_test.h"
#include "sync_log.h"

static dave_bool
_sync_server_tx(SyncClient *pClient, ORDER_CODE order_id, MBUF *data)
{
	dave_bool ret;

	if(pClient->client_socket == INVALID_SOCKET_ID)
	{
		SYNCLOG("invalid socket!");
		return dave_false;
	}

	if(data == NULL)
	{
		SYNCABNOR(("invalid data!"));
		return dave_false;
	}

	sync_lock();
	pClient->send_data_counter ++;
	sync_unlock();

	ret = rxtx_writes(pClient->client_socket, order_id, data);
	if(ret == dave_false)
	{
		SYNCLOG("client_socket:%d verno:%s write_failed!", pClient->client_socket, pClient->verno);
	}

	return ret;
}

static dave_bool
_sync_server_tx_run_internal_msg_req(
	SyncClient *pClient,
	ub msg_id, ub msg_len, void *msg_body)
{
	ub snd_max = SYNC_STACK_HEAD_MAX_LEN + msg_len;
	MBUF *snd_buffer;

	SYNCDEBUG("msg_id:%d msg_len:%d", msg_id, msg_len);

	snd_buffer = dave_mmalloc(snd_max);

	snd_buffer->tot_len = snd_buffer->len = sync_msg_packet(
		dave_mptr(snd_buffer), snd_max,
		INVALID_THREAD_ID, INVALID_THREAD_ID, (s8 *)SYNC_SERVER_THREAD_NAME, (s8 *)SYNC_CLIENT_THREAD_NAME, msg_id,
		BaseMsgType_Unicast, LOCAL_TASK_ATTRIB, LOCAL_TASK_ATTRIB,
		msg_len, msg_body);

	return _sync_server_tx(pClient, ORDER_CODE_RUN_INTERNAL_MSG_REQ, snd_buffer);
}

static dave_bool
_sync_server_tx_run_thread_msg_req(
	SyncClient *pClient,
	ThreadId route_src, ThreadId route_dst,
	s8 *src, s8 *dst,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	ub msg_id,
	BaseMsgType msg_type,
	ub msg_len, void *msg_body)
{
	ub snd_max = SYNC_STACK_HEAD_MAX_LEN + msg_len;
	MBUF *snd_buffer;

	SYNCDEBUG("%s/%lx/%d/%d->%s/%lx/%d/%d id:%d len:%d",
		src, route_src, thread_get_thread(route_src), thread_get_net(route_src),
		dst, route_dst, thread_get_thread(route_dst), thread_get_net(route_dst),
		msg_id, msg_len);

	snd_buffer = dave_mmalloc(snd_max);

	snd_buffer->tot_len = snd_buffer->len = sync_msg_packet(
		dave_mptr(snd_buffer), snd_max,
		route_src, route_dst, src, dst, msg_id,
		msg_type, src_attrib, dst_attrib,
		msg_len, msg_body);

	return _sync_server_tx(pClient, ORDER_CODE_RUN_THREAD_MSG_REQ, snd_buffer);
}

static void
_sync_server_tx_blocks_state(SyncClient *pClient)
{
	if(pClient == NULL)
	{
		SYNCLOG("pClient is NULL!");
		return;
	}

	if(pClient->notify_blocks_flag == (ub)(pClient->blocks_flag))
	{
		return;
	}

	SYNCTRACE("verno:%s blocks_flag:%d", pClient->verno, pClient->blocks_flag);

	if(pClient->blocks_flag == dave_true)
	{
		SystemMount mount;

		dave_memset(&mount, 0x00, sizeof(mount));

		mount.socket = pClient->client_socket;
		dave_strcpy(mount.verno, pClient->verno, DAVE_VERNO_STR_LEN);
		mount.NetInfo = pClient->NetInfo;

		sync_server_tx_run_internal_msg_req(pClient, MSGID_SYSTEM_MOUNT, sizeof(mount), &mount);
	}
	else
	{
		SystemDecoupling decoupling;

		dave_memset(&decoupling, 0x00, sizeof(decoupling));

		decoupling.socket = pClient->client_socket;
		dave_strcpy(decoupling.verno, pClient->verno, DAVE_VERNO_STR_LEN);
		decoupling.NetInfo = pClient->NetInfo;

		sync_server_tx_run_internal_msg_req(pClient, MSGID_SYSTEM_DECOUPLING, sizeof(decoupling), &decoupling);
	}

	pClient->notify_blocks_flag = (ub)(pClient->blocks_flag);
}

// =====================================================================

RetCode
sync_server_tx_run_thread_msg_req(
	SyncThread *pSrcThread, SyncThread *pDstThread,
	SyncClient *pSrcClient, SyncClient *pDstClient,
	ThreadId route_src, ThreadId route_dst,
	s8 *src, s8 *dst,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	ub msg_id,
	BaseMsgType msg_type,
	u8 *msg_body, ub msg_len)
{
	RetCode ret = RetCode_not_access;
	ub src_thread_index, dst_thread_index;

	if(pSrcThread != NULL)
	{
		src_thread_index = pSrcThread->thread_index;
	}
	else
	{
		SYNCDEBUG("%s is an internal thread and is not synchronized to other services.", src);
		src_thread_index = SYNC_THREAD_INDEX_MAX;
	}
	dst_thread_index = pDstThread->thread_index;

	route_src = thread_set_remote(0, thread_get_local(route_src), src_thread_index, pSrcClient->client_index);
	route_dst = thread_set_remote(0, thread_get_local(route_dst), dst_thread_index, pDstClient->client_index);

	SYNCDEBUG("%s/%lx->%s/%lx msg_id:%d msg_type:%d msg_len:%d client:%d/%d/%s->%d/%d/%s",
		src, route_src, dst, route_dst,
		msg_id, msg_type, msg_len,
		src_thread_index, pSrcClient->client_index, pSrcClient->verno,
		dst_thread_index, pDstClient->client_index, pDstClient->verno);

	if((route_src != INVALID_THREAD_ID) && (route_dst != INVALID_THREAD_ID))
	{
		if(_sync_server_tx_run_thread_msg_req(
				pDstClient,
				route_src, route_dst,
				src, dst,
				src_attrib, dst_attrib,
				msg_id,
				msg_type,
				msg_len, msg_body) == dave_true)
		{
			ret = RetCode_OK;
		}
		else
		{
			ret = RetCode_Send_failed;
		}
	}
	else
	{
		ret = RetCode_Invalid_parameter;
	}

	if(ret != RetCode_OK)
	{
		SYNCLOG("ret:%s %s/%lx->%s/%lx msg_id:%d msg_type:%d msg_len:%d client:%d/%d/%s->%d/%d/%s",
			retstr(ret),
			src, route_src, dst, route_dst,
			msg_id, msg_type, msg_len,
			src_thread_index, pSrcClient->client_index, pSrcClient->verno,
			dst_thread_index, pDstClient->client_index, pDstClient->verno);
	}
	else
	{
		sync_lock();
		if(pSrcClient != NULL)
			pSrcClient->recv_msg_counter ++;
		if(pDstClient != NULL)
			pDstClient->send_msg_counter ++;
		if(pSrcThread != NULL)
			pSrcThread->thread_recv_message_counter ++;
		if(pDstThread != NULL)
			pDstThread->thread_send_message_counter ++;
		sync_unlock();
	}

	return ret;
}

dave_bool
sync_server_tx_run_internal_msg_req(
	SyncClient *pClient,
	ub msg_id, ub msg_len, void *msg_body)
{
	return _sync_server_tx_run_internal_msg_req(pClient, msg_id, msg_len, msg_body);
}

dave_bool
sync_server_tx_test_run_thread_msg_req(
	SyncClient *pClient,
	ThreadId route_src, ThreadId route_dst,
	s8 *my_name, s8 *other_name,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	ub msg_id,
	BaseMsgType msg_type,
	u8 *msg_body, ub msg_len)
{
	ub snd_max = SYNC_STACK_HEAD_MAX_LEN + msg_len;
	MBUF *snd_buffer;

	SYNCDEBUG("msg_id:%d msg_len:%d", msg_id, msg_len);

	snd_buffer = dave_mmalloc(snd_max);

	snd_buffer->tot_len = snd_buffer->len = sync_msg_packet(
		dave_mptr(snd_buffer), snd_max,
		route_src, route_dst, my_name, other_name, msg_id,
		msg_type, src_attrib, dst_attrib,
		msg_len, msg_body);

	return _sync_server_tx(pClient, ORDER_CODE_TEST_RUN_THREAD_MSG_REQ, snd_buffer);
}

void
sync_server_tx_my_verno(SyncClient *pClient)
{
	MBUF *snd_buffer;
	u8 *snd_ptr;
	ub snd_max = 2048;
	ub snd_index = 0;

	snd_buffer = dave_mmalloc(snd_max);
	snd_ptr = dave_mptr(snd_buffer);

	snd_index += sync_str_packet(&snd_ptr[snd_index], snd_max-snd_index, dave_verno());
	snd_index += sync_str_packet(&snd_ptr[snd_index], snd_max-snd_index, sync_globally_identifier());
	snd_index += sync_ip_packet(&snd_ptr[snd_index], snd_max-snd_index, pClient->NetInfo.addr.ip.ip_addr);

	snd_buffer->len = snd_buffer->tot_len = snd_index;

	_sync_server_tx(pClient, ORDER_CODE_MY_VERNO, snd_buffer);
}

void
sync_server_tx_module_verno(SyncClient *pClient, s8 *verno)
{
	ub snd_max = DAVE_VERNO_STR_LEN * 16;
	MBUF *snd_buffer;

	if((pClient->verno[0] != '\0') && (verno[0] != '\0'))
	{
		snd_buffer = dave_mmalloc(snd_max);

		snd_buffer->tot_len = snd_buffer->len = sync_str_packet(dave_mptr(snd_buffer), snd_max, verno);

		_sync_server_tx(pClient, ORDER_CODE_MODULE_VERNO, snd_buffer);
	}
	else
	{
		SYNCLOG("client verno:%s/%s not ready!", pClient->verno, verno);
	}
}

void
sync_server_tx_heartbeat(SyncClient *pClient)
{
	MBUF *snd_buffer;

	snd_buffer = sync_heartbeat_packet(pClient->recv_data_counter, pClient->send_data_counter, t_time_get_date(NULL));

	_sync_server_tx(pClient, ORDER_CODE_HEARTBEAT_RSP, snd_buffer);
}

void
sync_server_tx_sync_thread_name_rsp(SyncClient *pClient, s8 *thread_name, sb thread_index)
{
	MBUF *snd_buffer;

	snd_buffer = sync_thread_name_packet(pClient->verno, pClient->globally_identifier, thread_name, thread_index);

	_sync_server_tx(pClient, ORDER_CODE_SYNC_THREAD_NAME_RSP, snd_buffer);
}

dave_bool
sync_server_tx_add_remote_thread_req(SyncClient *pClient, s8 *thread_name, sb thread_index)
{
	MBUF *snd_buffer;

	snd_buffer = sync_thread_name_packet(pClient->verno, pClient->globally_identifier, thread_name, thread_index);

	return _sync_server_tx(pClient, ORDER_CODE_ADD_REMOTE_THREAD_REQ, snd_buffer);
}

dave_bool
sync_server_tx_del_remote_thread_req(SyncClient *pClient, s8 *thread_name, sb thread_index)
{
	MBUF *snd_buffer;

	snd_buffer = sync_thread_name_packet(pClient->verno, pClient->globally_identifier, thread_name, thread_index);

	return _sync_server_tx(pClient, ORDER_CODE_DEL_REMOTE_THREAD_REQ, snd_buffer);
}

void
sync_server_tx_blocks_state(SyncClient *pClient)
{
	_sync_server_tx_blocks_state(pClient);
}

dave_bool
sync_server_tx_link_up_req(SyncClient *pClient, s8 *verno, s8 *globally_identifier, u8 *link_ip, u16 link_port)
{
	MBUF *snd_buffer;

	snd_buffer = sync_link_packet(verno, link_ip, link_port, globally_identifier);

	SYNCTRACE("to %s link %s:%s up!", pClient->verno, verno, ipv4str(link_ip, link_port));

	return _sync_server_tx(pClient, ORDER_CODE_LINK_UP_REQ, snd_buffer);
}

dave_bool
sync_server_tx_link_up_rsp(SyncClient *pClient)
{
	MBUF *snd_buffer;

	snd_buffer = sync_link_packet(pClient->verno, pClient->link_ip, pClient->link_port, pClient->globally_identifier);

	return _sync_server_tx(pClient, ORDER_CODE_LINK_UP_RSP, snd_buffer);
}

dave_bool
sync_server_tx_link_down_req(SyncClient *pClient, s8 *verno, u8 *link_ip, u16 link_port)
{
	MBUF *snd_buffer;

	snd_buffer = sync_link_packet(verno, link_ip, link_port, pClient->globally_identifier);

	SYNCTRACE("to %s link:%s:%s down!", pClient->verno, verno, ipv4str(link_ip, link_port));

	return _sync_server_tx(pClient, ORDER_CODE_LINK_DOWN_REQ, snd_buffer);
}

dave_bool
sync_server_tx_link_down_rsp(SyncClient *pClient)
{
	MBUF *snd_buffer;

	snd_buffer = sync_link_packet(pClient->verno, pClient->link_ip, pClient->link_port, pClient->globally_identifier);

	return _sync_server_tx(pClient, ORDER_CODE_LINK_DOWN_RSP, snd_buffer);
}

dave_bool
sync_server_tx_rpcver_req(SyncClient *pClient)
{
	MBUF *snd_buffer;

	snd_buffer = sync_rpcver_packet(3);

	return _sync_server_tx(pClient, ORDER_CODE_RPCVER_REQ, snd_buffer);
}

dave_bool
sync_server_tx_rpcver_rsp(SyncClient *pClient)
{
	MBUF *snd_buffer;

	snd_buffer = sync_rpcver_packet(3);

	return _sync_server_tx(pClient, ORDER_CODE_RPCVER_RSP, snd_buffer);
}

#endif

