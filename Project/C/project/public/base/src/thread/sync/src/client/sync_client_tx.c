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
#include "base_rxtx.h"
#include "base_tools.h"
#include "thread_tools.h"
#include "thread_chain.h"
#include "sync_base_package.h"
#include "sync_param.h"
#include "sync_client_tools.h"
#include "sync_client_data.h"
#include "sync_client_tx.h"
#include "sync_client_queue.h"
#include "sync_client_internal_buffer.h"
#include "sync_test.h"
#include "sync_lock.h"
#include "sync_log.h"

static inline dave_bool
_sync_client_tx(SyncServer *pServer, ORDER_CODE order_id, MBUF *data)
{
	dave_bool ret;

	if(pServer == NULL)
	{
		pServer = sync_client_data_sync_server();
	}

	if(data == NULL)
	{
		SYNCABNOR("invalid data!");
		return dave_false;
	}

	if(pServer->server_socket == INVALID_SOCKET_ID)
	{
		SYNCLTRACE(60,1,"socket disconnect, the order_id:%x send failed! verno:%s socket:%d",
			order_id, pServer->verno, pServer->server_socket);
		dave_mfree(data);
		return dave_false;
	}

	sync_lock();
	pServer->send_data_counter ++;
	sync_unlock();

	SYNCDEBUG("server_socket:%d order_id:%x", pServer->server_socket, order_id);

	ret = rxtx_write(pServer->server_socket, order_id, data);
	if(ret == dave_false)
	{
		SYNCLOG("server_socket:%d verno:%s write_failed!",
			pServer->server_socket, pServer->verno);
	}

	return ret;
}

static inline dave_bool
_sync_client_qtx(
	SyncServer *pServer,
	ThreadId route_src, ThreadId route_dst,
	s8 *src, s8 *dst,
	ub msg_id,
	MBUF *data)
{
	return sync_client_queue_upload(pServer, route_src, route_dst, src, dst, msg_id, data);
}

static dave_bool
_sync_client_tx_run_internal_msg_req(
	SyncServer *pServer,
	ub msg_id, ub msg_len, void *msg_body)
{
	MBUF *zip_body;
	ub snd_max = SYNC_STACK_HEAD_MAX_LEN + msg_len;
	MBUF *snd_buffer;

	SYNCTRACE("msg_id:%s msg_len:%d", msgstr(msg_id), msg_len);

	zip_body = t_rpc_zip(NULL, NULL, msg_id, msg_body, msg_len);
	if(zip_body == NULL)
	{
		SYNCLOG("msg_id:%d msg_len:%d zip", msg_id, msg_len);
		return dave_false;
	}

	snd_buffer = dave_mmalloc(snd_max);

	snd_buffer->tot_len = snd_buffer->len = sync_msg_packet(
		dave_mptr(snd_buffer), snd_max,
		INVALID_THREAD_ID, INVALID_THREAD_ID, (s8 *)SYNC_SERVER_THREAD_NAME, (s8 *)SYNC_CLIENT_THREAD_NAME, msg_id,
		BaseMsgType_Unicast, LOCAL_TASK_ATTRIB, LOCAL_TASK_ATTRIB,
		base_mlen(zip_body), NULL);

	dave_mchain(snd_buffer, zip_body);

	return _sync_client_tx(pServer, ORDER_CODE_RUN_INTERNAL_MSG_REQ, snd_buffer);
}

// =====================================================================

void
sync_client_tx_init(void)
{
	t_rpc_init();
}

void
sync_client_tx_exit(void)
{
	t_rpc_exit();
}

dave_bool
sync_client_tx_run_internal_msg_req(
	SyncServer *pServer,
	ub msg_id, ub msg_len, void *msg_body,
	dave_bool pop_flag)
{
	dave_bool ret;

	if(pServer == NULL)
	{
		SYNCLOG("send msg:%s failed! pServer is NULL.", msgstr(msg_id));
		return dave_false;
	}

	SYNCTRACE("server_cnt:%d msg_id:%s", pServer->server_cnt, msgstr(msg_id));

	if(pServer->server_cnt == dave_true)
	{
		ret = _sync_client_tx_run_internal_msg_req(pServer, msg_id, msg_len, msg_body);
	}
	else
	{
		ret = dave_false;
	}

	if((ret == dave_false) && (pop_flag == dave_false))
	{
		sync_client_internal_buffer_push(pServer, msg_id, msg_len, msg_body);

		return dave_true;
	}

	return ret;
}

void
sync_client_tx_my_verno(SyncServer *pServer)
{
	s8 host_name[DAVE_NORMAL_NAME_LEN];
	MBUF *snd_buffer;
	u8 *snd_ptr;
	ub snd_max = 2048;
	ub snd_index = 0;

	dave_os_load_host_name(host_name, sizeof(host_name));

	snd_buffer = dave_mmalloc(snd_max);
	snd_ptr = dave_mptr(snd_buffer);

	snd_index += sync_str_packet(&snd_ptr[snd_index], snd_max-snd_index, dave_verno());
	snd_index += sync_str_packet(&snd_ptr[snd_index], snd_max-snd_index, globally_identifier());
	snd_index += sync_ip_packet(&snd_ptr[snd_index], snd_max-snd_index, pServer->cfg_server_ip);
	snd_index += sync_str_packet(&snd_ptr[snd_index], snd_max-snd_index, host_name);

	snd_buffer->len = snd_buffer->tot_len = snd_index;

	_sync_client_tx(pServer, ORDER_CODE_MY_VERNO, snd_buffer);
}

void
sync_client_tx_heartbeat(SyncServer *pServer, dave_bool req_flag)
{
	MBUF *snd_buffer;

	SYNCDEBUG("%d/%s", pServer->server_socket, pServer->verno);

	snd_buffer = sync_heartbeat_packet(pServer->recv_data_counter, pServer->send_data_counter, t_time_get_date(NULL));

	if(req_flag == dave_true)
	{
		_sync_client_tx(pServer, ORDER_CODE_HEARTBEAT_REQ, snd_buffer);
	}
	else
	{
		_sync_client_tx(pServer, ORDER_CODE_HEARTBEAT_RSP, snd_buffer);
	}
}

dave_bool
sync_client_tx_sync_thread_name_req(SyncServer *pServer, s8 *thread_name, ub thread_index)
{
	MBUF *snd_buffer;

	snd_buffer = sync_thread_name_packet(dave_verno(), globally_identifier(), thread_name, thread_index);

	return _sync_client_tx(pServer, ORDER_CODE_SYNC_THREAD_NAME_REQ, snd_buffer);
}

dave_bool
sync_client_tx_sync_thread_name_rsp(SyncServer *pServer, s8 *thread_name, ub thread_index)
{
	MBUF *snd_buffer;

	snd_buffer = sync_thread_name_packet(dave_verno(), globally_identifier(), thread_name, thread_index);

	return _sync_client_tx(pServer, ORDER_CODE_SYNC_THREAD_NAME_RSP, snd_buffer);
}

void
sync_client_tx_run_thread_msg_rsp(SyncServer *pServer, s8 *src, s8 *dst, ub msg_id, RetCode ret)
{
	/*
	 * It's just a reminder message,
	 * you can stop sending this message "ORDER_CODE_RUN_THREAD_MSG_RSP" for the time being.
	 */

	if(ret != RetCode_OK)
	{
		SYNCLTRACE(60,1,"%s->%s %d ret:%s", src, dst, msg_id, retstr(ret));
	}

	sync_client_recv_statistics(pServer, src);
}

dave_bool
sync_client_tx_run_thread_msg_req(
	SyncServer *pServer,
	void *msg_chain, void *msg_router,
	ThreadId route_src, ThreadId route_dst,
	s8 *src, s8 *dst, ub msg_id,
	BaseMsgType msg_type, TaskAttribute src_attrib, TaskAttribute dst_attrib,
	ub msg_len, void *msg_body)
{
	MBUF *zip_body;
	MBUF *msg_head;

	msg_type = sync_client_queue_enable(pServer, route_src, route_dst, msg_id, msg_type);

	zip_body = t_rpc_zip(thread_chain_to_bson(msg_chain), thread_router_to_bson(msg_router), msg_id, msg_body, msg_len);
	if(zip_body == NULL)
	{
		SYNCLOG("%s/%lx/%d/%d->%s/%lx/%d/%d msg_type:%d msg_id:%d msg_len:%d",
			src, route_src, thread_get_thread(route_src), thread_get_net(route_src),
			dst, route_dst, thread_get_thread(route_dst), thread_get_net(route_dst),
			msg_type, msg_id, msg_len);

		return dave_false;
	}

	sync_client_detected_rpc_efficiency(msg_len, zip_body->len, msg_id);

	msg_head = dave_mmalloc(SYNC_STACK_HEAD_MAX_LEN);

	msg_head->len = msg_head->tot_len = sync_msg_packet(
		dave_mptr(msg_head), msg_head->len,
		route_src, route_dst, src, dst, msg_id,
		msg_type, src_attrib, dst_attrib,
		base_mlen(zip_body), NULL);

	dave_mchain(msg_head, zip_body);

	SYNCDEBUG("%s %s/%lx/%d/%d->%s/%lx/%d/%d id:%d len:%d/%d",
		pServer->verno,
		src, route_src, thread_get_thread(route_src), thread_get_net(route_src),
		dst, route_dst, thread_get_thread(route_dst), thread_get_net(route_dst),
		msg_id, msg_head->tot_len, zip_body->len);

	sync_client_send_statistics(pServer, dst);

	if(msg_type == BaseMsgType_Unicast_queue)
	{
		if(_sync_client_qtx(pServer, route_src, route_dst, src, dst, msg_id, msg_head) == dave_true)
		{
			return dave_true;
		}
	}

	return _sync_client_tx(pServer, ORDER_CODE_RUN_THREAD_MSG_REQ, msg_head);
}

dave_bool
sync_client_tx_test_run_thread_msg_req(SyncServer *pServer, s8 *my_name, s8 *other_name)
{
	SyncTestMsg test_data;
	MBUF *snd_buffer;
	ub snd_max = SYNC_STACK_HEAD_MAX_LEN + sizeof(SyncTestMsg);

	sync_client_test_data(&test_data);

	snd_buffer = dave_mmalloc(snd_max);

	snd_buffer->tot_len = snd_buffer->len = sync_msg_packet(
		dave_mptr(snd_buffer), snd_max,
		0, 0, my_name, other_name, MSGID_TEST,
		BaseMsgType_Unicast, LOCAL_TASK_ATTRIB, REMOTE_TASK_ATTRIB,
		sizeof(SyncTestMsg), &test_data);

	return _sync_client_tx(pServer, ORDER_CODE_TEST_RUN_THREAD_MSG_REQ, snd_buffer);
}

dave_bool
sync_client_tx_add_remote_thread_rsp(SyncServer *pServer, s8 *thread_name, sb thread_index)
{
	MBUF *snd_buffer;

	snd_buffer = sync_thread_name_packet(dave_verno(), globally_identifier(), thread_name, thread_index);

	return _sync_client_tx(pServer, ORDER_CODE_ADD_REMOTE_THREAD_RSP, snd_buffer);
}

dave_bool
sync_client_tx_del_remote_thread_rsp(SyncServer *pServer, s8 *thread_name, sb thread_index)
{
	MBUF *snd_buffer;

	snd_buffer = sync_thread_name_packet(dave_verno(), globally_identifier(), thread_name, thread_index);

	return _sync_client_tx(pServer, ORDER_CODE_DEL_REMOTE_THREAD_RSP, snd_buffer);
}

dave_bool
sync_client_tx_link_up_req(SyncServer *pServer, u8 ip[DAVE_IP_V6_ADDR_LEN], u16 port)
{
	MBUF *snd_buffer;

	snd_buffer = sync_link_packet(dave_verno(), ip, port, globally_identifier());

	return _sync_client_tx(pServer, ORDER_CODE_LINK_UP_REQ, snd_buffer);
}

dave_bool
sync_client_tx_link_up_rsp(SyncServer *pServer, s8 *verno, u8 *link_ip, u16 link_port)
{
	MBUF *snd_buffer;

	snd_buffer = sync_link_packet(verno, link_ip, link_port, globally_identifier());

	return _sync_client_tx(pServer, ORDER_CODE_LINK_UP_RSP, snd_buffer);
}

dave_bool
sync_client_tx_link_down_req(SyncServer *pServer, u8 ip[DAVE_IP_V6_ADDR_LEN], u16 port)
{
	MBUF *snd_buffer;

	snd_buffer = sync_link_packet(dave_verno(), ip, port, globally_identifier());

	return _sync_client_tx(pServer, ORDER_CODE_LINK_DOWN_REQ, snd_buffer);
}

dave_bool
sync_client_tx_link_down_rsp(SyncServer *pServer, s8 *verno, u8 *link_ip, u16 link_port)
{
	MBUF *snd_buffer;

	snd_buffer = sync_link_packet(verno, link_ip, link_port, globally_identifier());

	return _sync_client_tx(pServer, ORDER_CODE_LINK_DOWN_RSP, snd_buffer);
}

dave_bool
sync_client_tx_rpcver_req(SyncServer *pServer)
{
	MBUF *snd_buffer;

	snd_buffer = sync_rpcver_packet(3);

	return _sync_client_tx(pServer, ORDER_CODE_RPCVER_REQ, snd_buffer);
}

dave_bool
sync_client_tx_rpcver_rsp(SyncServer *pServer)
{
	MBUF *snd_buffer;

	snd_buffer = sync_rpcver_packet(3);

	return _sync_client_tx(pServer, ORDER_CODE_RPCVER_RSP, snd_buffer);
}

dave_bool
sync_client_tx_service_statement(SyncServer *pServer, s8 *service_statement)
{
	MBUF *snd_buffer = dave_mmalloc(256 + dave_strlen(service_statement));

	snd_buffer->len = snd_buffer->tot_len = sync_str_packet((u8 *)ms8(snd_buffer), mlen(snd_buffer), service_statement);

	return _sync_client_tx(pServer, ORDER_CODE_SERVICE_STATEMENT, snd_buffer);
}

#endif

