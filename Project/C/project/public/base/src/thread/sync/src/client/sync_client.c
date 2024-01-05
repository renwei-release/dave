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
#include "sync_base_package.h"
#include "sync_param.h"
#include "sync_cfg.h"
#include "sync_client_param.h"
#include "sync_client_data.h"
#include "sync_client_thread.h"
#include "sync_client_link.h"
#include "sync_client_tools.h"
#include "sync_client_tx.h"
#include "sync_client_rx.h"
#include "sync_client_run.h"
#include "sync_client_msg_buffer.h"
#include "sync_client_internal_buffer.h"
#include "sync_client_info.h"
#include "sync_client_route.h"
#include "sync_client_queue.h"
#include "sync_client_app_tx.h"
#include "sync_client_service_statement.h"
#include "sync_test.h"
#include "sync_lock.h"
#include "sync_log.h"

static TLock _sync_client_system_lock;
static ThreadId _sync_client_thread = INVALID_THREAD_ID;
static ThreadId _socket_thread = INVALID_THREAD_ID;
extern TLock _sync_client_data_pv;
static TIMERID _sync_client_reconnect_syncs_logic_timer = INVALID_TIMER_ID;
static TIMERID _sync_client_timer = INVALID_TIMER_ID;
static dave_bool _sync_client_reconnect_syncs_logic_flag = dave_false;

static void _sync_client_reconnect_syncs_action(void);
static void _sync_client_safe_reconnect_syncs_logic_timer_out(TIMERID timer_id, ub thread_index);

static void
_sync_client_system_mount(ThreadId src, SystemMount *pMount)
{
	SystemMount *pmount;

	if(src == _sync_client_thread)
	{
		/*
		 * 收到SYNC服务的内部挂载消息，
		 * 把这个消息广播给本地除自己外的其他线程。
		 */
		pmount = thread_msg(pmount);

		dave_memcpy(pmount, pMount, sizeof(SystemMount));

		SYNCTRACE("socker:%d verno:%s", pmount->socket, pmount->verno);

		broadcast_local_no_me(MSGID_SYSTEM_MOUNT, pmount);
	}

	sync_client_link_start();
}

static void
_sync_client_system_decoupling(ThreadId src, SystemDecoupling *pDecoupling)
{
	SystemDecoupling *pdecoupling;

	sync_client_link_stop();

	if(src == _sync_client_thread)
	{
		pdecoupling = thread_msg(pdecoupling);

		dave_memcpy(pdecoupling, pDecoupling, sizeof(SystemDecoupling));

		SYNCTRACE("socker:%d verno:%s", pdecoupling->socket, pdecoupling->verno);

		broadcast_local_no_me(MSGID_SYSTEM_DECOUPLING, pdecoupling);
	}
}

static void
_sync_client_disconnect_rsp(SocketDisconnectRsp *pRsp)
{
	SyncServer *pServer = (SyncServer *)(pRsp->ptr);

	if(pServer != NULL)
	{
		if(pServer->server_socket == pRsp->socket)
		{
			SYNCTRACE("socket:%d", pRsp->socket);
		}
	}
}

static void
_sync_client_disconnect_req(SyncServer *pServer)
{
	SocketDisconnectReq *pReq;
	s32 socket = pServer->server_socket;

	if(socket != INVALID_SOCKET_ID)
	{
		SYNCTRACE("verno:%s disconnect! socket:%d type:%d state:%d/%d/%d/%d left-timer:%d recnt:%d",
			pServer->verno,
			pServer->server_socket,
			pServer->server_type,
			pServer->server_connecting,
			pServer->server_cnt,
			pServer->server_booting,
			pServer->server_ready,
			pServer->left_timer,
			pServer->reconnect_times);

		pReq = thread_reset_msg(pReq);

		pReq->socket = socket;
		pReq->ptr = pServer;

		id_msg(_socket_thread, SOCKET_DISCONNECT_REQ, pReq);
	}
}

static void
_sync_client_connect_rsp(SocketConnectRsp *pRsp)
{
	SyncServer *pServer = (SyncServer *)(pRsp->ptr);
	dave_bool clean_flag;

	if(pServer == NULL)
	{
		SYNCLOG("can't find server:%s",
			ipv4str(pRsp->NetInfo.addr.ip.ip_addr, pRsp->NetInfo.port));
		return;
	}

	switch(pRsp->ConnectInfo)
	{
		case SOCKETINFO_CONNECT_OK:
		case SOCKETINFO_CONNECT_WAIT:
				if(pServer->server_connecting == dave_true)
				{
					pServer->server_socket = pRsp->socket;

					SYNCTRACE("socket:%d %d/%d",
						pServer->server_socket,
						pServer->server_connecting,
						pServer->server_cnt);
				}
			break;
		default:
				SYNCABNOR("%s type:%s failed! (%d)",
					ipv4str(pRsp->NetInfo.addr.ip.ip_addr, pRsp->NetInfo.port),
					sync_client_type_to_str(pServer->server_type),
					pRsp->ConnectInfo);

				clean_flag = pServer->server_type == SyncServerType_sync_client ? dave_false : dave_true;
				sync_client_data_reset_server(pServer, clean_flag);
			break;
	}
}

static void
_sync_client_connect_req(SyncServer *pServer)
{
	SocketConnectReq *pReq;

	if((pServer->server_socket == INVALID_SOCKET_ID)
		&& (pServer->server_connecting == dave_false)
		&& (pServer->cfg_server_port != 0)
		&& (base_power_state() == dave_true))
	{
		pServer->server_connecting = dave_true;
		pServer->server_cnt = dave_false;
		pServer->left_timer = SYNC_SERVER_LEFT_MAX;

		pReq = thread_reset_msg(pReq);

		pReq->NetInfo.domain = DM_SOC_PF_RAW_INET;
		pReq->NetInfo.type = TYPE_SOCK_STREAM;
		pReq->NetInfo.addr_type = NetAddrIPType;
		pReq->NetInfo.addr.ip.ver = IPVER_IPV4;
		dave_memcpy(pReq->NetInfo.addr.ip.ip_addr, pServer->cfg_server_ip, sizeof(pServer->cfg_server_ip));
		pReq->NetInfo.port = pServer->cfg_server_port;
		pReq->NetInfo.fixed_src_flag = NotFixedPort;
		pReq->NetInfo.enable_keepalive_flag = KeepAlive_disable;
		pReq->NetInfo.netcard_bind_flag = NetCardBind_disable;

		pReq->ptr = pServer;

		SYNCTRACE("%s", ipv4str(pReq->NetInfo.addr.ip.ip_addr, pReq->NetInfo.port));

		id_msg(_socket_thread, SOCKET_CONNECT_REQ, pReq);
	}
}

static dave_bool
_sync_client_disconnect_all(dave_bool reboot_flag)
{
	SyncServer *pServerHead = sync_client_data_head_server();
	ub server_index;
	dave_bool has_disconnect = dave_false;

	for(server_index=0; server_index<SERVER_DATA_MAX; server_index++)
	{
		if(pServerHead[server_index].server_socket != INVALID_SOCKET_ID)
		{
			SYNCDEBUG("reboot_flag:%d server_socket:%d server_type:%d cfg_server_port:%d",
				reboot_flag,
				pServerHead[server_index].server_socket,
				pServerHead[server_index].server_type,
				pServerHead[server_index].cfg_server_port);

			if((reboot_flag == dave_true)
				|| ((pServerHead[server_index].server_type == SyncServerType_client)
					&& (pServerHead[server_index].cfg_server_port == 0)))
			{
				has_disconnect = dave_true;

				_sync_client_disconnect_req(&pServerHead[server_index]);
			}
		}
	}

	return has_disconnect;
}

static void
_sync_client_connect_all(void)
{
	SyncServer *pServerHead = sync_client_data_head_server();
	ub server_index;

	for(server_index=0; server_index<SERVER_DATA_MAX; server_index++)
	{
		if(((pServerHead[server_index].server_type == SyncServerType_sync_client)
			  || (pServerHead[server_index].server_type == SyncServerType_client))
			&& (pServerHead[server_index].server_socket == INVALID_SOCKET_ID)
			&& (pServerHead[server_index].server_connecting == dave_false)
			&& (pServerHead[server_index].cfg_server_port != 0))
		{
			_sync_client_connect_req(&pServerHead[server_index]);
		}
	}
}

static void
_sync_client_plugin_server(SocketPlugIn *pPlugIn, SyncServer *pServer)
{
	if(pServer == NULL)
	{
		SYNCLOG("can't find server! socket:%d", pPlugIn->child_socket);
		return;
	}

	build_rxtx(TYPE_SOCK_STREAM, pPlugIn->child_socket, pPlugIn->NetInfo.port);

	pServer->server_socket = pPlugIn->child_socket;
	pServer->server_connecting = dave_false;
	pServer->server_cnt = dave_true;
	pServer->server_booting = dave_true;
	pServer->booting_reciprocal = SYNC_BOOTING_RECIPROCAL;

	pServer->left_timer = SYNC_SERVER_LEFT_MAX;
	pServer->reconnect_times = SYNC_RECONNECT_TIMES;

	pServer->recv_data_counter = 0;
	pServer->send_data_counter = 0;
}

static void
_sync_client_plugout_server(SocketPlugOut *pPlugOut, SyncServer *pServer)
{
	dave_bool clean_flag;

	if(pServer == NULL)
	{
		SYNCLOG("can't find server! socket:%d", pPlugOut->socket);
		return;
	}

	SYNCTRACE("verno:%s %s socket:%d",
		pServer->verno, sync_client_type_to_str(pServer->server_type),
		pServer->server_socket);

	clean_rxtx(pPlugOut->socket);

	sync_client_data_del_server_on_all_thread(pServer);

	clean_flag = (_sync_client_reconnect_syncs_logic_flag == dave_true ? dave_true : dave_false);
	if(pServer->server_type == SyncServerType_child)
	{
		clean_flag = dave_true;
	}

	sync_client_data_reset_server(pServer, clean_flag);

	if((_sync_client_reconnect_syncs_logic_flag == dave_true)
		&& (sync_client_data_all_server_is_disconnect() == dave_true))
	{
		_sync_client_reconnect_syncs_action();
	}
}

static SyncServer *
_sync_client_plugin(SocketPlugIn *pPlugIn)
{
	SyncServer *pServer;

	pServer = sync_client_link_plugin(pPlugIn);
	if(pServer == NULL)
	{
		pServer = (SyncServer *)(pPlugIn->ptr);

		pServer->server_socket = pPlugIn->child_socket;
	}

	if(pServer == NULL)
	{
		SYNCLOG("socket:%d/%d %s/%s empty pServer!",
			pPlugIn->father_socket, pPlugIn->child_socket,
			ipv4str(pPlugIn->NetInfo.addr.ip.ip_addr, pPlugIn->NetInfo.port),
			ipv4str2(pPlugIn->NetInfo.src_ip.ip_addr, pPlugIn->NetInfo.src_port));
		return NULL;
	}

	SYNCLOG("socket:%d/%d pServer:%x/%x/%x type:%s %s/%s",
		pPlugIn->father_socket, pPlugIn->child_socket,
		pServer, pServer->server_index, pServer->shadow_index,
		sync_client_type_to_str(pServer->server_type),
		ipv4str(pPlugIn->NetInfo.addr.ip.ip_addr, pPlugIn->NetInfo.port),
		ipv4str2(pPlugIn->NetInfo.src_ip.ip_addr, pPlugIn->NetInfo.src_port));

	SAFECODEv2W(pServer->rxtx_pv, _sync_client_plugin_server(pPlugIn, pServer););

	return pServer;
}

static void
_sync_client_plugout(SocketPlugOut *pPlugOut)
{
	SyncServer *pServer;

	pServer = sync_client_link_plugout(pPlugOut);
	if(pServer == NULL)
	{
		pServer = sync_client_data_server_inq_on_socket(pPlugOut->socket);
		if(pServer == NULL)
		{
			pServer = sync_client_data_server_inq_on_net(pPlugOut->NetInfo.addr.ip.ip_addr, pPlugOut->NetInfo.port);
		}
	}

	if(pServer != NULL)
	{
		SYNCLOG("socket:%d pServer:%x/%x/%x type:%s %s/%s time:%d/%d",
			pPlugOut->socket,
			pServer, pServer->server_index, pServer->shadow_index,
			sync_client_type_to_str(pServer->server_type),
			pServer->globally_identifier, pServer->verno,
			pServer->left_timer, pServer->reconnect_times);

		SAFECODEv2W(pServer->rxtx_pv, _sync_client_plugout_server(pPlugOut, pServer););
	}
}

static void
_sync_client_reboot(RESTARTREQMSG *pRestart)
{
	if(pRestart->times == 1)
	{
		sync_client_link_stop();

		if(_sync_client_timer != INVALID_TIMER_ID)
		{
			base_timer_die(_sync_client_timer);
			_sync_client_timer = INVALID_TIMER_ID;
		}
	}
	else if(pRestart->times == 3)
	{
		_sync_client_disconnect_all(dave_true);
	}
}

static void
_sync_client_check_time(void)
{
	SyncServer *pServerHead = sync_client_data_head_server();
	SyncServer *pServer;
	ub server_index;
	sb left_timer;

	for(server_index=0; server_index<SERVER_DATA_MAX; server_index++)
	{
		pServer = &pServerHead[server_index];
		if(pServer->server_socket != INVALID_SOCKET_ID)
		{
			sync_lock();
			left_timer = -- pServer->left_timer;
			sync_unlock();

			if(left_timer < 0)
			{
				SYNCLOG("verno:%s disconnect! socket:%d state:%d/%d/%d/%d left-timer:%d recnt:%d",
					pServer->verno,
					pServer->server_socket,
					pServer->server_connecting,
					pServer->server_cnt,
					pServer->server_booting,
					pServer->server_ready,
					pServer->left_timer,
					pServer->reconnect_times);

				_sync_client_disconnect_req(pServer);

				sync_lock();
				pServer->left_timer = SYNC_SERVER_LEFT_MAX;
				sync_unlock();
			}
			else if(pServer->server_cnt == dave_true)
			{
				SYNCDEBUG("verno:%s server_socket:%d state:%d/%d/%d/%d left_timer:%d",
					pServer->verno,
					pServer->server_socket,
					pServer->server_connecting,
					pServer->server_cnt,
					pServer->server_booting,
					pServer->server_ready,
					pServer->left_timer);

				if((SYNC_SERVER_LEFT_MAX - left_timer) > SYNC_CLIENT_HEARTBEAT_TIME)
				{
					sync_client_tx_heartbeat(pServer, dave_true);
				}
			}
		}
	}
}

static void
_sync_client_booting_time(void)
{
	SyncServer *pServerHead = sync_client_data_head_server();
	SyncServer *pServer;
	ub server_index;

	for(server_index=0; server_index<SERVER_DATA_MAX; server_index++)
	{
		pServer = &pServerHead[server_index];
		if((pServer->server_socket != INVALID_SOCKET_ID)
			&& (pServer->server_cnt == dave_true)
			&& (pServer->server_booting == dave_true)
			&& ((-- pServer->booting_reciprocal) <= 0))
		{
			SYNCTRACE("%x type:%s socket:%d (%d%d%d%d)",
				pServer,
				sync_client_type_to_str(pServer->server_type),
				pServer->server_socket,
				pServer->server_connecting, pServer->server_cnt,
				pServer->server_booting, pServer->server_ready);

			pServer->booting_reciprocal = SYNC_BOOTING_RECIPROCAL;

			sync_client_tx_my_verno(pServer);
			sync_client_tx_rpcver_req(pServer);
			sync_client_service_statement_tx(pServer);
		}
	}
}

static void
_sync_client_guard_time(TIMERID timer_id, ub thread_index)
{
	SAFECODEv2TW(_sync_client_system_lock, {
		SAFECODEv2TW(_sync_client_data_pv, {

			/*
			 * 用到了sync_client_data_head_server结构体的位置都需要用
			 * 锁锁起来！
			 */

			_sync_client_booting_time();
			_sync_client_check_time();

			if(_sync_client_reconnect_syncs_logic_flag == dave_false)
			{
				_sync_client_connect_all();
				_sync_client_disconnect_all(dave_false);
			}

		} );
	} );
}

static void
_sync_client_system_busy(SystemBusy *pBusy)
{
	sync_client_data_set_busy(dave_true);

	sync_client_tx_system_state(NULL);
}

static void
_sync_client_system_idle(SystemIdle *pIdle)
{
	sync_client_data_set_busy(dave_false);

	sync_client_tx_system_state(NULL);
}

static void
_sync_client_reconnect_syncs_action(void)
{
	SyncServer *pSyncServer;

	_sync_client_reconnect_syncs_logic_flag = dave_false;
	
	sync_client_data_reset_sync_server();
	pSyncServer = sync_client_data_sync_server();
	_sync_client_connect_all();

	SYNCLOG("Configuration has changed, Connect to SYNC service from %s!",
		ipv4str(pSyncServer->cfg_server_ip, pSyncServer->cfg_server_port));
}

static void
_sync_client_reconnect_syncs_logic(void)
{
	SyncServer *pSyncServer = sync_client_data_sync_server();
	u8 cfg_ip_before[16];
	u16 cfg_port_before;
	u8 cfg_ip_after[16];
	u16 cfg_port_after;

	dave_memset(cfg_ip_before, 0x00, sizeof(cfg_ip_before));
	dave_memset(cfg_ip_after, 0x00, sizeof(cfg_ip_after));

	dave_memcpy(cfg_ip_before, pSyncServer->cfg_server_ip, sizeof(pSyncServer->cfg_server_ip));
	cfg_port_before = pSyncServer->cfg_server_port;
	sync_client_data_reset_sync_server();
	dave_memcpy(cfg_ip_after, pSyncServer->cfg_server_ip, sizeof(pSyncServer->cfg_server_ip));
	cfg_port_after = pSyncServer->cfg_server_port;

	if((dave_memcmp(cfg_ip_before, cfg_ip_after, 4) == dave_true)
		&& (cfg_port_before == cfg_port_after))
	{
		SYNCLOG("Configuration not change!");
	}
	else
	{
		_sync_client_reconnect_syncs_logic_flag = dave_true;

		sync_client_link_stop();

		SYNCLOG("Configuration has changed(%s->%s), restart sync now(%d) ...",
			ipv4str(cfg_ip_before, cfg_port_before),
			ipv4str2(cfg_ip_after, cfg_port_after),
			_sync_client_reconnect_syncs_logic_flag);

		if(_sync_client_disconnect_all(dave_true) == dave_false)
		{
			_sync_client_reconnect_syncs_action();
		}
	}
}

static void
_sync_client_cfg_update(CFGUpdate *pUpdate)
{
	if(dave_strcmp(pUpdate->cfg_name, CFG_SYNC_SERVER_DOMAIN) == dave_true)
	{
		SYNCTRACE("%s update! %d/%d",
			pUpdate->cfg_name,
			_sync_client_reconnect_syncs_logic_flag,
			_sync_client_reconnect_syncs_logic_timer);

		if((_sync_client_reconnect_syncs_logic_flag == dave_false)
			&& (_sync_client_reconnect_syncs_logic_timer == INVALID_TIMER_ID))
		{
			/*
			 * 有可能还有CFG_SYNC_PORT设置信息过来，
			 * 通过定时器同步。
			 */
			_sync_client_reconnect_syncs_logic_timer = base_timer_creat("scrl", _sync_client_safe_reconnect_syncs_logic_timer_out, 3000);
		}
	}
}

static inline void
_sync_client_route(MSGBODY *pMsg)
{
	ThreadId msg_dst;

	if((pMsg->msg_type == BaseMsgType_Broadcast_local)
		|| (pMsg->msg_type == BaseMsgType_Broadcast_local_no_me))
	{
		return;
	}

	if(pMsg->msg_type != BaseMsgType_Broadcast_remote)
	{
		msg_dst = sync_client_thread_id_change_from_user(pMsg->msg_dst, _sync_client_thread);
		if(msg_dst == INVALID_THREAD_ID)
		{
			SYNCLOG("%lx/%s->%lx/%s:%s %s change failed!",
				pMsg->msg_src, thread_name(pMsg->msg_src),
				pMsg->msg_dst, thread_name(pMsg->msg_dst),
				msgstr(pMsg->msg_id),
				t_auto_BaseMsgType_str(pMsg->msg_type));
			return;
		}
		pMsg->msg_dst = msg_dst;
	}

	sync_client_message_route(pMsg);
}

static void
_sync_client_safe_reconnect_syncs_logic_timer_out(TIMERID timer_id, ub thread_index)
{
	SAFECODEv2W(_sync_client_system_lock, {

		if(_sync_client_reconnect_syncs_logic_flag == dave_false)
		{
			_sync_client_reconnect_syncs_logic();
		}

		base_timer_die(timer_id);

		_sync_client_reconnect_syncs_logic_timer = INVALID_TIMER_ID;

	} );
}

static void
_sync_client_safe_system_mount(ThreadId src, SystemMount *pMount)
{
	SAFECODEv2W(_sync_client_system_lock, { _sync_client_system_mount(src, pMount); } );
}

static void
_sync_client_safe_system_decoupling(ThreadId src, SystemDecoupling *pDecoupling)
{
	SAFECODEv2W(_sync_client_system_lock, { _sync_client_system_decoupling(src, pDecoupling); } );
}

static void
_sync_client_safe_connect_rsp(SocketConnectRsp *pRsp)
{
	SAFECODEv2W(_sync_client_system_lock, { _sync_client_connect_rsp(pRsp); } );
}

static void
_sync_client_safe_disconnect_rsp(SocketDisconnectRsp *pRsp)
{
	SAFECODEv2W(_sync_client_system_lock, { _sync_client_disconnect_rsp(pRsp); } );
}

static void
_sync_client_safe_plugin(SocketPlugIn *pPlugIn)
{
	SyncServer *pServer = NULL;

	if(base_power_state() == dave_false)
	{
		return;
	}

	SAFECODEv2W(_sync_client_system_lock, { pServer = _sync_client_plugin(pPlugIn); });

	if(pServer != NULL)
	{
		sync_client_tx_heartbeat(pServer, dave_true);
	}
}

static void
_sync_client_safe_plugout(SocketPlugOut *pPlugOut)
{
	SAFECODEv2W(_sync_client_system_lock, { _sync_client_plugout(pPlugOut); });
}

static void
_sync_client_safe_rx_event(SocketRawEvent *pEvent)
{
	SyncServer *pServer = sync_client_data_server_inq_on_socket(pEvent->socket);
	dave_bool re_event = dave_true;

	if((pServer == NULL) || (pServer->server_socket == INVALID_SOCKET_ID))
	{
		dave_mfree(pEvent->data);
		return;
	}

	if(pServer->server_cnt == dave_false)
	{
		dave_os_sleep(1000);
	}
	else
	{
		SAFECODEv2TR( pServer->rxtx_pv, sync_client_rx(pServer, pEvent); re_event = dave_false; );
	}

	if(re_event == dave_true)
	{
		SocketRawEvent *pNewEvent = thread_msg(pNewEvent);	
		*pNewEvent = *pEvent;
		id_msg(self(), SOCKET_RAW_EVENT, pNewEvent);
	}
}

static void
_sync_client_safe_reboot(RESTARTREQMSG *pRestart)
{
	SAFECODEv2W(_sync_client_system_lock, { _sync_client_reboot(pRestart); } );
}

static void
_sync_client_safe_cfg_update(CFGUpdate *pUpdate)
{
	SAFECODEv2W(_sync_client_system_lock, { _sync_client_cfg_update(pUpdate); } );
}

static void
_sync_client_cfg_remote_update(CFGRemoteSyncUpdate *pUpdate)
{
	sync_client_tx_run_internal_msg_req(
		sync_client_data_sync_server(),
		MSGID_CFG_REMOTE_SYNC_UPDATE, sizeof(CFGRemoteSyncUpdate), pUpdate,
		dave_false);
}

static void
_sync_client_init(MSGBODY *msg)
{
	t_lock_reset(&_sync_client_system_lock);

	_socket_thread = thread_id(SOCKET_THREAD_NAME);

	sync_client_data_init();
	sync_client_run_init();
	sync_client_thread_init();
	sync_client_link_init();
	sync_client_msg_buffer_init();
	sync_client_internal_buffer_init();
	sync_client_tx_init();

	_sync_client_connect_all();

	_sync_client_timer = base_timer_creat("syncct", _sync_client_guard_time, SYNC_CLIENT_BASE_TIME);
}

static void
_sync_client_main(MSGBODY *msg)
{	
	switch((ub)(msg->msg_id))
	{
		case MSGID_RESTART_REQ:
		case MSGID_POWER_OFF:
				_sync_client_safe_reboot((RESTARTREQMSG *)(msg->msg_body));
			break;
		case MSGID_CFG_UPDATE:
				_sync_client_safe_cfg_update((CFGUpdate *)(msg->msg_body));
			break;
		case MSGID_CFG_REMOTE_SYNC_UPDATE:
				_sync_client_cfg_remote_update((CFGRemoteSyncUpdate *)(msg->msg_body));
			break;
		case MSGID_DEBUG_REQ:
				sync_test_req(msg->msg_src, (DebugReq *)(msg->msg_body), sync_client_info);
			break;
		case MSGID_SYSTEM_MOUNT:
				_sync_client_safe_system_mount(msg->msg_src, (SystemMount *)(msg->msg_body));
			break;
		case MSGID_SYSTEM_DECOUPLING:
				_sync_client_safe_system_decoupling(msg->msg_src, (SystemDecoupling *)(msg->msg_body));
			break;
		case MSGID_SYSTEM_BUSY:
				_sync_client_system_busy((SystemBusy *)(msg->msg_body));
			break;
		case MSGID_SYSTEM_IDLE:
				_sync_client_system_idle((SystemIdle *)(msg->msg_body));
			break;
		case MSGID_INTERNAL_LOOP:
				SYNCLOG("This message should not appear here, it is already handled in module base_rxtx!");
			break;
		case MSGID_QUEUE_RUN_MESSAGE_REQ:
				sync_client_queue_run((QueueRunMsgReq *)(msg->msg_body));
			break;
		case SOCKET_CONNECT_RSP:
				_sync_client_safe_connect_rsp((SocketConnectRsp *)(msg->msg_body));
			break;
		case SOCKET_DISCONNECT_RSP:
				_sync_client_safe_disconnect_rsp((SocketDisconnectRsp *)(msg->msg_body));
			break;
		case SOCKET_PLUGIN:
				_sync_client_safe_plugin((SocketPlugIn *)(msg->msg_body));
			break;
		case SOCKET_PLUGOUT:
				_sync_client_safe_plugout((SocketPlugOut *)(msg->msg_body));
			break;
		case SOCKET_RAW_EVENT:
				_sync_client_safe_rx_event((SocketRawEvent *)(msg->msg_body));
			break;
		case MSGID_WAKEUP:
		case MSGID_LOCAL_THREAD_READY:
		case MSGID_LOCAL_THREAD_REMOVE:
		case MSGID_CFG_REMOTE_UPDATE:
		case MSGID_REMOTE_THREAD_ID_READY:
		case MSGID_REMOTE_THREAD_ID_REMOVE:
		case MSGID_REMOTE_THREAD_READY:
		case MSGID_REMOTE_THREAD_REMOVE:
		case MSGID_THREAD_BUSY:
		case MSGID_THREAD_IDLE:
			break;
		default:
				_sync_client_route(msg);
			break;
	}
}

static void
_sync_client_exit(MSGBODY *msg)
{
	sync_client_run_exit();
	sync_client_internal_buffer_exit();
	sync_client_msg_buffer_exit();
	sync_client_thread_exit();
	sync_client_link_exit();
	sync_client_data_exit();
	sync_client_tx_exit();
}

// =====================================================================

void
sync_client_init(s8 *sync_domain)
{
	ub thread_number = dave_os_cpu_process_number();

	sync_cfg_external_incoming_sync_domain(sync_domain);

	_sync_client_thread = base_thread_creat(SYNC_CLIENT_THREAD_NAME, thread_number, THREAD_THREAD_FLAG|THREAD_PRIVATE_FLAG|THREAD_CORE_FLAG, _sync_client_init, _sync_client_main, _sync_client_exit);
	if(_sync_client_thread == INVALID_THREAD_ID)
		base_restart(SYNC_CLIENT_THREAD_NAME);
}

void
sync_client_exit(void)
{
	if(_sync_client_thread != INVALID_THREAD_ID)
		base_thread_del(_sync_client_thread);
	_sync_client_thread = INVALID_THREAD_ID;
}

ThreadId
sync_client_thread_id(ThreadId thread_id)
{
	ThreadId new_id;

	new_id = sync_client_thread_id_change_from_user(thread_id, _sync_client_thread);
	if(new_id == INVALID_THREAD_ID)
		return thread_id;
	else
		return new_id;
}

#endif

