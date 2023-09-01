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
#include "dave_os.h"
#include "base_rxtx.h"
#include "thread_tools.h"
#include "sync_param.h"
#include "sync_base_package.h"
#include "sync_server_param.h"
#include "sync_server_tx.h"
#include "sync_server_rx.h"
#include "sync_server_data.h"
#include "sync_server_broadcadt.h"
#include "sync_server_tools.h"
#include "sync_server_sync.h"
#include "sync_server_run.h"
#include "sync_server_msg_buffer.h"
#include "sync_server_flag.h"
#include "sync_lock.h"
#include "sync_test.h"
#include "sync_log.h"

static void
_sync_server_rx_disconnect(s32 socket)
{
	SocketDisconnectReq *pReq;

	if(socket == INVALID_SOCKET_ID)
	{
		return;
	}
	
	SYNCTRACE("socket:%d", socket);

	pReq = thread_reset_msg(pReq);

	pReq->socket = socket;
	pReq->ptr = NULL;

	name_msg(SOCKET_THREAD_NAME, SOCKET_DISCONNECT_REQ, pReq);
}

static void
_sync_server_rx_show_thread_client(SyncThread *pThread)
{
	ub client_number;
	ub client_index;
	SyncClient *pClient;

	client_number = 0;

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		pClient = pThread->pClient[client_index];
	
		if(pClient != NULL)
		{
			SYNCTRACE("verno:%s msg:%d/%d ready:%d blocks:%d client:%d release_quantity:%d",
				pClient->verno,
				pClient->recv_data_counter,
				pClient->send_data_counter,
				pClient->ready_flag,
				pClient->blocks_flag,
				pClient->client_flag,
				pClient->release_quantity);

			client_number ++;
		}
	}

	if(client_number == 0)
	{
		SYNCABNOR("thread:%s has empty client!", pThread->thread_name);
	}
}

static dave_bool
_sync_server_rx_find_thread_and_client_from_route_dst(
	ThreadId route_src, s8 *src,
	ThreadId route_dst, s8 *dst,
	ub msg_id,
	SyncThread **ppDstThread, SyncClient **ppDstClient)
{
	ub thread_index, client_index;
	dave_bool ret = dave_true;

	if((thread_is_remote(route_dst) == dave_true)
		&& (thread_get_thread(route_dst) < SYNC_THREAD_INDEX_MAX)
		&& (thread_get_net(route_dst) < SYNC_NET_INDEX_MAX))
	{
		thread_index = thread_get_thread(route_dst);
		if(thread_index >= SYNC_THREAD_MAX)
		{
			SYNCLOG("invalid route_dst:%x thread_index:%d!", route_dst, thread_index);
			*ppDstThread = NULL;
		}
		else
		{
			*ppDstThread = sync_server_thread(thread_index);
			if(dave_strcmp((*ppDstThread)->thread_name, dst) == dave_false)
			{
				SYNCLOG("%s<%lx>->%s<%s><%lx> %d the thread lost!",
					src, route_src,
					dst, (*ppDstThread)->thread_name, route_dst,
					msg_id);
				*ppDstThread = NULL;
			}
		}

		client_index = thread_get_net(route_dst);
		if(client_index >= SYNC_CLIENT_MAX)
		{
			*ppDstClient = NULL;
		}
		else
		{
			*ppDstClient = sync_server_client(client_index);
			if((*ppDstClient)->client_socket == INVALID_SOCKET_ID)
			{
				SYNCLTRACE(60,10,"%s<%lx>->%s<%lx>:%d the client(%d) lost!",
					src, route_src,
					dst, route_dst,
					msg_id,
					client_index);
				*ppDstClient = NULL;
			}
			else
			{
				if(sync_server_client_on_thread(*ppDstThread, *ppDstClient) == dave_false)
				{
					SYNCLTRACE(60,10,"%s's client:%s mismatch! %s->%s:%d",
						(*ppDstThread)->thread_name, (*ppDstClient)->verno,
						src, dst, msg_id);
					ret = dave_false;
				}
			}
		}
	}

	return ret;
}

static inline SyncClient *
_sync_server_rx_select_client(SyncThread *pThread)
{
	SyncClient *pClient = NULL;
	ub client_index;
	ub safe_counter;

	client_index = pThread->chose_client_index % SYNC_CLIENT_MAX;
	for(safe_counter=0; safe_counter<SYNC_CLIENT_MAX; safe_counter++)
	{
		if(client_index >= SYNC_CLIENT_MAX)
		{
			client_index = 0;
		}

		if(pThread->pClient[client_index] != NULL)
		{
			if(sync_server_client_on_work(pThread->pClient[client_index]) == dave_true)
			{
				pClient = pThread->pClient[client_index];

				pThread->chose_client_index = ++ client_index;
				break;
			}
		}

		client_index ++;
	}

	return pClient;
}

static inline SyncClient *
_sync_server_rx_safe_select_client(SyncThread *pThread)
{
	SyncClient *pClient;

	t_lock_spin(&(pThread->chose_client_pv));
	pClient = _sync_server_rx_select_client(pThread);
	t_unlock_spin(&(pThread->chose_client_pv));

	return pClient;
}

static SyncClient *
_sync_server_rx_select_cycle_client_on_thread(SyncThread *pThread)
{
	SyncClient *pClient = _sync_server_rx_safe_select_client(pThread);

	if(pClient == NULL)
	{
		SYNCTRACE("thread:%s can't find pClient!", pThread->thread_name);

		_sync_server_rx_show_thread_client(pThread);
	}
	else
	{
		sync_lock();
		if(pClient->release_quantity > 0)
		{
			pClient->release_quantity --;
		}
		sync_unlock();
	}

	return pClient;
}

static void
_sync_server_rx_find_thread_and_client_from_thread_table(
	ThreadId route_src, s8 *src,
	ThreadId route_dst, s8 *dst,
	ub msg_id,
	SyncThread **ppDstThread, SyncClient **ppDstClient)
{
	*ppDstThread = sync_server_find_thread(dst);
	if(*ppDstThread != NULL)
	{
		*ppDstClient = _sync_server_rx_select_cycle_client_on_thread(*ppDstThread);
	}
	else
	{
		SYNCTRACE("the %s/%x->%s/%x:%d thread lost! <%d>",
			src, src, dst, dst, msg_id,
			thread_get_thread(route_dst));
		*ppDstClient = NULL;
	}
}

static dave_bool
_sync_server_rx_build_run_req_param(
	ThreadId route_src, s8 *src,
	ThreadId route_dst, s8 *dst,
	ub msg_id,
	SyncThread **ppDstThread, SyncClient **ppDstClient)
{
	*ppDstThread = NULL;
	*ppDstClient = NULL;

	if(_sync_server_rx_find_thread_and_client_from_route_dst(route_src, src, route_dst, dst, msg_id, ppDstThread, ppDstClient) == dave_false)
	{
		return dave_false;
	}

	if((*ppDstClient) == NULL)
	{
		/*
		 * 没有找到任何可发送的客户端，那么系统自动选择一个。
		 * 一般情况下，请求消息或者未确定发给那个具体服务的消息会经过这条选择逻辑。
		 * 而应答消息不需要经过这个选择逻辑，应答消息通过线程目的地ID里面的net字段来
		 * 自动选择需要发到那个客户端，该操作由
		 * _sync_server_rx_find_thread_and_client_from_route_dst 完成。
		 */
		_sync_server_rx_find_thread_and_client_from_thread_table(route_src, src, route_dst, dst, msg_id, ppDstThread, ppDstClient);
	}

	return dave_true;
}

static RetCode
_sync_server_rx_run_alone_thread_msg(
	SyncThread *pSrcThread, SyncClient *pSrcClient,
	ThreadId route_src, s8 *src,
	ThreadId route_dst, s8 *dst,
	ub msg_id,
	BaseMsgType msg_type,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	u8 *msg_body, ub msg_len)
{
	RetCode ret;
	SyncThread *pDstThread;
	SyncClient *pDstClient;

	if(_sync_server_rx_build_run_req_param(route_src, src, route_dst, dst, msg_id, &pDstThread, &pDstClient) == dave_false)
	{
		SYNCLTRACE(60,10,"lllegal routing of messages, maybe the service receiving this message has been disconnected! %s(%lx)->%s(%lx):%d",
			src, route_src, dst, route_dst, msg_id);
		return RetCode_OK;
	}

	if((pDstThread != NULL) && (pDstClient != NULL))
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

		ret = sync_server_tx_run_thread_msg_req(
				pSrcThread, pDstThread,
				pSrcClient, pDstClient,
				route_src, route_dst,
				src, dst,
				src_attrib, dst_attrib,
				msg_id,
				msg_type,
				msg_body, msg_len);
	}
	else
	{
		if(pDstThread == NULL)
		{
			ret = RetCode_can_not_find_thread;
		}
		else
		{
			ret = RetCode_can_not_find_client;
		}
	}

	return ret;
}

static RetCode
_sync_server_rx_run_broadcadt_thread_msg(
	SyncThread *pSrcThread, SyncClient *pSrcClient,
	ThreadId route_src, s8 *src,
	ThreadId route_dst, s8 *dst,
	ub msg_id,
	BaseMsgType msg_type,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	u8 *msg_body, ub msg_len)
{
	sync_lock();
	if(pSrcClient != NULL)
		pSrcClient->recv_msg_counter ++;
	if(pSrcThread != NULL)
		pSrcThread->thread_recv_message_counter ++;
	sync_unlock();

	return sync_server_broadcadt(
		pSrcClient,
		route_src, src,
		route_dst, dst,
		msg_id,
		msg_type,
		src_attrib, dst_attrib,
		msg_body, msg_len);
}

static void
_sync_server_rx_snd_events(SyncServerEvents events, void *ptr)
{
	InternalEvents *pEvents = thread_msg(pEvents);

	pEvents->event_id = (ub)events;
	pEvents->ptr = ptr;

	id_msg(self(), MSGID_INTERNAL_EVENTS, pEvents);
}

static void
_sync_server_rx_verno(SyncClient *pClient, ub frame_len, u8 *frame_ptr)
{
	dave_bool frist_get_version;
	u8 detect_my_ip[16];
	ub frame_index = 0;

	if(pClient->verno[0] == '\0')
		frist_get_version = dave_true;
	else
		frist_get_version = dave_false;

	frame_index += sync_str_unpacket(&frame_ptr[frame_index], frame_len-frame_index, pClient->verno, sizeof(pClient->verno));
	frame_index += sync_str_unpacket(&frame_ptr[frame_index], frame_len-frame_index, pClient->globally_identifier, sizeof(pClient->globally_identifier));
	if(frame_index < frame_len)
	{
		frame_index += sync_ip_unpacket(&frame_ptr[frame_index], frame_len-frame_index, detect_my_ip);
		frame_index += sync_str_unpacket(&frame_ptr[frame_index], frame_len-frame_index, pClient->host_name, sizeof(pClient->host_name));
	}

	pClient->work_start_second = dave_os_time_s();

	sync_server_tx_my_verno(pClient);

	sync_server_tx_rpcver_req(pClient);

	_sync_server_rx_snd_events(SyncServerEvents_version, pClient);

	if(frist_get_version == dave_true)
	{
		SYNCLOG("socket:%d %s/%d %s/%s",
			pClient->client_socket,
			ipv4str(pClient->NetInfo.addr.ip.ip_addr, pClient->NetInfo.port),
			pClient->NetInfo.src_port,
			pClient->globally_identifier, pClient->verno);
	}
}

static void
_sync_server_rx_heartbeat_req(SyncClient *pClient, ub frame_len, u8 *frame_ptr)
{
	ub recv_data_counter, send_data_counter;

	SYNCDEBUG("%s/%s", pClient->globally_identifier, pClient->verno);

	sync_heartbeat_unpacket(frame_ptr, frame_len, &recv_data_counter, &send_data_counter, NULL);

	sync_server_tx_heartbeat(pClient, dave_false);
}

static void
_sync_server_rx_sync_thread_name_req(SyncClient *pClient, ub frame_len, u8 *frame_ptr)
{
	s8 verno[DAVE_VERNO_STR_LEN];
	s8 globally_identifier[DAVE_GLOBALLY_IDENTIFIER_LEN];
	s8 thread_name[SYNC_THREAD_NAME_LEN];
	sb thread_index;

	sync_thread_name_unpacket(frame_ptr, frame_len, verno, globally_identifier, thread_name, &thread_index);

	SYNCDEBUG("verno:%s thread:%s", pClient->verno, thread_name);

	if(thread_name[0] != '\0')
	{
		sync_server_add_thread(pClient, thread_name);

		sync_server_tx_sync_thread_name_rsp(pClient, thread_name, thread_index);
	}
	else
	{
		SYNCTRACE("the client:%s/%s sync done!",
			pClient->verno, pClient->globally_identifier);

		pClient->receive_thread_done = dave_true;
	}
}

static void
_sync_server_rx_add_remote_thread_rsp(SyncClient *pClient, ub frame_len, u8 *frame_ptr)
{
	s8 verno[DAVE_VERNO_STR_LEN];
	s8 globally_identifier[DAVE_GLOBALLY_IDENTIFIER_LEN];
	s8 thread_name[SYNC_THREAD_NAME_LEN];
	sb thread_index;
	SyncThread *pThread, *pCurrentThread;

	sync_thread_name_unpacket(frame_ptr, frame_len, verno, globally_identifier, thread_name, &thread_index);
	if(thread_index >= 0)
	{
		pClient->sync_thread_index = thread_index;
	}

	SYNCDEBUG("socket:%d thread:%s sync index:%d",
		pClient->client_socket,
		thread_name,
		pClient->sync_thread_index);

	if(pClient->sync_thread_index < SYNC_THREAD_MAX)
	{
		pThread = sync_server_thread(pClient->sync_thread_index);

		if(dave_strcmp(pThread->thread_name, thread_name) == dave_false)
		{
			pCurrentThread = sync_server_find_thread(thread_name);

			SYNCLOG("%x/%s thread<%s/%s> not match, maybe a service \
disconnection event occurred while synchronizing. resynchronization(index:%d/%d counter:%d)!",
				pClient, pClient->verno,
				pThread->thread_name, thread_name,
				pClient->sync_thread_index, pCurrentThread == NULL ? -1 : pCurrentThread->thread_index,
				pClient->sync_resynchronization_counter);

			if((pClient->sync_resynchronization_counter ++) < SYNC_THREAD_MAX)
			{
				if(pCurrentThread == NULL)
				{
					pClient->sync_thread_index = 0;
				}
				else
				{
					pClient->sync_thread_index = pCurrentThread->thread_index;
				}
			}
		}
	}
	else
	{
		SYNCABNOR("sync thread buffer overflow!");
	}

	sync_server_sync_thread_next(pClient);
}

static void
_sync_server_rx_del_remote_thread_rsp(SyncClient *pClient, ub frame_len, u8 *frame_ptr)
{
	s8 verno[DAVE_VERNO_STR_LEN];
	s8 globally_identifier[DAVE_GLOBALLY_IDENTIFIER_LEN];
	s8 thread_name[SYNC_THREAD_NAME_LEN];
	sb thread_index;

	sync_thread_name_unpacket(frame_ptr, frame_len, verno, globally_identifier, thread_name, &thread_index);

	SYNCDEBUG("verno:%s del thread:%s success!", pClient->verno, thread_name);
}

static RetCode
_sync_server_rx_run_thread_msg_req(SyncClient *pClient, ub frame_len, u8 *frame_ptr, dave_bool buffer_pop)
{
	ThreadId route_src, route_dst;
	s8 src[SYNC_THREAD_NAME_LEN];
	s8 dst[SYNC_THREAD_NAME_LEN];
	ub msg_id = MSGID_RESERVED;
	BaseMsgType msg_type;
	TaskAttribute src_attrib, dst_attrib;
	ub msg_len;
	u8 *msg_body;
	SyncThread *pSrcThread = NULL;
	RetCode ret;

	sync_msg_unpacket(
		frame_ptr, frame_len,
		&route_src, &route_dst, src, dst, &msg_id,
		&msg_type, &src_attrib, &dst_attrib,
		&msg_len, &msg_body);

	SYNCDEBUG("%s/%d/%d->%s/%d/%d msg_id:%d msg_type:%d msg_len:%d",
		src, thread_get_thread(route_src), thread_get_net(route_src),
		dst, thread_get_thread(route_dst), thread_get_net(route_dst),
		msg_id, msg_type, msg_len);

	if((src[0] != '\0') && (dst[0] != '\0') && (msg_id != MSGID_RESERVED) && (msg_len > 0))
	{
		pSrcThread = sync_server_find_thread(src);

		if(msg_type == BaseMsgType_Unicast)
		{
			ret = _sync_server_rx_run_alone_thread_msg(
					pSrcThread, pClient,
					route_src, src,
					route_dst, dst,
					msg_id,
					msg_type,
					src_attrib, dst_attrib,
					msg_body, msg_len);
		}
		else
		{
			ret = _sync_server_rx_run_broadcadt_thread_msg(
					pSrcThread, pClient,
					route_src, src,
					route_dst, dst,
					msg_id,
					msg_type,
					src_attrib, dst_attrib,
					msg_body, msg_len);
		}

		if(ret != RetCode_OK)
		{
			SYNCDEBUG("%s %s/%x/%lx/%d/%d->%s/%x/%lx/%d/%d id:%d len:%d ret:%s",
				buffer_pop == dave_false ? "push buffer" : "pop buffer",
				src, src, route_src, thread_get_thread(route_src), thread_get_net(route_src),
				dst, dst, route_dst, thread_get_thread(route_dst), thread_get_net(route_dst),
				msg_id, msg_len,
				retstr(ret));

			if(buffer_pop == dave_false)
			{
				if((dave_strcmp(src, "NULL") == dave_false)
					&& (dave_strcmp(dst, "NULL") == dave_false))
				{
					sync_server_msg_buffer_push(pClient, frame_len, frame_ptr, _sync_server_rx_run_thread_msg_req);
				}
				else
				{
					SYNCLTRACE(60,1,"can't push message! %s/%x/%lx/%d/%d->%s/%x/%lx/%d/%d id:%d len:%d ret:%s",
						src, src, route_src, thread_get_thread(route_src), thread_get_net(route_src),
						dst, dst, route_dst, thread_get_thread(route_dst), thread_get_net(route_dst),
						msg_id, msg_len,
						retstr(ret));
				}
			}
		}
	}
	else
	{
		SYNCABNOR("find invalid parameter, src:%s dst:%s msg_id:%d msg_len:%d buffer_pop:%d",
			src, dst, msg_id, msg_len, buffer_pop);

		ret = RetCode_OK;
	}

	return ret;
}

static void
_sync_server_rx_test_run_thread_msg_req(SyncClient *pClient, ub frame_len, u8 *frame_ptr)
{
	ThreadId route_src, route_dst;
	s8 my_name[SYNC_THREAD_NAME_LEN];
	s8 other_name[SYNC_THREAD_NAME_LEN];
	ub msg_id = MSGID_RESERVED;
	BaseMsgType msg_type;
	TaskAttribute src_attrib, dst_attrib;
	ub msg_len;
	u8 *msg_body;

	sync_msg_unpacket(
		frame_ptr, frame_len,
		&route_src, &route_dst, my_name, other_name, &msg_id,
		&msg_type, &src_attrib, &dst_attrib,
		&msg_len, &msg_body);

	sync_server_run_test(pClient,
		route_src, route_dst,
		my_name, other_name,
		msg_id,
		msg_type,
		src_attrib, dst_attrib,
		msg_len, msg_body);
}

static void
_sync_server_rx_run_internal_msg_req(SyncClient *pClient, ub frame_len, u8 *frame_ptr)
{
	ThreadId route_src, route_dst;
	s8 src[SYNC_THREAD_NAME_LEN];
	s8 dst[SYNC_THREAD_NAME_LEN];
	ub msg_id;
	BaseMsgType msg_type;
	TaskAttribute src_attrib, dst_attrib;
	u8 *packet_ptr = NULL;
	ub packet_len = 0;
	void *msg_body = NULL;
	ub msg_len = 0;

	sync_lock();
	pClient->recv_msg_counter ++;
	sync_unlock();

	sync_msg_unpacket(
		frame_ptr, frame_len,
		&route_src, &route_dst, src, dst, &msg_id,
		&msg_type, &src_attrib, &dst_attrib,
		&packet_len, &packet_ptr);

	msg_len = packet_len;
	if(msg_len > 0)
	{
		msg_body = base_thread_msg_creat(msg_len, dave_false, (s8 *)__func__, (ub)__LINE__);
		dave_memcpy(msg_body, packet_ptr, msg_len);
	}

	if((src[0] != '\0') && (dst[0] != '\0') && (msg_id != MSGID_RESERVED) && (msg_len > 0))
	{
		sync_server_run_internal(pClient, src, dst, msg_id, msg_len, msg_body);
	}
	else
	{
		SYNCABNOR("find invalid parameter, src:%s dst:%s msg_id:%d msg_len:%d",
			src, dst, msg_id, msg_len);

		thread_msg_release(msg_body);
	}
}

static inline void
_sync_server_rx_run_internal_msg_v2_req(SyncClient *pClient, ub frame_len, u8 *frame_ptr)
{
	ThreadId route_src, route_dst;
	s8 src[SYNC_THREAD_NAME_LEN];
	s8 dst[SYNC_THREAD_NAME_LEN];
	ub msg_id;
	BaseMsgType msg_type;
	TaskAttribute src_attrib, dst_attrib;
	u8 *packet_ptr = NULL;
	ub packet_len = 0;
	void *pChainBson = NULL, *pRouterBson = NULL;
	void *msg_body = NULL;
	ub msg_len = 0;

	sync_msg_unpacket(
		frame_ptr, frame_len,
		&route_src, &route_dst, src, dst, &msg_id,
		&msg_type, &src_attrib, &dst_attrib,
		&packet_len, &packet_ptr);

	if(t_rpc_unzip(&pChainBson, &pRouterBson, &msg_body, &msg_len, msg_id, (s8 *)packet_ptr, packet_len) == dave_false)
	{
		SYNCLTRACE(60,1,"%s/%lx/%d/%d->%s/%lx/%d/%d msg_type:%d msg_id:%s packet_len:%d",
			src, route_src, thread_get_thread(route_src), thread_get_net(route_src),
			dst, route_dst, thread_get_thread(route_dst), thread_get_net(route_dst),
			msg_type, msgstr(msg_id), packet_len);

		dave_memset(msg_body, 0x00, msg_len);
	}

	if((src[0] != '\0') && (dst[0] != '\0') && (msg_id != MSGID_RESERVED) && (msg_len > 0))
	{
		sync_server_run_internal(pClient, src, dst, msg_id, msg_len, msg_body);
	}
	else
	{
		SYNCABNOR("find invalid parameter, src:%s dst:%s msg_id:%d msg_len:%d",
			src, dst, msg_id, msg_len);
	}
}

static void
_sync_server_rx_link_up_req(SyncClient *pClient, ub frame_len, u8 *frame_ptr)
{
	s8 verno[DAVE_VERNO_STR_LEN];
	s8 globally_identifier[DAVE_GLOBALLY_IDENTIFIER_LEN];
	u8 ip[16];
	u16 port;

	sync_link_unpacket(
		frame_ptr, frame_len,
		verno, sizeof(verno),
		ip, &port,
		globally_identifier, sizeof(globally_identifier));

	if((pClient->verno[0] != '\0') && (dave_strcmp(pClient->verno, verno) == dave_false))
	{
		SYNCLOG("verno mismatch:%s/%s!", pClient->verno, verno);
	}
	if((pClient->globally_identifier[0] != '\0')
		&& (globally_identifier[0] != '\0')
		&& (dave_strcmp(pClient->globally_identifier, globally_identifier) == dave_false))
	{
		SYNCLOG("globally_identifier mismatch:%s/%s!", pClient->globally_identifier, globally_identifier);
	}

	/*
	 * 如果实际来源地IP有效，以实际来源地IP为准，
	 * 而不使用客户端报告的IP。
	 */

	if(((pClient->NetInfo.addr.ip.ip_addr[0] == 127)
		&& (pClient->NetInfo.addr.ip.ip_addr[1] == 0)
		&& (pClient->NetInfo.addr.ip.ip_addr[2] == 0)
		&& (pClient->NetInfo.addr.ip.ip_addr[3] == 1))
		||
		((pClient->NetInfo.addr.ip.ip_addr[0] == 0)
		&& (pClient->NetInfo.addr.ip.ip_addr[1] == 0)
		&& (pClient->NetInfo.addr.ip.ip_addr[2] == 0)
		&& (pClient->NetInfo.addr.ip.ip_addr[3] == 0)))
	{
		dave_memcpy(pClient->link_ip, ip, sizeof(pClient->link_ip));
	}
	else
	{
		dave_memcpy(pClient->link_ip, pClient->NetInfo.addr.ip.ip_addr, sizeof(pClient->link_ip));		
	}

	pClient->link_port = port;

	SYNCTRACE("%s %s link:%s",
		pClient->verno,
		ipv4str(pClient->NetInfo.addr.ip.ip_addr, pClient->NetInfo.port),
		ipv4str2(pClient->link_ip, pClient->link_port));

	sync_server_tx_link_up_rsp(pClient);

	_sync_server_rx_snd_events(SyncServerEvents_link_up, pClient);
}

static void
_sync_server_rx_link_up_rsp(SyncClient *pClient, ub frame_len, u8 *frame_ptr)
{

}

static void
_sync_server_rx_link_down_req(SyncClient *pClient, ub frame_len, u8 *frame_ptr)
{
	s8 verno[DAVE_VERNO_STR_LEN];
	s8 globally_identifier[DAVE_GLOBALLY_IDENTIFIER_LEN];
	u8 ip[16];
	u16 port;

	sync_link_unpacket(
		frame_ptr, frame_len,
		verno, sizeof(verno),
		ip, &port,
		globally_identifier, sizeof(globally_identifier));

	if((pClient->verno[0] != '\0') && (dave_strcmp(pClient->verno, verno) == dave_false))
	{
		SYNCLOG("verno mismatch:%s/%s!", pClient->verno, verno);
	}
	if((pClient->link_port != 0) && (pClient->link_port != port))
	{
		SYNCLOG("port mismatch:%d/%d! verno:%s", pClient->link_port, port, verno);
	}
	if((pClient->globally_identifier[0] != '\0')
		&& (globally_identifier[0] != '\0')
		&& (dave_strcmp(pClient->globally_identifier, globally_identifier) == dave_false))
	{
		SYNCLOG("globally_identifier mismatch:%s/%s!", pClient->globally_identifier, globally_identifier);
	}

	SYNCTRACE("%s %s link:%s",
		pClient->verno,
		ipv4str(pClient->NetInfo.addr.ip.ip_addr, pClient->NetInfo.port),
		ipv4str2(pClient->link_ip, pClient->link_port));

	sync_server_tx_link_down_rsp(pClient);

	_sync_server_rx_snd_events(SyncServerEvents_link_down, pClient);
}

static void
_sync_server_rx_link_down_rsp(SyncClient *pClient, ub frame_len, u8 *frame_ptr)
{

}

static void
_sync_server_rx_rpcver_req(SyncClient *pClient, ub frame_len, u8 *frame_ptr)
{
	sb rpc_version;

	sync_rpcver_unpacket(frame_ptr, frame_len, &rpc_version);

	/*
	 * RPC从第3版本才有此版本同步的功能。
	 */
	if(rpc_version >= 3)
	{
		pClient->rpc_version = rpc_version;
	}

	sync_server_tx_rpcver_rsp(pClient);
}

static void
_sync_server_rx_rpcver_rsp(SyncClient *pClient, ub frame_len, u8 *frame_ptr)
{
	sb rpc_version;

	sync_rpcver_unpacket(frame_ptr, frame_len, &rpc_version);

	if(rpc_version >= 3)
	{
		pClient->rpc_version = rpc_version;
	}
}

static void
_sync_server_rx(SyncClient *pClient, ORDER_CODE order_id, ub frame_len, u8 *frame_ptr)
{
	sync_lock();
	pClient->left_timer = SYNC_CLIENT_LEFT_MAX;
	pClient->recv_data_counter ++;
	sync_unlock();

	SYNCDEBUG("socket:%d order_id:%x", pClient->client_socket, order_id);

	switch(order_id)
	{
		case ORDER_CODE_MY_VERNO:
				_sync_server_rx_verno(pClient, frame_len, frame_ptr);
			break;
		case ORDER_CODE_HEARTBEAT_REQ:
				_sync_server_rx_heartbeat_req(pClient, frame_len, frame_ptr);
			break;
		case ORDER_CODE_HEARTBEAT_RSP:
			break;
		case ORDER_CODE_SYNC_THREAD_NAME_REQ:
				_sync_server_rx_sync_thread_name_req(pClient, frame_len, frame_ptr);
			break;
		case ORDER_CODE_ADD_REMOTE_THREAD_RSP:
				_sync_server_rx_add_remote_thread_rsp(pClient, frame_len, frame_ptr);
			break;
		case ORDER_CODE_DEL_REMOTE_THREAD_RSP:
				_sync_server_rx_del_remote_thread_rsp(pClient, frame_len, frame_ptr);
			break;
		case ORDER_CODE_RUN_THREAD_MSG_REQ:
				_sync_server_rx_run_thread_msg_req(pClient, frame_len, frame_ptr, dave_false);
			break;
		case ORDER_CODE_TEST_RUN_THREAD_MSG_REQ:
				_sync_server_rx_test_run_thread_msg_req(pClient, frame_len, frame_ptr);
			break;
		case ORDER_CODE_RUN_INTERNAL_MSG_REQ:
				_sync_server_rx_run_internal_msg_req(pClient, frame_len, frame_ptr);
			break;
		case ORDER_CODE_RUN_INTERNAL_MSG_V2_REQ:
				_sync_server_rx_run_internal_msg_v2_req(pClient, frame_len, frame_ptr);
			break;
		case ORDER_CODE_RUN_INTERNAL_MSG_RSP:
			break;
		case ORDER_CODE_LINK_UP_REQ:
				_sync_server_rx_link_up_req(pClient, frame_len, frame_ptr);
			break;
		case ORDER_CODE_LINK_UP_RSP:
				_sync_server_rx_link_up_rsp(pClient, frame_len, frame_ptr);
			break;
		case ORDER_CODE_LINK_DOWN_REQ:
				_sync_server_rx_link_down_req(pClient, frame_len, frame_ptr);
			break;
		case ORDER_CODE_LINK_DOWN_RSP:
				_sync_server_rx_link_down_rsp(pClient, frame_len, frame_ptr);
			break;
		case ORDER_CODE_RPCVER_REQ:
				_sync_server_rx_rpcver_req(pClient, frame_len, frame_ptr);
			break;
		case ORDER_CODE_RPCVER_RSP:
				_sync_server_rx_rpcver_rsp(pClient, frame_len, frame_ptr);
			break;
		case ORDER_CODE_SERVICE_STATEMENT:
			break;
		default:
				SYNCLOG("can't process order_id:%x", order_id);
			break;
	}
}

static void
_sync_server_rx_input(void *param, s32 socket, IPBaseInfo *pInfo, FRAMETYPE ver_type, ORDER_CODE order_id, ub frame_len, u8 *frame_ptr)
{
	SyncClient *pClient = (SyncClient *)param;

	if(pClient == NULL)
	{
		pClient = sync_server_find_client(socket);
	}
	if((pClient == NULL) || (pClient->client_socket != socket))
	{
		SYNCLOG("the socket:%d can't find, %d,%d,%d,%x",
			socket, order_id, ver_type, frame_len, pClient);
		return;
	}

	_sync_server_rx(pClient, order_id, frame_len, frame_ptr);
}

static void
_sync_server_rx_event_process(SyncClient *pClient, SocketRawEvent *pEvent)
{
	RetCode ret = RetCode_Parameter_conflicts;

	if(pClient->client_socket != INVALID_SOCKET_ID)
	{
		ret = rxtx_event(pEvent, _sync_server_rx_input, pClient);
	}

	if(ret != RetCode_OK)
	{
		SYNCTRACE("%s socket:%d/%d/%s event:%d domain:%d type:%d error:%s",
			pClient->verno,
			pEvent->socket, pEvent->os_socket,
			ipv4str(pClient->NetInfo.addr.ip.ip_addr, pClient->NetInfo.port),
			pEvent->event,
			pEvent->NetInfo.domain, pEvent->NetInfo.type,
			retstr(ret));

		if(ret != RetCode_Invalid_data)
		{
			_sync_server_rx_disconnect(pEvent->socket);
		}
	}
}

static void
_sync_server_rx_version_process(void *ptr)
{
	SyncClient *pClient = (SyncClient *)ptr;
	SyncClient *pConflictClient;

	sync_server_setup_flag(pClient);

	sync_server_my_verno_to_all_client(pClient);

	pConflictClient = sync_server_check_globally_identifier_conflict(pClient);
	if(pConflictClient != NULL)
	{
		SYNCABNOR("The client has conflict identifier, try disconnect! pClient:(%x %d/%s %s/%s %d%d%d) pConflictClient:(%x %d/%s %s/%s %d%d%d)",
			pClient,
			pClient->client_socket, ipv4str(pClient->NetInfo.addr.ip.ip_addr, pClient->NetInfo.port),
			pClient->globally_identifier, pClient->verno,
			pClient->ready_flag, pClient->blocks_flag, pClient->client_flag,
			pConflictClient,
			pConflictClient->client_socket, ipv4str2(pConflictClient->NetInfo.addr.ip.ip_addr, pConflictClient->NetInfo.port),
			pConflictClient->globally_identifier, pConflictClient->verno,
			pConflictClient->ready_flag, pConflictClient->blocks_flag, pConflictClient->client_flag);

		_sync_server_rx_disconnect(pConflictClient->client_socket);
	}
}

// =====================================================================

void
sync_server_rx_event(SyncClient *pClient, SocketRawEvent *pEvent)
{
	SAFECODEv1(pClient->opt_pv, _sync_server_rx_event_process(pClient, pEvent););
}

void
sync_server_rx_version(SyncClient *pClient)
{
	SAFECODEv1(pClient->opt_pv, _sync_server_rx_version_process(pClient););
}

void
sync_server_rx_link_up(SyncClient *pClient)
{
	SAFECODEv1(pClient->opt_pv, sync_server_sync_link(pClient, dave_true););
}

void
sync_server_rx_link_down(SyncClient *pClient)
{
	SAFECODEv1(pClient->opt_pv, sync_server_sync_link(pClient, dave_false););
}

#endif

