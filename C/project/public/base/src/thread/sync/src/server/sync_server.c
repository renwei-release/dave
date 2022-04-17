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
#include "sync_param.h"
#include "sync_base_package.h"
#include "sync_server_param.h"
#include "sync_server_broadcadt.h"
#include "sync_server_tx.h"
#include "sync_server_rx.h"
#include "sync_server_data.h"
#include "sync_server_blocks.h"
#include "sync_server_tools.h"
#include "sync_server_sync.h"
#include "sync_server_run.h"
#include "sync_server_msg_buffer.h"
#include "sync_server_info.h"
#include "sync_cfg.h"
#include "sync_lock.h"
#include "sync_test.h"
#include "sync_log.h"

static TLock _sync_server_system_lock;
static ThreadId _sync_server_thread = INVALID_THREAD_ID;
static ThreadId _socket_thread = INVALID_THREAD_ID;
static s32 _sync_server_socket = INVALID_SOCKET_ID;

static void
_sync_server_bind_rsp(SocketBindRsp *pRsp)
{
	switch(pRsp->BindInfo)
	{
		case SOCKETINFO_BIND_OK:
				SYNCTRACE("sync server bind OK! (socket:%d)", pRsp->socket);
				_sync_server_socket = pRsp->socket;
			break;
		default:
				SYNCABNOR("sync server bind failed! (%d/%s)",
					pRsp->BindInfo,
					ipv4str(pRsp->NetInfo.addr.ip.ip_addr, pRsp->NetInfo.port));
				base_restart("sync server bind reboot!");
			break;
	}
}

static void
_sync_server_bind_req(void)
{
	SocketBindReq *pReq = thread_reset_msg(pReq);

	pReq->NetInfo.domain = DM_SOC_PF_RAW_INET;
	pReq->NetInfo.type = TYPE_SOCK_STREAM;
	pReq->NetInfo.addr_type = NetAddrIPType;
	pReq->NetInfo.addr.ip.ver = IPVER_IPV4;
	dave_memset(pReq->NetInfo.addr.ip.ip_addr, 0x00, 16);
	pReq->NetInfo.port = sync_cfg_get_syncs_port();
	pReq->NetInfo.fixed_src_flag = NotFixedPort;
	pReq->NetInfo.enable_keepalive_flag = KeepAlive_disable;
	pReq->NetInfo.netcard_bind_flag = NetCardBind_disable;

	SYNCTRACE("%s", ipv4str(pReq->NetInfo.addr.ip.ip_addr, pReq->NetInfo.port));

	write_msg(_socket_thread, SOCKET_BIND_REQ, pReq);
}

static void
_sync_server_disconnect(s32 socket)
{
	SocketDisconnectReq *pReq;

	SYNCTRACE("socket:%d", socket);

	if(socket == INVALID_SOCKET_ID)
	{
		return;
	}

	pReq = thread_reset_msg(pReq);

	pReq->socket = socket;
	pReq->ptr = NULL;

	write_msg(_socket_thread, SOCKET_DISCONNECT_REQ, pReq);
}

static void
_sync_server_events(InternalEvents *pEvents)
{
	switch(pEvents->event_id)
	{
		case SyncServerEvents_version:
				sync_server_rx_version((SyncClient *)(pEvents->ptr));
			break;
		case SyncServerEvents_link_up:
				sync_server_rx_link_up((SyncClient *)(pEvents->ptr));
			break;
		case SyncServerEvents_link_down:
				sync_server_rx_link_down((SyncClient *)(pEvents->ptr));
			break;
		default:
				SYNCLOG("invalid event_id:%d", pEvents->event_id);
			break;
	}
}

static void
_sync_server_client_busy(ClientBusy *pBusy)
{
	SyncClient *pClient = (SyncClient *)(pBusy->ptr);

	sync_server_client_state(pClient, dave_false);

	SYNCLOG("%s on busy! %s", pBusy->verno, ipv4str(pClient->NetInfo.addr.ip.ip_addr, pClient->NetInfo.port));
}

static void
_sync_server_client_idle(ClientIdle *pIdle)
{
	SyncClient *pClient = (SyncClient *)(pIdle->ptr);

	sync_server_client_state(pClient, dave_true);

	SYNCLOG("%s on idle! %s", pIdle->verno, ipv4str(pClient->NetInfo.addr.ip.ip_addr, pClient->NetInfo.port));
}

static void
_sync_server_plugin(SocketPlugIn *pPlugIn)
{
	if(_sync_server_socket == pPlugIn->father_socket)
	{
		sync_server_add_client(pPlugIn->child_socket, &(pPlugIn->NetInfo));

		SYNCLOG("socket:%d %s/%d",
			pPlugIn->child_socket,
			ipv4str(pPlugIn->NetInfo.addr.ip.ip_addr, pPlugIn->NetInfo.port),
			pPlugIn->NetInfo.src_port);
	}
	else
	{
		SYNCABNOR("father socket mismatch!%d,%d", _sync_server_socket, pPlugIn->father_socket);
	}
}

static void
_sync_server_plugout(SocketPlugOut *pPlugOut)
{
	SyncClient *pClient;

	if(_sync_server_socket == pPlugOut->socket)
	{
		_sync_server_socket = INVALID_SOCKET_ID;
	}
	else
	{
		pClient = sync_server_find_client(pPlugOut->socket);

		if(pClient != NULL)
		{
			SYNCLOG("socket:%d %s %s/%s reason:%d",
				pPlugOut->socket,
				ipv4str(pPlugOut->NetInfo.addr.ip.ip_addr, pPlugOut->NetInfo.port),
				pClient->globally_identifier, pClient->verno,
				pPlugOut->reason);

			sync_server_del_client(pClient);
		}
		else
		{
			SYNCLOG("can't find the client:%d", pPlugOut->socket);
		}
	}
}

static void
_sync_server_reboot(RESTARTREQMSG *pRestart)
{
	if(pRestart->times == 1)
	{
		sync_server_msg_buffer_exit();

		sync_server_data_exit();

		sync_server_broadcadt_exit();

		sync_server_run_exit();

		if(_sync_server_socket != INVALID_SOCKET_ID)
		{
			_sync_server_disconnect(_sync_server_socket);
		}
	}
}

static void
_sync_server_check_client_time(SyncClient *pClient)
{
	sb left_timer;

	if(pClient->client_socket != INVALID_SOCKET_ID)
	{
		sync_lock();
		left_timer = -- pClient->left_timer;
		sync_unlock();

		if(left_timer < 0)
		{
			SYNCLOG("socket:%d verno:%s disconnect! left-timer:%d",
				pClient->client_socket, pClient->verno, pClient->left_timer);

			sync_lock();
			pClient->left_timer = SYNC_CLIENT_LEFT_MAX;
			sync_unlock();

			_sync_server_disconnect(pClient->client_socket);
		}
	
		if((-- pClient->sync_timer) < 0)
		{
			if((pClient->receive_thread_done == dave_true) && (pClient->sync_thread_flag == dave_true))
			{
				sync_server_sync_thread_booting(pClient);
			}
		}
	}
}

static void
_sync_server_guard_time(TIMERID timer_id, ub thread_index)
{
	ub client_index;
	SyncClient *pClient;

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		pClient = sync_server_client(client_index);

		SAFECODEidlev1(pClient->opt_pv, _sync_server_check_client_time(pClient););
	}
}

static void
_sync_server_safe_guard_time(TIMERID timer_id, ub thread_index)
{
	SAFECODEv2TW(_sync_server_system_lock, _sync_server_guard_time(timer_id, thread_index););
}

static void
_sync_server_safe_reboot(RESTARTREQMSG *pRestart)
{
	SAFECODEv2W(_sync_server_system_lock, _sync_server_reboot(pRestart););
}

static void
_sync_server_safe_events(InternalEvents *pEvents)
{
	SAFECODEv2W(_sync_server_system_lock, _sync_server_events(pEvents););
}

static void
_sync_server_safe_client_busy(ClientBusy *pBusy)
{
	SAFECODEv2W(_sync_server_system_lock, _sync_server_client_busy(pBusy););
}

static void
_sync_server_safe_client_idle(ClientIdle *pIdle)
{
	SAFECODEv2W(_sync_server_system_lock, _sync_server_client_idle(pIdle););
}

static void
_sync_server_safe_plugin(SocketPlugIn *pPlugIn)
{
	SAFECODEv2W(_sync_server_system_lock, _sync_server_plugin(pPlugIn););
}

static void
_sync_server_safe_plugout(SocketPlugOut *pPlugOut)
{
	SAFECODEv2W(_sync_server_system_lock, _sync_server_plugout(pPlugOut););
}

static void
_sync_server_safe_bind_rsp(SocketBindRsp *pRsp)
{
	SAFECODEv2W(_sync_server_system_lock, _sync_server_bind_rsp(pRsp););
}

static void
_sync_server_safe_blocks_command(ThreadId src, MsgBlocksReq *pReq)
{
	SAFECODEv2W(_sync_server_system_lock, sync_server_blocks_command(src, pReq););
}

static void
_sync_server_safe_rx_read(SocketRead *pRead)
{
	SyncClient *pClient;

	pClient = sync_server_find_client(pRead->socket);
	if(pClient == NULL)
	{
		dave_mfree(pRead->data);
		return;
	}

	SAFECODEv2R(_sync_server_system_lock, sync_server_rx_read(pClient, pRead););
}

static void
_sync_server_safe_rx_event(SocketRawEvent *pEvent)
{
	SyncClient *pClient;

	pClient = sync_server_find_client(pEvent->socket);
	if(pClient == NULL)
	{
		dave_mfree(pEvent->data);
		return;
	}

	SAFECODEv2R(_sync_server_system_lock, sync_server_rx_event(pClient, pEvent););
}

static void
_sync_server_init(MSGBODY *msg)
{
	t_lock_reset(&_sync_server_system_lock);

	_socket_thread = thread_id(SOCKET_THREAD_NAME);
	_sync_server_socket = INVALID_SOCKET_ID;

	sync_server_data_init();

	sync_server_run_init();

	sync_server_broadcadt_init();

	sync_server_msg_buffer_init();

	_sync_server_bind_req();

	base_timer_creat("syncst", _sync_server_safe_guard_time, SYNC_SERVER_BASE_TIME);
}

static void
_sync_server_main(MSGBODY *msg)
{
	switch((ub)(msg->msg_id))
	{
		case MSGID_DEBUG_REQ:
				sync_test_req(msg->msg_src, (DebugReq *)(msg->msg_body), sync_server_info);
			break;
		case MSGID_RESTART_REQ:
		case MSGID_POWER_OFF:
				_sync_server_safe_reboot((RESTARTREQMSG *)(msg->msg_body));
			break;
		case MSGID_INTERNAL_EVENTS:
				_sync_server_safe_events((InternalEvents *)(msg->msg_body));
			break;
		case MSGID_CLIENT_BUSY:
				_sync_server_safe_client_busy((ClientBusy *)(msg->msg_body));
			break;
		case MSGID_CLIENT_IDLE:
				_sync_server_safe_client_idle((ClientIdle *)(msg->msg_body));
			break;
		case SOCKET_PLUGIN:
				_sync_server_safe_plugin((SocketPlugIn *)msg->msg_body);
			break;
		case SOCKET_PLUGOUT:
				_sync_server_safe_plugout((SocketPlugOut *)msg->msg_body);
			break;
		case SOCKET_BIND_RSP:
				_sync_server_safe_bind_rsp((SocketBindRsp *)msg->msg_body);
			break;
		case MSGID_BLOCKS_REQ:
				_sync_server_safe_blocks_command(msg->msg_src, (MsgBlocksReq *)msg->msg_body);
			break;
		case SOCKET_READ:
				_sync_server_safe_rx_read((SocketRead *)msg->msg_body);
			break;
		case SOCKET_RAW_EVENT:
				_sync_server_safe_rx_event((SocketRawEvent *)(msg->msg_body));
			break;
		default:
			break;
	}
}

static void
_sync_server_exit(MSGBODY *msg)
{

}

// =====================================================================

void
sync_server_init(void)
{
	ub thread_number = dave_os_cpu_process_number();

	_sync_server_thread = base_thread_creat(SYNC_SERVER_THREAD_NAME, thread_number, THREAD_THREAD_FLAG|THREAD_PRIVATE_FLAG, _sync_server_init, _sync_server_main, _sync_server_exit);
	if(_sync_server_thread == INVALID_THREAD_ID)
		base_restart(SYNC_SERVER_THREAD_NAME);
}

void
sync_server_exit(void)
{
	if(_sync_server_thread != INVALID_THREAD_ID)
		base_thread_del(_sync_server_thread);
	_sync_server_thread = INVALID_THREAD_ID;
}

#endif

