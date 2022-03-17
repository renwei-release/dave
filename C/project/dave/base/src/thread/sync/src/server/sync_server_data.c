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
#include "sync_param.h"
#include "sync_base_package.h"
#include "sync_server_param.h"
#include "sync_server_tx.h"
#include "sync_server_rx.h"
#include "sync_server_data.h"
#include "sync_server_tools.h"
#include "sync_server_sync.h"
#include "sync_cfg.h"
#include "sync_lock.h"
#include "sync_test.h"
#include "sync_log.h"

#define SYNC_FAST_INDEX_MAX (DAVE_SERVER_SUPPORT_SOCKET_MAX * 2)

typedef struct {
	SyncClient *pClient;
} SyncServerSocketFastIndex;

static TLock _sync_server_data_pv;
static ub _chose_client_loop_index = 0;
static SyncClient _sync_client[SYNC_CLIENT_MAX];
static SyncServerSocketFastIndex _socket_fast_index[SYNC_FAST_INDEX_MAX];
static SyncThread _sync_thread[SYNC_THREAD_MAX];

static ub _sync_server_del_thread(SyncThread *pThread, SyncClient *pClient);

static void
_sync_server_reset_client(SyncClient *pClient)
{
	ub index;

	pClient->client_socket = INVALID_SOCKET_ID;

	pClient->left_timer = SYNC_CLIENT_LEFT_MAX;
	pClient->sync_timer = 0;

	pClient->recv_data_counter = 0;
	pClient->send_data_counter = 0;
	pClient->recv_msg_counter = 0;
	pClient->send_msg_counter = 0;

	pClient->sync_resynchronization_counter = 0;
	pClient->sync_thread_index = 0;

	dave_memset(pClient->verno, 0x00, sizeof(pClient->verno));
	dave_memset(pClient->globally_identifier, 0x00, sizeof(pClient->globally_identifier));
	pClient->rpc_version = 3;
	pClient->work_start_second = 0;
	dave_memset(&(pClient->NetInfo), 0x00, sizeof(SocNetInfo));
	pClient->link_flag = dave_false;
	dave_memset(pClient->link_ip, 0x00, sizeof(pClient->link_ip));
	pClient->link_port = 0;

	pClient->receive_thread_done = dave_false;
	pClient->sync_thread_flag = dave_false;
	for(index=0; index<SYNC_CLIENT_MAX; index++)
	{
		pClient->send_down_and_up_flag[index] = dave_false;
	}

	pClient->ready_flag = dave_false;
	pClient->blocks_flag = dave_false;
	pClient->client_flag = dave_false;

	pClient->notify_blocks_flag = 1234567;
	pClient->release_quantity = 0;
}

static void
_sync_server_reset_all_client(void)
{
	ub client_index;

	dave_memset(_sync_client, 0x00, sizeof(_sync_client));

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		_sync_client[client_index].client_index = client_index;

		_sync_server_reset_client(&_sync_client[client_index]);

		t_lock_reset(&(_sync_client[client_index].opt_pv));
	}
}

static void
_sync_server_data_reset_index(SyncServerSocketFastIndex *pIndex)
{
	pIndex->pClient = NULL;
}

static void
_sync_server_data_reset_all_index(void)
{
	ub index_index;

	dave_memset(_socket_fast_index, 0x00, sizeof(_socket_fast_index));

	for(index_index=0; index_index<SYNC_FAST_INDEX_MAX; index_index++)
	{
		_sync_server_data_reset_index(&_socket_fast_index[index_index]);
	}
}

static SyncClient *
_sync_server_data_build_socket_fast_index(s32 socket)
{
	ub index_index = socket % SYNC_FAST_INDEX_MAX;
	ub client_index;

	if(_socket_fast_index[index_index].pClient != NULL)
	{
		if(_socket_fast_index[index_index].pClient->client_socket == socket)
		{
			return _socket_fast_index[index_index].pClient;
		}

		_socket_fast_index[index_index].pClient = NULL;
	}

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		if(_sync_client[client_index].client_socket == socket)
		{
			if(socket < SYNC_FAST_INDEX_MAX)
			{
				_socket_fast_index[index_index].pClient = &_sync_client[client_index];
			}

			return &_sync_client[client_index];
		}
	}

	return NULL;
}

static void
_sync_server_reset_thread(SyncThread *pThread)
{
	ub client_index;

	dave_memset(pThread->thread_name, 0x00, sizeof(pThread->thread_name));

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		pThread->pClient[client_index] = NULL;
	}

	pThread->chose_client_index = 0;

	pThread->thread_send_message_counter = 0;
	pThread->thread_recv_message_counter = 0;
}

static void
_sync_server_reset_all_thread(void)
{
	ub thread_index;

	dave_memset(_sync_thread, 0x00, sizeof(_sync_thread));

	for(thread_index=0; thread_index<SYNC_THREAD_MAX; thread_index++)
	{
		_sync_thread[thread_index].thread_index = thread_index;

		_sync_server_reset_thread(&_sync_thread[thread_index]);

		t_lock_reset(&(_sync_thread[thread_index].chose_client_pv));
	}
}

static ub
_sync_server_thread_name_to_index(s8 *thread_name)
{
	ub thread_index, name_index;

	thread_index = 0;

	for(name_index=0; name_index<SYNC_THREAD_NAME_LEN; name_index++)
	{
		if(thread_name[name_index] == '\0')
		{
			break;
		}

		thread_index *= 0xff;

		thread_index += (ub)(thread_name[name_index]);
	}

	thread_index = thread_index % SYNC_THREAD_MAX;

	return thread_index;
}

static inline SyncClient *
_sync_server_find_client(s32 socket, dave_bool find_new)
{
	ub client_index, safe_counter;
	SyncClient *pClient;

	if(find_new == dave_true)
	{
		client_index = _chose_client_loop_index ++;
	}
	else
	{
		client_index = 0;
	}

	for(safe_counter=0; safe_counter<SYNC_CLIENT_MAX; safe_counter++)
	{
		if(client_index >= SYNC_CLIENT_MAX)
		{
			client_index = 0; 
		}
		pClient = &_sync_client[client_index ++];

		if(((find_new == dave_true) && (pClient->client_socket == INVALID_SOCKET_ID))
			|| (pClient->client_socket == socket))
		{
			pClient->client_socket = socket;

			return pClient;
		}
	}

	return NULL;
}

static void
_sync_server_please_sync_all_thread_to_all_client(void)
{
	ub client_index;

	// Because there are new threads, all the thread information to synchronize once!
	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		if(_sync_client[client_index].client_socket != INVALID_SOCKET_ID)
		{
			_sync_client[client_index].sync_timer = SYNC_CLIENT_SYNC_MAX;
			_sync_client[client_index].sync_thread_flag = dave_true;
		}
	}
}

static void
_sync_server_clean_client_all_thread(SyncClient *pClient)
{
	SyncThread *pThread;
	sb thread_index, client_index;
	s8 thread_name[SYNC_THREAD_NAME_LEN];

	for(thread_index=0; thread_index<SYNC_THREAD_MAX; thread_index++)
	{
		pThread = &_sync_thread[thread_index];

		dave_strcpy(thread_name, pThread->thread_name, SYNC_THREAD_NAME_LEN);

		if((pThread->thread_name[0] != '\0')
			&& (_sync_server_del_thread(pThread, pClient) == 0))
		{
			for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
			{
				if((_sync_client[client_index].client_socket != INVALID_SOCKET_ID)
					&& (&_sync_client[client_index] != pClient))
				{
					sync_server_tx_del_remote_thread_req(&_sync_client[client_index], thread_name, thread_index);
				}
			}
		}
	}
}

static void
_sync_server_clean_the_client_to_down_flag(SyncClient *pClient)
{
	ub client_index;

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		_sync_client[client_index].send_down_and_up_flag[pClient->client_index] = dave_false;
	}
}

static SyncClient *
_sync_server_add_client(s32 socket, SocNetInfo *pNetInfo)
{
	SyncClient *pClient;
	dave_bool ret = dave_false;

	pClient = _sync_server_find_client(socket, dave_false);
	if(pClient == NULL)
	{
		pClient = _sync_server_find_client(socket, dave_true);
	}

	if(pClient == NULL)
	{
		SYNCABNOR("socket:%d can't find pClient!", socket);
	}
	else
	{
		SAFEZONEv3(pClient->opt_pv, {
			_sync_server_reset_client(pClient);
			pClient->client_socket = socket;
			pClient->NetInfo = *pNetInfo;
			ret = build_rxtx(TYPE_SOCK_STREAM, socket, pNetInfo->port);
		} );
	}

	if(ret == dave_false)
	{
		SYNCABNOR("socket:%d add client failed!", socket);
	}
	else
	{
		SYNCDEBUG("socket:%d", socket);
	}

	_sync_server_please_sync_all_thread_to_all_client();

	return pClient;
}

static void
_sync_server_del_client(SyncClient *pClient)
{
	SAFEZONEv3(pClient->opt_pv, {
		_sync_server_clean_client_all_thread(pClient);
		_sync_server_clean_the_client_to_down_flag(pClient);
		clean_rxtx(pClient->client_socket);
		_sync_server_reset_client(pClient);
	} );
}

static ub
_sync_server_del_thread_client(SyncThread *pThread, SyncClient *pClient)
{
	ub client_index, del_number, client_number;

	if((pThread == NULL) || (pThread->thread_name[0] == '\0'))
	{
		return SYNC_CLIENT_MAX;
	}

	for(client_index=0,del_number=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		if(pThread->pClient[client_index] == pClient)
		{
			pThread->pClient[client_index] = NULL;
			del_number ++;
		}
	}

	if(del_number == 0)
	{
		return SYNC_CLIENT_MAX;
	}

	for(client_index=0,client_number=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		if(pThread->pClient[client_index] != NULL)
		{
			client_number ++;
		}
	}

	return client_number;
}

static dave_bool
_sync_server_add_thread_client(SyncThread *pThread, SyncClient *pClient)
{
	ub client_index;

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		if(pThread->pClient[client_index] != NULL)
		{
			if(sync_server_are_they_brothers(pThread->pClient[client_index], pClient) == dave_false)
			{
				SYNCABNOR("Currently different services do not allow adding threads with the same name:%s! %s/%s",
					pThread->thread_name,
					pThread->pClient[client_index]->verno,
					pClient->verno);
				return dave_false;
			}
		}
	}

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		if(pThread->pClient[client_index] == pClient)
		{
			return dave_true;
		}
	}

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		if(pThread->pClient[client_index] == NULL)
		{
			pThread->pClient[client_index] = pClient;

			return dave_true;
		}
	}

	return dave_false;
}

static inline SyncThread *
_sync_server_find_thread(s8 *thread_name, ub thread_index, dave_bool find_new)
{
	ub safe_counter;
	SyncThread *pThread;

	pThread = NULL;

	for(safe_counter=0; safe_counter<SYNC_THREAD_MAX; safe_counter++)
	{
		if(thread_index >= SYNC_THREAD_MAX)
		{
			thread_index = 0;
		}

		pThread = &_sync_thread[thread_index];

		if(((find_new == dave_true) && (pThread->thread_name[0] == '\0'))
			|| (dave_strcmp(pThread->thread_name, thread_name) == dave_true))
		{
			if(find_new == dave_true)
			{
				_sync_server_reset_thread(pThread);

				dave_strcpy(pThread->thread_name, thread_name, SYNC_THREAD_NAME_LEN);
			}

			return pThread;
		}

		thread_index ++;
	}

	return NULL;
}

static SyncThread *
_sync_server_add_thread(SyncClient *pClient, s8 *thread_name, ub thread_index)
{
	SyncThread *pThread;

	pThread = _sync_server_find_thread(thread_name, thread_index, dave_false);
	if(pThread != NULL)
	{
		if(_sync_server_add_thread_client(pThread, pClient) == dave_false)
		{
			SYNCABNOR("Thread conflict, the thread<%s> exists on %d servers!",
				thread_name, SYNC_CLIENT_MAX);
		}
	}
	else
	{
		pThread = _sync_server_find_thread(thread_name, thread_index, dave_true);
		if(pThread == NULL)
		{
			SYNCABNOR("Thread pool resources are limited!<%s>", thread_name);
		}
		else
		{
			SYNCTRACE("verno:%s add thread:%s", pClient->verno, thread_name);

			dave_strcpy(pThread->thread_name, thread_name, SYNC_THREAD_NAME_LEN);

			_sync_server_add_thread_client(pThread, pClient);

			_sync_server_please_sync_all_thread_to_all_client();
		}
	}

	return pThread;
}

static ub
_sync_server_del_thread(SyncThread *pThread, SyncClient *pClient)
{
	ub client_number;

	client_number = _sync_server_del_thread_client(pThread, pClient);
	if(client_number == 0)
	{
		SYNCTRACE("verno:%s del thread:%s", pClient->verno, pThread->thread_name);

		_sync_server_reset_thread(pThread);
	}

	return client_number;
}

static void
_sync_server_data_client_reboot(void)
{
	ub client_index;
	SyncClient *pClient;
	SocketDisconnectReq *pReq;

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		pClient = &_sync_client[client_index];

		if(pClient->client_socket != INVALID_SOCKET_ID)
		{
			pReq = thread_reset_msg(pReq);

			pReq->socket = pClient->client_socket;
			pReq->ptr = pClient;
		
			write_msg(thread_id(SOCKET_THREAD_NAME), SOCKET_DISCONNECT_REQ, pReq);
		}
	}
}

static dave_bool
_sync_server_client_on_the_thread(SyncThread *pThread, SyncClient *pClient)
{
	ub client_index;

	if((pThread == NULL) || (pClient == NULL))
	{
		return dave_false;
	}

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		if(pThread->pClient[client_index] == pClient)
		{
			return dave_true;
		}
	}

	return dave_false;
}

static inline SyncClient *
_sync_server_find_effective_client(s8 *thread_name)
{
	ub thread_index = _sync_server_thread_name_to_index(thread_name);
	SyncThread *pThread;
	ub valid_index, client_index;

	pThread = _sync_server_find_thread(thread_name, thread_index, dave_false);
	if(pThread == NULL)
	{
		SYNCLOG("thread_name:%s", thread_name);
		return NULL;
	}

	valid_index = SYNC_CLIENT_MAX;

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		if(pThread->pClient[client_index] != NULL)
		{
			valid_index = client_index;

			if(pThread->pClient[client_index]->ready_flag == dave_true)
			{
				return pThread->pClient[client_index];
			}
		}
	}

	if(valid_index < SYNC_CLIENT_MAX)
	{
		return pThread->pClient[valid_index];
	}

	return NULL;
}

static SyncClient *
_sync_server_check_globally_identifier_conflict(SyncClient *pClient)
{
	ub client_index;
	SyncClient *pCheckClient;

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		pCheckClient = &(_sync_client[client_index]);

		if((pCheckClient != pClient)
			&& (pCheckClient->client_socket != INVALID_SOCKET_ID)
			&& (pClient->client_socket != INVALID_SOCKET_ID)
			&& (dave_strcmp(pCheckClient->globally_identifier, pClient->globally_identifier) == dave_true))
		{
			if((dave_memcmp(pCheckClient->NetInfo.addr.ip.ip_addr, pClient->NetInfo.addr.ip.ip_addr, 4) == dave_true)
				&& (pCheckClient->NetInfo.port == pClient->NetInfo.port))
			{
				SYNCLOG("The same service on the same machine initiates two connections, \
which is usually the reason for the delayed message processing.\
%s/%s/%s(%d%d%d) %s/%s/%s(%d%d%d)",
					pCheckClient->verno, pCheckClient->globally_identifier,
					ipv4str(pCheckClient->NetInfo.addr.ip.ip_addr, pCheckClient->NetInfo.port),
					pCheckClient->ready_flag, pCheckClient->blocks_flag, pCheckClient->client_flag,
					pClient->verno, pClient->globally_identifier,
					ipv4str(pClient->NetInfo.addr.ip.ip_addr, pClient->NetInfo.port),
					pClient->ready_flag, pClient->blocks_flag, pClient->client_flag);
				return pClient;
			}
			else
			{
				SYNCABNOR("This is a serious system conflict! You can use command: \
cfgset GLOBALLYIDENTIFIER [16Byte] to modify one of the globally identifier. \
%s/%s/%s(%d%d%d) %s/%s/%s(%d%d%d)",
					pCheckClient->verno, pCheckClient->globally_identifier,
					ipv4str(pCheckClient->NetInfo.addr.ip.ip_addr, pCheckClient->NetInfo.port),
					pCheckClient->ready_flag, pCheckClient->blocks_flag, pCheckClient->client_flag,
					pClient->verno, pClient->globally_identifier,
					ipv4str(pClient->NetInfo.addr.ip.ip_addr, pClient->NetInfo.port),
					pClient->ready_flag, pClient->blocks_flag, pClient->client_flag);

				return pCheckClient;
			}
		}
	}

	return NULL;
}

static void
_sync_server_my_verno_to_all_client(SyncClient *pMyClient)
{
	SyncClient *pOtherClient;
	ub client_index;

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		pOtherClient = &_sync_client[client_index];
	
		if((pOtherClient->client_socket != INVALID_SOCKET_ID)
			&& (pOtherClient != pMyClient)
			&& (pOtherClient->verno[0] != '\0'))
		{
			// sync my verno to other client.
			sync_server_tx_module_verno(pOtherClient, pMyClient->verno);

			// sync other verno to me.
			sync_server_tx_module_verno(pMyClient, pOtherClient->verno);
		}
	}
}

static void
_sync_server_run_test(SyncClient *pClient,
	ThreadId route_src, ThreadId route_dst,
	s8 *my_name, s8 *other_name,
	ub msg_id,
	BaseMsgType msg_type,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	ub msg_len, u8 *msg_body)
{
	SyncClient *pOtherClient;
	ub client_index;

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		pOtherClient = &_sync_client[client_index];
	
		if((pOtherClient->client_socket != INVALID_SOCKET_ID)
			&& (pOtherClient != pClient)
			&& (dave_strcmp(pOtherClient->verno, other_name) == dave_true))
		{
			sync_server_tx_test_run_thread_msg_req(
				pOtherClient,
				route_src, route_dst,
				my_name, other_name,
				src_attrib, dst_attrib,
				msg_id,
				msg_type,
				msg_body, msg_len);
		}
	}
}

static void
_sync_server_client_state(SyncClient *pClient, dave_bool idle)
{
	dave_bool state_change = dave_false;

	if(pClient->client_flag != idle)
	{
		state_change = dave_true;
	}

	pClient->client_flag = idle;

	if(state_change == dave_true)
	{
		sync_server_sync_auto_link(pClient);
	}
}

// =====================================================================

void
sync_server_data_init(void)
{
	t_lock_reset(&_sync_server_data_pv);

	_sync_server_reset_all_client();
	_sync_server_data_reset_all_index();
	_sync_server_reset_all_thread();	
}

void
sync_server_data_exit(void)
{
	SAFEZONEv5W(_sync_server_data_pv, _sync_server_data_client_reboot(););
}

SyncClient *
sync_server_add_client(s32 socket, SocNetInfo *pNetInfo)
{
	SyncClient *pClient = NULL;

	SAFEZONEv5W(_sync_server_data_pv, pClient = _sync_server_add_client(socket, pNetInfo); );

	return pClient;
}

void
sync_server_del_client(SyncClient *pClient)
{
	SAFEZONEv5W(_sync_server_data_pv, _sync_server_del_client(pClient); );
}

SyncClient *
sync_server_find_client(s32 socket)
{
	ub index_index = socket % SYNC_FAST_INDEX_MAX;
	SyncClient *pClient = NULL;

	if((_socket_fast_index[index_index].pClient != NULL)
		&& (_socket_fast_index[index_index].pClient->client_socket == socket))
	{
		return _socket_fast_index[index_index].pClient;
	}

	SAFEZONEv5R(_sync_server_data_pv, {

		pClient = _sync_server_find_client(socket, dave_false);

		if(_sync_server_data_build_socket_fast_index(socket) != pClient)
		{
			SYNCABNOR("If an abnormality is found, is there a competition problem? %s/%s",
				pClient->globally_identifier, pClient->verno);
		}

	} );

	return pClient;
}

SyncThread *
sync_server_add_thread(SyncClient *pClient, s8 *thread_name)
{
	ub thread_index = _sync_server_thread_name_to_index(thread_name);
	SyncThread *pThread = NULL;

	SAFEZONEv5W(_sync_server_data_pv, { pThread = _sync_server_add_thread(pClient, thread_name, thread_index); } );

	return pThread;
}

ub
sync_server_del_thread(SyncThread *pThread, SyncClient *pClient)
{
	ub client_number = SYNC_CLIENT_MAX;

	SAFEZONEv5W(_sync_server_data_pv, { client_number = _sync_server_del_thread(pThread, pClient); } );

	return client_number;
}

SyncThread *
sync_server_find_thread(s8 *thread_name)
{
	ub thread_index = _sync_server_thread_name_to_index(thread_name);
	SyncThread *pThread = NULL;

	SAFEZONEv5R(_sync_server_data_pv, { pThread = _sync_server_find_thread(thread_name, thread_index, dave_false); } );

	return pThread;
}

dave_bool
sync_server_client_on_the_thread(SyncThread *pThread, SyncClient *pClient)
{
	dave_bool ret = dave_false;

	SAFEZONEv5R(_sync_server_data_pv, ret = _sync_server_client_on_the_thread(pThread, pClient););

	return ret;
}

SyncClient *
sync_server_find_effective_client(s8 *thread_name)
{
	SyncClient *pClient = NULL;

	SAFEZONEv5R(_sync_server_data_pv, pClient = _sync_server_find_effective_client(thread_name););

	return pClient;
}

SyncClient *
sync_server_check_globally_identifier_conflict(SyncClient *pClient)
{
	SyncClient *pConflictClient = NULL;

	SAFEZONEv5R(_sync_server_data_pv, pConflictClient = _sync_server_check_globally_identifier_conflict(pClient););

	return pConflictClient;
}

void
sync_server_my_verno_to_all_client(SyncClient *pMyClient)
{
	SAFEZONEv5R(_sync_server_data_pv, _sync_server_my_verno_to_all_client(pMyClient););
}

void
sync_server_run_test(SyncClient *pClient,
	ThreadId route_src, ThreadId route_dst,
	s8 *my_name, s8 *other_name,
	ub msg_id,
	BaseMsgType msg_type,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	ub msg_len, u8 *msg_body)
{
	SAFEZONEv5R(_sync_server_data_pv, _sync_server_run_test(pClient, route_src, route_dst, my_name, other_name, msg_id, msg_type, src_attrib, dst_attrib, msg_len, msg_body););
}

void
sync_server_client_state(SyncClient *pClient, dave_bool idle)
{
	SAFEZONEv3(pClient->opt_pv, _sync_server_client_state(pClient, idle););
}

SyncClient *
sync_server_client(ub client_index)
{
	if(client_index >= SYNC_CLIENT_MAX)
		return NULL;

	return &_sync_client[client_index];
}

SyncThread *
sync_server_thread(ub thread_index)
{
	if(thread_index >= SYNC_THREAD_MAX)
		return NULL;

	return &_sync_thread[thread_index];
}

#endif

