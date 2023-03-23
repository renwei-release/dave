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
#include "dave_os.h"
#include "dave_tools.h"
#include "base_rxtx.h"
#include "thread_tools.h"
#include "sync_client_param.h"
#include "sync_base_package.h"
#include "sync_param.h"
#include "sync_client_thread.h"
#include "sync_client_tools.h"
#include "sync_client_tx.h"
#include "sync_client_rx.h"
#include "sync_client_run.h"
#include "sync_client_data.h"
#include "sync_client_link.h"
#include "sync_client_ntp.h"
#include "sync_test.h"
#include "sync_lock.h"
#include "sync_log.h"

static inline void
_sync_client_thread_name_sync_booting(SyncServer *pServer, u8 *detect_my_ip)
{
	if(pServer->server_type == SyncServerType_sync_client)
	{
		sync_client_link_tell_sync_server(detect_my_ip);
	}

	if((pServer->sync_thread_name_index != 0)
		&& (pServer->sync_thread_name_index < pServer->sync_thread_name_number))
	{
		SYNCLOG("Synchronization task is not completed!<%d/%d> %s/%s",
			pServer->sync_thread_name_index, pServer->sync_thread_name_number,
			pServer->globally_identifier, pServer->verno);
	}

	pServer->sync_thread_name_number = sync_client_data_thread_local_reset();
	if(pServer->sync_thread_name_number > SYNC_THREAD_MAX)
	{
		SYNCABNOR("thread name number too big!<%d>", pServer->sync_thread_name_number);
		pServer->sync_thread_name_number = SYNC_THREAD_MAX;
	}
	pServer->sync_thread_name_index = 0;

	SYNCTRACE("%s/%s %d/%d",
		pServer->verno, pServer->globally_identifier,
		pServer->sync_thread_name_index, pServer->sync_thread_name_number);

	if(pServer->sync_thread_name_index < pServer->sync_thread_name_number)
	{
		if(sync_client_tx_sync_thread_name_req(
			pServer,
			sync_client_data_thread_local_name(pServer->sync_thread_name_index),
			(sb)pServer->sync_thread_name_index) == dave_false)
		{
			SYNCABNOR("send the thread:%s failed!", sync_client_data_thread_local_name(pServer->sync_thread_name_index));
		}
	}
	else
	{
		sync_client_tx_sync_thread_name_req(pServer, (s8 *)"", pServer->sync_thread_name_index);
	}
}

static inline void
_sync_client_rx_disconnect(s32 socket)
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

	id_msg(thread_id(SOCKET_THREAD_NAME), SOCKET_DISCONNECT_REQ, pReq);
}

static inline void
_sync_client_rx_verno(SyncServer *pServer, ub frame_len, u8 *frame)
{
	ub frame_index = 0;
	u8 detect_my_ip[16];

	frame_index += sync_str_unpacket(&frame[frame_index], frame_len-frame_index, pServer->verno, sizeof(pServer->verno));
	frame_index += sync_str_unpacket(&frame[frame_index], frame_len-frame_index, pServer->globally_identifier, sizeof(pServer->globally_identifier));
	frame_index += sync_ip_unpacket(&frame[frame_index], frame_len-frame_index, detect_my_ip);
	if(frame_index < frame_len)
	{
		sync_str_unpacket(&frame[frame_index], frame_len-frame_index, pServer->host_name, sizeof(pServer->host_name));
	}

	pServer->work_start_second = dave_os_time_s();

	SYNCTRACE("server:%x %s/%s (%d%d%d%d)",
		pServer,
		pServer->globally_identifier, pServer->verno,
		pServer->server_connecting, pServer->server_cnt,
		pServer->server_booting, pServer->server_ready);

	_sync_client_thread_name_sync_booting(pServer, detect_my_ip);
}

static inline void
_sync_client_rx_heartbeat_req(SyncServer *pServer, ub frame_len, u8 *frame)
{
	ub recv_data_counter, send_data_counter;

	sync_heartbeat_unpacket(frame, frame_len, &recv_data_counter, &send_data_counter, NULL);

	SYNCDEBUG("%d/%s", pServer->server_socket, pServer->verno);

	sync_client_tx_heartbeat(pServer, dave_false);
}

static inline void
_sync_client_rx_heartbeat_rsp(SyncServer *pServer, ub frame_len, u8 *frame)
{
	ub recv_data_counter, send_data_counter;
	DateStruct date = t_time_get_date(NULL);

	sync_heartbeat_unpacket(frame, frame_len, &recv_data_counter, &send_data_counter, &date);

	sync_client_ntp(pServer, date);
}

static inline void
_sync_client_rx_sync_thread_name_req(SyncServer *pServer, ub frame_len, u8 *frame)
{
	s8 thread_name[SYNC_THREAD_NAME_LEN];
	sb thread_index;

	sync_thread_name_unpacket(frame, frame_len, pServer->verno, pServer->globally_identifier, thread_name, &thread_index);

	if(thread_name[0] != '\0')
	{
		SYNCTRACE("%s/%s thread:%s", pServer->globally_identifier, pServer->verno, thread_name);

		sync_client_data_thread_add(pServer, thread_name);

		sync_client_tx_sync_thread_name_rsp(pServer, thread_name, thread_index);

		/*
		 * 由SYNC以外的服务同步远端线程信息，发现同步开始后，才可以清除server_booting标记。
		 */
	
		pServer->server_booting = dave_false;
	}
	else
	{
		SYNCTRACE("the client socket:%d verno:%s sync done!",
			pServer->server_socket,
			pServer->verno);

		/*
		 * 由SYNC以外的服务同步远端线程信息，发现同步完成后，才设置server_ready标记。
		 */

		pServer->server_booting = dave_false;
		pServer->server_ready = dave_true;
	}
}

static inline void
_sync_client_rx_sync_thread_name_rsp(SyncServer *pServer, ub frame_len, u8 *frame)
{
	s8 verno[DAVE_VERNO_STR_LEN];
	s8 globally_identifier[DAVE_GLOBALLY_IDENTIFIER_LEN];
	s8 thread_name[SYNC_THREAD_NAME_LEN];
	sb thread_index;

	sync_thread_name_unpacket(frame, frame_len, verno, globally_identifier, thread_name, &thread_index);
	if(thread_index >= 0)
	{
		pServer->sync_thread_name_index = thread_index;
	}

	SYNCDEBUG("thread:%s %d/%d", thread_name, pServer->sync_thread_name_index, pServer->sync_thread_name_number);

	if(pServer->sync_thread_name_index >= pServer->sync_thread_name_number)
	{
		sync_client_tx_sync_thread_name_req(pServer, (s8 *)"", thread_index);
		return;
	}

	if(dave_strcmp(sync_client_data_thread_local_name(pServer->sync_thread_name_index), thread_name) == dave_false)
	{
		SYNCLOG("thread name mismatch! index:%d/%d %s/%s %s/%s",
			pServer->sync_thread_name_index, pServer->sync_thread_name_number,
			sync_client_data_thread_local_name(pServer->sync_thread_name_index), thread_name,
			pServer->globally_identifier, pServer->verno);

		pServer->sync_thread_name_number = sync_client_data_thread_local_reset();
		pServer->sync_thread_name_index = sync_client_data_thread_local_index(thread_name) + 1;
		if(pServer->sync_thread_name_index >= SYNC_THREAD_MAX)
		{
			pServer->sync_thread_name_index = 0;
		}
	}
	else
	{
		pServer->sync_thread_name_index ++;
	}

	if(pServer->sync_thread_name_index < pServer->sync_thread_name_number)
	{
		if(sync_client_tx_sync_thread_name_req(
			pServer,
			sync_client_data_thread_local_name(pServer->sync_thread_name_index),
			(sb)pServer->sync_thread_name_index) == dave_false)
		{
			SYNCABNOR("send the thread:%s failed!",
				sync_client_data_thread_local_name(pServer->sync_thread_name_index));
		}
	}
	else
	{
		SYNCDEBUG("sync my thread name done!");

		sync_client_tx_sync_thread_name_req(pServer, (s8 *)"", thread_index);
	}
}

static inline void
_sync_client_rx_run_thread_msg_rsp(SyncServer *pServer, ub frame_len, u8 *frame)
{
	/*
	 * No need to process this message.
	 */
}

static inline void
_sync_client_rx_run_thread_msg_req(SyncServer *pServer, ub frame_len, u8 *frame)
{
	sync_client_run_thread(pServer, frame_len, frame);
}

static inline void
_sync_client_rx_test_run_thread_msg_req(SyncServer *pServer, ub frame_len, u8 *frame)
{
	s8 my_name[SYNC_THREAD_NAME_LEN];
	s8 other_name[SYNC_THREAD_NAME_LEN];
	ub msg_id = MSGID_RESERVED;
	ub msg_len = SYNC_STACK_HEAD_MAX_LEN + sizeof(SyncTestMsg);
	u8 *msg_body;
	dave_bool match;

	sync_msg_unpacket(
		frame, frame_len,
		NULL, NULL, other_name, my_name, &msg_id,
		NULL, NULL, NULL,
		&msg_len, &msg_body);

	SYNCDEBUG("%s->%s", other_name, my_name);

	match = dave_true;

	if(dave_strcmp(dave_verno(), my_name) == dave_false)
	{
		SYNCABNOR("mismatch my name:%s,%s", dave_verno(), my_name);
		match = dave_false;
	}

	if(msg_id != MSGID_TEST)
	{
		SYNCABNOR("mismatch msg_id:%d", msg_id);
		match = dave_false;
	}

	if(msg_len != sizeof(SyncTestMsg))
	{
		SYNCABNOR("mismatch msg len:%d,%d", msg_len, sizeof(SyncTestMsg));
		match = dave_false;
	}

	if(sync_client_test_data_valid((SyncTestMsg *)(msg_body)) == dave_false)
	{
		SYNCABNOR("find invalid msg body!");
		match = dave_false;
	}

	if(match == dave_true)
	{
		SYNCDEBUG("I match the byte order of the %s", other_name);
	}
}

static inline void
_sync_client_rx_run_internal_msg_req(SyncServer *pServer, ub frame_len, u8 *frame)
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

	sync_msg_unpacket(
		frame, frame_len,
		&route_src, &route_dst, src, dst, &msg_id,
		&msg_type, &src_attrib, &dst_attrib,
		&packet_len, &packet_ptr);

	msg_len = packet_len;
	if(msg_len > 0)
	{
		msg_body = packet_ptr;
	}

	if((src[0] != '\0') && (dst[0] != '\0') && (msg_id != MSGID_RESERVED) && (msg_len > 0))
	{
		sync_client_run_internal(src, dst, msg_id, msg_len, msg_body);
	}
	else
	{
		SYNCABNOR("find invalid parameter, src:%s dst:%s msg_id:%d msg_len:%d",
			src, dst, msg_id, msg_len);
	}
}

static inline void
_sync_client_rx_run_internal_msg_v2_req(SyncServer *pServer, ub frame_len, u8 *frame)
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
		frame, frame_len,
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
		sync_client_run_internal(src, dst, msg_id, msg_len, msg_body);
	}
	else
	{
		SYNCABNOR("find invalid parameter, src:%s dst:%s msg_id:%s msg_len:%d",
			src, dst, msgstr(msg_id), msg_len);
	}
}

static inline void
_sync_client_rx_add_remote_thread_req(SyncServer *pServer, ub frame_len, u8 *frame)
{
	s8 verno[DAVE_VERNO_STR_LEN];
	s8 globally_identifier[DAVE_GLOBALLY_IDENTIFIER_LEN];
	s8 thread_name[SYNC_THREAD_NAME_LEN];
	sb thread_index;

	sync_thread_name_unpacket(frame, frame_len, verno, globally_identifier, thread_name, &thread_index);

	SYNCTRACE("%s/%s thread:%s", pServer->globally_identifier, pServer->verno, thread_name);

	if(thread_name[0] != '\0')
	{
		sync_client_data_thread_add(pServer, thread_name);

		sync_client_tx_add_remote_thread_rsp(pServer, thread_name, thread_index);
	}

	/*
	 * 由SYNC服务同步远端线程信息，只要发现有同步，就设置server_ready标记。
	 * 同时复位server_booting标记。
	 * server_booting标记也可以在_sync_client_rx_sync_thread_name_rsp复位。
	 */
	pServer->server_booting = dave_false;
	pServer->server_ready = dave_true;
}

static inline void
_sync_client_rx_del_remote_thread_req(SyncServer *pServer, ub frame_len, u8 *frame)
{
	s8 verno[DAVE_VERNO_STR_LEN];
	s8 globally_identifier[DAVE_GLOBALLY_IDENTIFIER_LEN];
	s8 thread_name[SYNC_THREAD_NAME_LEN];
	sb thread_index;

	sync_thread_name_unpacket(frame, frame_len, verno, globally_identifier, thread_name, &thread_index);

	SYNCTRACE("verno:%s thread:%s", verno, thread_name);

	if(thread_name[0] != '\0')
	{
		sync_client_data_thread_del(pServer, thread_name);
	}

	sync_client_tx_del_remote_thread_rsp(pServer, thread_name, thread_index);
}

static inline void
_sync_client_rx_module_verno(SyncServer *pServer, ub frame_len, u8 *frame)
{
	s8 verno[DAVE_VERNO_STR_LEN];

	sync_str_unpacket(frame, frame_len, verno, DAVE_VERNO_STR_LEN);

	if(verno[0] == '\0')
	{
		SYNCABNOR("empty verno!");
	}

	SYNCTRACE("module verno:%s", verno);

	sync_client_tx_test_run_thread_msg_req(pServer, dave_verno(), verno);
}

static inline void
_sync_client_rx_link_up_req(SyncServer *pServer, ub frame_len, u8 *frame)
{
	s8 verno[DAVE_VERNO_STR_LEN];
	s8 globally_identifier[DAVE_GLOBALLY_IDENTIFIER_LEN];
	u8 ip[16];
	u16 port;

	sync_link_unpacket(frame, frame_len, verno, sizeof(verno), ip, &port, globally_identifier, sizeof(globally_identifier));

	sync_client_link_up(verno, globally_identifier, ip, port);

	sync_client_tx_link_up_rsp(pServer, verno, ip, port);
}

static inline void
_sync_client_rx_link_up_rsp(SyncServer *pServer, ub frame_len, u8 *frame)
{

}

static inline void
_sync_client_rx_link_down_req(SyncServer *pServer, ub frame_len, u8 *frame)
{
	s8 verno[DAVE_VERNO_STR_LEN];
	s8 globally_identifier[DAVE_GLOBALLY_IDENTIFIER_LEN];
	u8 ip[16];
	u16 port;

	sync_link_unpacket(frame, frame_len, verno, sizeof(verno), ip, &port, globally_identifier, sizeof(globally_identifier));

	sync_client_link_down(verno, globally_identifier, ip, port);

	sync_client_tx_link_down_rsp(pServer, verno, ip, port);
}

static inline void
_sync_client_rx_link_down_rsp(SyncServer *pServer, ub frame_len, u8 *frame)
{

}

static inline void
_sync_client_rx_rpcver_req(SyncServer *pServer, ub frame_len, u8 *frame)
{
	sync_rpcver_unpacket(frame, frame_len, &(pServer->rpc_version));

	sync_client_tx_rpcver_rsp(pServer);
}

static inline void
_sync_client_rx_rpcver_rsp(SyncServer *pServer, ub frame_len, u8 *frame)
{
	sync_rpcver_unpacket(frame, frame_len, &(pServer->rpc_version));
}

static inline void
_sync_client_rx_record_processing(SyncServer *pServer)
{
	sync_lock();
	pServer->left_timer = SYNC_SERVER_LEFT_MAX;
	pServer->recv_data_counter ++;
	sync_unlock();
}

static inline void
_sync_client_rx_order(SyncServer *pServer, ORDER_CODE order_id, ub frame_len, u8 *frame_ptr)
{
	switch(order_id)
	{
		case ORDER_CODE_MY_VERNO:
				_sync_client_rx_verno(pServer, frame_len, frame_ptr);
			break;
		case ORDER_CODE_HEARTBEAT_REQ:
				_sync_client_rx_heartbeat_req(pServer, frame_len, frame_ptr);
			break;
		case ORDER_CODE_HEARTBEAT_RSP:
				_sync_client_rx_heartbeat_rsp(pServer, frame_len, frame_ptr);
			break;
		case ORDER_CODE_MODULE_VERNO:
				_sync_client_rx_module_verno(pServer, frame_len, frame_ptr);
			break;
		case ORDER_CODE_SYNC_THREAD_NAME_REQ:
				_sync_client_rx_sync_thread_name_req(pServer, frame_len, frame_ptr);
			break;
		case ORDER_CODE_SYNC_THREAD_NAME_RSP:
				_sync_client_rx_sync_thread_name_rsp(pServer, frame_len, frame_ptr);
			break;
		case ORDER_CODE_ADD_REMOTE_THREAD_REQ:
				_sync_client_rx_add_remote_thread_req(pServer, frame_len, frame_ptr);
			break;
		case ORDER_CODE_DEL_REMOTE_THREAD_REQ:
				_sync_client_rx_del_remote_thread_req(pServer, frame_len, frame_ptr);
			break;
		case ORDER_CODE_RUN_THREAD_MSG_REQ:
				_sync_client_rx_run_thread_msg_req(pServer, frame_len, frame_ptr);
			break;
		case ORDER_CODE_RUN_THREAD_MSG_RSP:
				_sync_client_rx_run_thread_msg_rsp(pServer, frame_len, frame_ptr);
			break;
		case ORDER_CODE_TEST_RUN_THREAD_MSG_REQ:
				_sync_client_rx_test_run_thread_msg_req(pServer, frame_len, frame_ptr);
			break;
		case ORDER_CODE_RUN_INTERNAL_MSG_REQ:
				_sync_client_rx_run_internal_msg_req(pServer, frame_len, frame_ptr);
			break;
		case ORDER_CODE_RUN_INTERNAL_MSG_V2_REQ:
				_sync_client_rx_run_internal_msg_v2_req(pServer, frame_len, frame_ptr);
			break;
		case ORDER_CODE_LINK_UP_REQ:
				_sync_client_rx_link_up_req(pServer, frame_len, frame_ptr);
			break;
		case ORDER_CODE_LINK_UP_RSP:
				_sync_client_rx_link_up_rsp(pServer, frame_len, frame_ptr);
			break;
		case ORDER_CODE_LINK_DOWN_REQ:
				_sync_client_rx_link_down_req(pServer, frame_len, frame_ptr);
			break;
		case ORDER_CODE_LINK_DOWN_RSP:
				_sync_client_rx_link_down_rsp(pServer, frame_len, frame_ptr);
			break;
		case ORDER_CODE_RPCVER_REQ:
				_sync_client_rx_rpcver_req(pServer, frame_len, frame_ptr);
			break;
		case ORDER_CODE_RPCVER_RSP:
				_sync_client_rx_rpcver_rsp(pServer, frame_len, frame_ptr);
			break;
		default:
			break;
	}
}

static void
_sync_client_rx(void *param, s32 socket, IPBaseInfo *pInfo, FRAMETYPE ver_type, ORDER_CODE order_id, ub frame_len, u8 *frame_ptr)
{
	SyncServer *pServer = (SyncServer *)param;

	if((pServer == NULL) || (socket != pServer->server_socket))
	{
		SYNCLOG("the socket:%d/%s can't find, order:%x ver:%d frame_len:%d pServer:%x",
			socket, ipv4str(pInfo->src_ip, pInfo->dst_port),
			order_id, ver_type, frame_len,
			pServer);
		return;
	}

	_sync_client_rx_record_processing(pServer);

	SYNCDEBUG("socket:%d order_id:%x %s left_timer:%d",
		socket, order_id, pServer->verno, pServer->left_timer);

	_sync_client_rx_order(pServer, order_id, frame_len, frame_ptr);
}

// =====================================================================

void
sync_client_rx(SyncServer *pServer, SocketRawEvent *pEvent)
{
	RetCode ret;

	ret = rxtx_event(pEvent, _sync_client_rx, pServer);
	if(ret != RetCode_OK)
	{
		SYNCTRACE("socket:%d/%d event error:%s",
			pEvent->socket, pEvent->os_socket,
			retstr(ret));

		_sync_client_rx_disconnect(pEvent->socket);	
	}
}

#endif

