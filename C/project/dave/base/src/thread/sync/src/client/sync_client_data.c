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
#include "sync_tools.h"
#include "sync_client_param.h"
#include "sync_client_tools.h"
#include "sync_client_data.h"
#include "sync_client_link.h"
#include "sync_client_thread.h"
#include "sync_client_thread_ready_remove.h"
#include "sync_cfg.h"
#include "sync_test.h"
#include "sync_lock.h"
#include "sync_log.h"

#define SYNC_SERVER_INDEX 0
#define SYNC_FAST_INDEX_MAX (DAVE_SERVER_SUPPORT_SOCKET_MAX * 2)

typedef struct {
	SyncServer *pServer;
} SyncClientSocketFastIndex;

TLock _sync_client_data_pv;
static ub _chose_server_loop_index = 0;
static SyncServer _server_data[SERVER_DATA_MAX];
static SyncClientSocketFastIndex _socket_fast_index[SYNC_FAST_INDEX_MAX];
static LinkThread _link_thread[SYNC_THREAD_MAX];
static s8 _local_thread_name[SYNC_THREAD_MAX][SYNC_THREAD_NAME_LEN];

static LinkThread * _sync_client_data_thread_del(SyncServer *pServer, s8 *thread_name, ub thread_index);

static void
_sync_client_data_reset_server_(SyncServer *pServer)
{
	pServer->server_type = SyncServerType_max;
	pServer->server_socket = INVALID_SOCKET_ID;

	pServer->server_connecting = dave_false;
	pServer->server_cnt = dave_false;
	pServer->server_booting = dave_false;
	pServer->server_ready = dave_false;

	pServer->left_timer = SYNC_SERVER_LEFT_MAX;
	pServer->reconnect_times = SYNC_RECONNECT_TIMES;
	pServer->booting_reciprocal = SYNC_BOOTING_RECIPROCAL;

	pServer->recv_data_counter = 0;
	pServer->send_data_counter = 0;

	pServer->sync_thread_name_number = 0;
	pServer->sync_thread_name_index = 0;

	pServer->server_send_message_counter = 0;
	pServer->server_recv_message_counter = 0;

	pServer->verno[0] = '\0';
	pServer->globally_identifier[0] = '\0';
	pServer->rpc_version = 3;
	pServer->work_start_second = 0;

	pServer->cfg_server_ip[0] = 0; pServer->cfg_server_ip[1] = 0; pServer->cfg_server_ip[2] = 0; pServer->cfg_server_ip[3] = 0;
	pServer->cfg_server_port = 0;
	pServer->child_ip[0] = 0; pServer->child_ip[1] = 0; pServer->child_ip[2] = 0; pServer->child_ip[3] = 0;
	pServer->child_port = 0;

	if(pServer->server_index == SYNC_SERVER_INDEX)
	{
		pServer->shadow_index = SYNC_SERVER_INDEX;
	}
	else
	{
		pServer->shadow_index = SERVER_DATA_MAX;
	}
}

static void
_sync_client_data_reset_all_server(void)
{
	ub server_index;

	dave_memset(_server_data, 0x00, sizeof(_server_data));

	for(server_index=0; server_index<SERVER_DATA_MAX; server_index++)
	{
		_server_data[server_index].server_index = server_index;

		t_lock_reset(&(_server_data[server_index].rxtx_pv));

		_sync_client_data_reset_server_(&_server_data[server_index]);
	}
}

static void
_sync_client_data_reset_index(SyncClientSocketFastIndex *pIndex)
{
	pIndex->pServer = NULL;
}

static void
_sync_client_data_reset_all_index(void)
{
	ub index_index;

	dave_memset(_socket_fast_index, 0x00, sizeof(_socket_fast_index));

	for(index_index=0; index_index<SYNC_FAST_INDEX_MAX; index_index++)
	{
		_sync_client_data_reset_index(&_socket_fast_index[index_index]);
	}
}

static SyncServer *
_sync_client_data_build_socket_fast_index(s32 socket)
{
	ub index_index = socket % SYNC_FAST_INDEX_MAX;
	ub server_index;

	if(_socket_fast_index[index_index].pServer != NULL)
	{
		if(_socket_fast_index[index_index].pServer->server_socket == socket)
		{
			return _socket_fast_index[index_index].pServer;
		}

		_socket_fast_index[index_index].pServer = NULL;
	}

	for(server_index=0; server_index<SERVER_DATA_MAX; server_index++)
	{
		if(_server_data[server_index].server_socket == socket)
		{
			if(index_index < SYNC_FAST_INDEX_MAX)
			{
				_socket_fast_index[index_index].pServer = &_server_data[server_index];
			}

			return &_server_data[server_index];
		}
	}

	return NULL;
}

static void
_sync_client_data_reset_thread(LinkThread *pThread)
{
	ub server_index;

	dave_memset(pThread->thread_name, 0x00, sizeof(pThread->thread_name));

	pThread->chose_server_index = 0;

	for(server_index=0; server_index<SERVER_DATA_MAX; server_index++)
	{
		pThread->pServer[server_index] = NULL;
		pThread->shadow_index_ready_remove_flag[server_index] = dave_false;
		pThread->shadow_index_ready_remove_counter[server_index] = 0;
	}

	pThread->thread_send_message_counter = 0;
	pThread->thread_recv_message_counter = 0;
}

static void
_sync_client_data_reset_thread_all(void)
{
	ub thread_index;

	dave_memset(_link_thread, 0x00, sizeof(_link_thread));

	for(thread_index=0; thread_index<SYNC_THREAD_MAX; thread_index++)
	{
		t_lock_reset(&(_link_thread[thread_index].chose_server_pv));

		_link_thread[thread_index].thread_index = thread_index;

		_sync_client_data_reset_thread(&_link_thread[thread_index]);
	}
}

static SyncServer *
_sync_client_data_sync_server(void)
{
	SyncServer *pServer = &_server_data[SYNC_SERVER_INDEX];

	pServer->server_type = SyncServerType_sync_client;

	return pServer;
}

static void
_sync_client_data_reset_sync_server(void)
{
	SyncServer *pServer = _sync_client_data_sync_server();

	sync_cfg_get_syncs_ip(pServer->cfg_server_ip);

	pServer->cfg_server_port = sync_cfg_get_syncs_port();	
}

static SyncServer *
_sync_client_data_find_server_on_net(u8 *ip, u16 port, SyncServerType server_type)
{
	ub server_index;

	for(server_index=0; server_index<SERVER_DATA_MAX; server_index++)
	{
		if((_server_data[server_index].server_type == server_type)
			&& (_server_data[server_index].cfg_server_port == port)
			&& (_server_data[server_index].cfg_server_ip[0] == ip[0])
			&& (_server_data[server_index].cfg_server_ip[1] == ip[1])
			&& (_server_data[server_index].cfg_server_ip[2] == ip[2])
			&& (_server_data[server_index].cfg_server_ip[3] == ip[3]))
		{
			return &_server_data[server_index];
		}
	}

	return NULL;
}

static LinkThread *
_sync_client_data_find_thread(s8 *thread_name, ub thread_index, dave_bool find_new)
{
	ub safe_counter;
	LinkThread *pThread;

	if(thread_name == NULL)
	{
		return NULL;
	}

	pThread = NULL;

	for(safe_counter=0; safe_counter<SYNC_THREAD_MAX; safe_counter++)
	{
		if(thread_index >= SYNC_THREAD_MAX)
		{
			thread_index = 0;
		}

		pThread = &_link_thread[thread_index];

		if(((find_new == dave_true) && (pThread->thread_name[0] == '\0'))
			|| (dave_strcmp(pThread->thread_name, thread_name) == dave_true))
		{
			if(find_new == dave_true)
			{
				dave_strcpy(pThread->thread_name, thread_name, SYNC_THREAD_NAME_LEN);
			}

			return pThread;
		}

		thread_index ++;
	}

	return NULL;
}

static dave_bool
_sync_client_data_the_server_on_the_thread(SyncServer *pServer, LinkThread *pThread)
{
	ub server_index;

	for(server_index=0; server_index<SERVER_DATA_MAX; server_index++)
	{
		if(pThread->pServer[server_index] == pServer)
			return dave_true;
	}

	return dave_false;
}

static dave_bool
_sync_client_data_thread_has_server(LinkThread *pThread)
{
	ub server_index;

	for(server_index=0; server_index<SERVER_DATA_MAX; server_index++)
	{
		if(pThread->pServer[server_index] != NULL)
		{
			return dave_true;
		}
	}

	return dave_false;
}

static void
_sync_client_data_add_server_to_thread(LinkThread *pThread, SyncServer *pServer)
{
	ub server_index, safe_counter, empty_index = SERVER_DATA_MAX;

	server_index = pServer->server_index;

	for(safe_counter=0; safe_counter<SERVER_DATA_MAX; safe_counter++)
	{
		if(server_index >= SERVER_DATA_MAX)
		{
			SYNCABNOR("Arithmetic error! %d %s/%s", server_index, pThread->thread_name, pServer->verno);
			server_index = 0;
		}

		if(pThread->pServer[server_index] == pServer)
		{
			empty_index = server_index;
			break;
		}

		if((pThread->pServer[server_index] == NULL) && (empty_index == SERVER_DATA_MAX))
		{
			empty_index = server_index;
		}
	}

	if(empty_index < SERVER_DATA_MAX)
	{
		pThread->pServer[empty_index] = pServer;

		SYNCDEBUG("add pServer:%x to pThread:%x server_index:%d", pServer, pThread, empty_index);
	}
	else
	{
		SYNCABNOR("thread:%s verno:%s can't add!", pThread->thread_name, pServer->verno);
	}
}

static dave_bool
_sync_client_data_thread_del_server_from_thread(LinkThread *pThread, SyncServer *pServer)
{
	ub del_counter, server_index;
	dave_bool ret = dave_false;

	del_counter = 0;

	for(server_index=0; server_index<SERVER_DATA_MAX; server_index++)
	{
		if(pThread->pServer[server_index] == pServer)
		{
			SYNCDEBUG("del pServer:%d/%s from pThread:%s server_index:%d",
				pServer->server_type, pServer->verno,
				pThread->thread_name,
				server_index);

			del_counter ++;
			pThread->pServer[server_index] = NULL;
			ret = dave_true;
		}
	}

	if(del_counter >= 2)
	{
		SYNCABNOR("find error! %s on %s/%s was deleted %d times",
			pThread->thread_name,
			pServer->globally_identifier, pServer->verno,
			del_counter);
	}

	return ret;
}

static void
_sync_client_data_del_server_on_all_thread(SyncServer *pServer)
{
	ub thread_index;

	SYNCTRACE("verno:%s server_type:%s", pServer->verno, sync_client_type_to_str(pServer->server_type));

	for(thread_index=0; thread_index<SYNC_THREAD_MAX; thread_index++)
	{
		if((_link_thread[thread_index].thread_name[0] != '\0')
			&& (_sync_client_data_the_server_on_the_thread(pServer, &_link_thread[thread_index]) == dave_true))
		{
			_sync_client_data_thread_del(pServer, _link_thread[thread_index].thread_name, sync_client_data_thread_name_to_index(_link_thread[thread_index].thread_name));
		}
	}
}

static SyncServer *
_sync_client_data_server_add_client(s8 *verno, s8 *globally_identifier, u8 *ip, u16 port)
{
	SyncServer *pServer;
	ub safe_counter;

	pServer = _sync_client_data_find_server_on_net(ip, port, SyncServerType_client);
	if(pServer != NULL)
	{
		SYNCTRACE("repeat add! verno:%s ip:%s", verno, ipv4str(ip, port));
		return pServer;
	}

	for(safe_counter=0; safe_counter<SERVER_DATA_MAX; safe_counter++)
	{
		if(_chose_server_loop_index >= SERVER_DATA_MAX)
		{
			_chose_server_loop_index = 0; 
		}
		pServer = &_server_data[_chose_server_loop_index ++];

		if((pServer->server_type == SyncServerType_max)
			&& (pServer->server_socket == INVALID_SOCKET_ID)
			&& (pServer->cfg_server_port == 0))
		{
			/*
			 * 设置cfg_server_port不为0，等待 _sync_client_guard_time -> _sync_client_connect_all 的动作，
			 * 来启动连接行为。
			 */

			SYNCTRACE("pServer:%x/%x/%x %s/%s %s",
				pServer, pServer->server_index, pServer->shadow_index,
				globally_identifier, verno,
				ipv4str(ip, port));

			pServer->server_type = SyncServerType_client;
			dave_strcpy(pServer->verno, verno, sizeof(pServer->verno));
			dave_strcpy(pServer->globally_identifier, globally_identifier, sizeof(pServer->globally_identifier));
			dave_memcpy(pServer->cfg_server_ip, ip, sizeof(pServer->cfg_server_ip));
			pServer->cfg_server_port = port;

			return pServer;
		}
	}

	SYNCABNOR("Please increase the SERVER_DATA_MAX(%d) parameter!", SERVER_DATA_MAX);

	return NULL;
}

static SyncServer *
_sync_client_data_server_del_client(u8 *ip, u16 port)
{
	SyncServer *pServer;

	pServer = _sync_client_data_find_server_on_net(ip, port, SyncServerType_client);
	if(pServer != NULL)
	{
		/*
		 * 设置cfg_server_port为0，如果是已经连接的套接字，
		 * 等待 _sync_client_guard_time -> _sync_client_disconnect_all 的动作，
		 * 来启动断开行为。
		 */
		if(pServer->server_socket == INVALID_SOCKET_ID)
		{
			_sync_client_data_reset_server_(pServer);
		}

		SYNCTRACE("socket:%d pServer:%x/%x/%x %s/%s %s",
			pServer->server_socket,
			pServer, pServer->server_index, pServer->shadow_index,
			pServer->globally_identifier, pServer->verno,
			ipv4str(pServer->cfg_server_ip, pServer->cfg_server_port));

		_sync_client_data_del_server_on_all_thread(pServer);

		pServer->cfg_server_port = 0;
	}

	return NULL;
}

static SyncServer *
_sync_client_data_server_add_child(s32 socket, u8 *ip, u16 port)
{
	SyncServer *pServer;
	ub safe_counter;

	for(safe_counter=0; safe_counter<SERVER_DATA_MAX; safe_counter++)
	{
		if(_chose_server_loop_index >= SERVER_DATA_MAX)
		{
			_chose_server_loop_index = 0; 
		}
		pServer = &_server_data[_chose_server_loop_index ++];

		if((pServer->server_socket == INVALID_SOCKET_ID)
			&& (pServer->cfg_server_port == 0))
		{
			pServer->server_type = SyncServerType_child;
			pServer->server_socket = socket;
			pServer->server_connecting = dave_false;

			dave_memcpy(pServer->child_ip, ip, sizeof(pServer->child_ip));
			pServer->child_port = port;

			SYNCTRACE("socket:%d pServer:%x/%x/%x %s",
				socket, pServer, pServer->server_index, pServer->shadow_index,
				ipv4str(ip, port));

			return pServer;
		}
	}

	SYNCABNOR("Please increase the SERVER_DATA_MAX(%d) parameter!", SERVER_DATA_MAX);

	return NULL;
}

static SyncServer *
_sync_client_data_server_del_child(s32 socket)
{
	ub server_index;
	SyncServer *pRetServer, *pServer;

	pRetServer = NULL;

	for(server_index=0; server_index<SERVER_DATA_MAX; server_index++)
	{
		pServer = &_server_data[server_index];

		if((pServer->server_type == SyncServerType_child)
			&& (pServer->server_socket == socket)
			&& (pServer->cfg_server_port == 0))
		{
			SYNCTRACE("socket:%d pServer:%x/%x/%x %d%d%d%d l:%d/%d %s/%s %s",
				socket,
				pServer, pServer->server_index, pServer->shadow_index,
				pServer->server_connecting, pServer->server_cnt, pServer->server_booting, pServer->server_ready,
				pServer->left_timer, pServer->reconnect_times,
				pServer->globally_identifier, pServer->verno,
				ipv4str(pServer->child_ip, pServer->child_port));

			_sync_client_data_del_server_on_all_thread(pServer);

			pServer->server_connecting = dave_false;
			pServer->server_cnt = dave_false;
			pServer->server_booting = dave_false;
			pServer->server_ready = dave_false;

			pServer->shadow_index = SERVER_DATA_MAX;

			if(pRetServer == NULL)
			{
				pRetServer = pServer;
			}
			else
			{
				SYNCABNOR("the socket:%d has two server!", socket);
			}
		}
	}

	return pRetServer;
}

static SyncServer *
_sync_client_data_server_inq_on_net(u8 *ip, u16 port)
{
	SyncServer *pServer;

	pServer = _sync_client_data_find_server_on_net(ip, port, SyncServerType_sync_client);
	if(pServer == NULL)
	{
		pServer = _sync_client_data_find_server_on_net(ip, port, SyncServerType_client);
	}

	return pServer;
}

static LinkThread *
_sync_client_data_thread_add(SyncServer *pServer, s8 *thread_name, ub thread_index)
{
	LinkThread *pThread;

	pThread = _sync_client_data_find_thread(thread_name, thread_index, dave_false);
	if(pThread == NULL)
	{
		pThread = _sync_client_data_find_thread(thread_name, thread_index, dave_true);
	}
	if(pThread == NULL)
	{
		SYNCABNOR("Please increase the SYNC_THREAD_MAX(%d) parameter!", SYNC_THREAD_MAX);
		return NULL;
	}

	SYNCTRACE("pServer:%x pThread:%x socket:%d verno:%s thread:%s",
		pServer, pThread,
		pServer->server_socket,
		pServer->verno,
		thread_name);

	_sync_client_data_add_server_to_thread(pThread, pServer);

	sync_client_thread_add(pServer, pThread->thread_name);

	sync_client_thread_ready(pServer, pThread);

	return pThread;
}

static LinkThread *
_sync_client_data_thread_del(SyncServer *pServer, s8 *thread_name, ub thread_index)
{
	LinkThread *pThread;

	pThread = _sync_client_data_find_thread(thread_name, thread_index, dave_false);
	if(pThread == NULL)
	{
		/*
		 * 如果线程结构已经被删除了，那么调用sync_client_thread_del
		 * 时用线程名的方式删除线程。
		 * 来自SYNC或者对端服务的删除动作，存在多次调用线程删除的情况存在。
		 */
		sync_client_thread_del(pServer, thread_name);

		return NULL;
	}

	SYNCTRACE("pServer:%x pThread:%x/%s socket:%d verno:%s",
		pServer, pThread, thread_name,
		pServer->server_socket,
		pServer->verno);

	if(_sync_client_data_thread_del_server_from_thread(pThread, pServer) == dave_true)
	{
		sync_client_thread_remove(pServer, pThread);
	}

	if(_sync_client_data_thread_has_server(pThread) == dave_true)
	{
		SYNCTRACE("the %s has server , do't remove!", pThread->thread_name);

		if(pThread->pServer[pServer->server_index] != NULL)
		{
			SYNCABNOR("Arithmetic error! %x/%s  %x/%d %x/%d %d/%d",
				pThread, pThread->thread_name,
				pThread->pServer[pServer->server_index], pThread->pServer[pServer->server_index]->server_index,
				pServer, pServer->server_index,
				pThread->shadow_index_ready_remove_flag[pServer->server_index],
				pThread->shadow_index_ready_remove_counter[pServer->server_index]);
		}
		pThread->shadow_index_ready_remove_flag[pServer->server_index] = dave_false;
		pThread->shadow_index_ready_remove_counter[pServer->server_index] = 0;

		return pThread;
	}

	sync_client_thread_del(pServer, thread_name);

	_sync_client_data_reset_thread(pThread);

	return pThread;
}

static void
_sync_client_data_thread_check(void)
{
	ub thread_index1, thread_index2;

	for(thread_index1=0; thread_index1<SYNC_THREAD_MAX; thread_index1++)
	{
		for(thread_index2=thread_index1+1; thread_index2<SYNC_THREAD_MAX; thread_index2++)
		{
			if((_link_thread[thread_index1].thread_name[0] != '\0')
				&& (_link_thread[thread_index2].thread_name[0] != '\0'))
			{
				if(dave_strcmp(_link_thread[thread_index1].thread_name,
					_link_thread[thread_index2].thread_name) == dave_true)
				{
					SYNCABNOR("find same thread:%s", _link_thread[thread_index1].thread_name);
				}
			}
		}
	}
}

static LinkThread *
_sync_client_data_thread_on_name(s8 *thread_name, ub thread_index)
{
	LinkThread *pThread;

	pThread = _sync_client_data_find_thread(thread_name, thread_index, dave_false);
	if(pThread == NULL)
	{
		SYNCTRACE("You send a message to the remote, \
but the source(%s) of the message was not registered as a remote thread.", thread_name);
		return NULL;
	}

	return pThread;
}

static ub
_sync_client_data_thread_index_on_name(s8 *thread_name)
{
	LinkThread *pThread;

	SYNCDEBUG("thread:%s", thread_name);

	pThread = _sync_client_data_thread_on_name(thread_name, SYNC_THREAD_INDEX_MAX);
	if(pThread == NULL)
	{
		return SYNC_THREAD_INDEX_MAX;
	}

	return pThread->thread_index;
}

static void
_sync_client_data_reset_server(SyncServer *pServer, dave_bool clean_flag)
{
	SyncServerType server_type;
	s32 server_socket;
	sb left_timer, reconnect_times;
	s8 verno[DAVE_VERNO_STR_LEN];
	s8 globally_identifier[DAVE_GLOBALLY_IDENTIFIER_LEN];
	u8 cfg_server_ip[16];
	u16 cfg_server_port;

	SYNCTRACE("verno:%s server_socket:%d server_type:%s reconnect_times:%d cfg_link:%s",
		pServer->verno,
		pServer->server_socket,
		sync_client_type_to_str(pServer->server_type),
		pServer->reconnect_times,
		ipv4str(pServer->cfg_server_ip, pServer->cfg_server_port));

	server_socket = pServer->server_socket;	

	if(clean_flag == dave_false)
	{
		server_type = pServer->server_type;
		left_timer = pServer->left_timer;
		reconnect_times = pServer->reconnect_times;
		dave_strcpy(verno, pServer->verno, sizeof(verno));
		dave_strcpy(globally_identifier, pServer->globally_identifier, sizeof(globally_identifier));
		dave_memcpy(cfg_server_ip, pServer->cfg_server_ip, sizeof(cfg_server_ip));
		cfg_server_port = pServer->cfg_server_port;
	}

	_sync_client_data_reset_server_(pServer);

	if(clean_flag == dave_false)
	{
		/*
		 * 与SYNC服务器相关的参数需要保留，这样保留参数后，
		 * sync_client.c 的 _sync_client_connect_all
		 * 就会自动去重试建立与SYNC的链路。
		 * *** 目前的设计是，如果链路断开后，没有SYNC参与，在 SYNC_RECONNECT_TIMES 次数后，
		 * *** 其他服务就不自动建立链路。
		 * *** 如果不通过SYNC集中管控，一个服务断开后，其链路端口很可能会更换，其他服务
		 * *** 不方便获取到更新的端口信息。
		 */

		if(server_type == SyncServerType_sync_client)
		{
			reconnect_times = SYNC_RECONNECT_TIMES;
		}

		if(((server_type == SyncServerType_sync_client) || (server_type == SyncServerType_client))
			&& ((-- reconnect_times) > 0)
			&& (cfg_server_port != 0))
		{
			SYNCDEBUG("server_type:%s ip:%s",
				sync_client_type_to_str(server_type),
				ipv4str(cfg_server_ip, cfg_server_port));

			pServer->server_type = server_type;
			pServer->reconnect_times = reconnect_times;
			dave_strcpy(pServer->verno, verno, sizeof(pServer->verno));
			dave_strcpy(pServer->globally_identifier, globally_identifier, sizeof(pServer->globally_identifier));
			dave_memcpy(pServer->cfg_server_ip, cfg_server_ip, sizeof(cfg_server_ip));
			pServer->cfg_server_port = cfg_server_port;
		}
		else
		{
			SYNCTRACE("socket:%d pServer:%x type:%s %s/%s times:%d/%d %s link has been recycled.",
				server_socket, pServer,
				sync_client_type_to_str(server_type),
				globally_identifier, verno,
				left_timer, reconnect_times,
				ipv4str(cfg_server_ip, cfg_server_port));
		}
	}
}

// =====================================================================

void
sync_client_data_init(void)
{
	t_lock_reset(&_sync_client_data_pv);
	dave_memset(_server_data, 0x00, sizeof(_server_data));
	dave_memset(_socket_fast_index, 0x00, sizeof(_socket_fast_index));
	dave_memset(_link_thread, 0x00, sizeof(_link_thread));
	dave_memset(_local_thread_name, 0x00, sizeof(_local_thread_name));

	sync_client_data_thread_local_reset();

	_sync_client_data_reset_all_server();

	_sync_client_data_reset_all_index();

	_sync_client_data_reset_sync_server();

	_sync_client_data_reset_thread_all();

	sync_client_thread_ready_remove_init();
}

void
sync_client_data_exit(void)
{
	sync_client_thread_ready_remove_exit();
}

void
sync_client_data_reset_sync_server(void)
{
	_sync_client_data_reset_sync_server();
}

SyncServer *
sync_client_data_sync_server(void)
{
	return _sync_client_data_sync_server();
}

SyncServer *
sync_client_data_head_server(void)
{
	return &_server_data[0];
}

LinkThread *
sync_client_data_head_thread(void)
{
	return &_link_thread[0];
}

dave_bool
sync_client_data_all_server_is_disconnect(void)
{
	ub server_index;

	for(server_index=0; server_index<SERVER_DATA_MAX; server_index++)
	{
		if(_server_data[server_index].server_socket != INVALID_SOCKET_ID)
		{
			return dave_false;
		}
	}

	return dave_true;
}

SyncServer *
sync_client_data_server_inq_on_socket(s32 socket)
{
	ub index_index = socket % SYNC_FAST_INDEX_MAX;
	SyncServer *pServer;

	if((_socket_fast_index[index_index].pServer != NULL)
		&& (_socket_fast_index[index_index].pServer->server_socket == socket))
	{
		return _socket_fast_index[index_index].pServer;
	}

	pServer = NULL;

	SAFEZONEv5R(_sync_client_data_pv, pServer = _sync_client_data_build_socket_fast_index(socket); );

	return pServer;
}

SyncServer *
sync_client_data_server_inq_on_net(u8 *ip, u16 port)
{
	SyncServer *pServer = NULL;

	SAFEZONEv5R(_sync_client_data_pv, pServer = _sync_client_data_server_inq_on_net(ip, port); );

	return pServer;
}

SyncServer *
sync_client_data_server_inq_on_index(ub server_index)
{
	if(server_index >= SERVER_DATA_MAX)
	{
		return NULL;
	}

	return &_server_data[server_index];
}

LinkThread *
sync_client_data_thread_on_name(s8 *thread_name, ub thread_index)
{
	LinkThread *pThread = NULL;

	if(thread_index == SYNC_THREAD_INDEX_MAX)
	{
		thread_index = sync_client_data_thread_name_to_index(thread_name);
	}

	SAFEZONEv5R(_sync_client_data_pv, pThread = _sync_client_data_thread_on_name(thread_name, thread_index); );

	return pThread;
}

ub
sync_client_data_thread_index_on_name(s8 *thread_name)
{
	ub thread_index = SYNC_THREAD_INDEX_MAX;

	SAFEZONEv5R(_sync_client_data_pv, thread_index = _sync_client_data_thread_index_on_name(thread_name); );

	return thread_index;
}

ub
sync_client_data_thread_local_reset(void)
{
	return base_thread_name_array(_local_thread_name, SYNC_THREAD_MAX);
}

s8 *
sync_client_data_thread_local_name(ub thread_index)
{
	if(thread_index >= SYNC_THREAD_MAX)
	{
		SYNCABNOR("invalid thread_index:%d/%d", thread_index, SYNC_THREAD_MAX);
		return _local_thread_name[0];
	}

	return _local_thread_name[thread_index];
}

ub
sync_client_data_thread_local_index(s8 *thread_name)
{
	ub thread_index;

	for(thread_index=0; thread_index<SYNC_THREAD_MAX; thread_index++)
	{
		if(dave_strcmp(thread_name, _local_thread_name[thread_index]) == dave_true)
		{
			return thread_index;
		}
	}

	return SYNC_THREAD_MAX;
}

void
sync_client_data_reset_server(SyncServer *pServer, dave_bool clean_flag)
{
	SAFEZONEv5W(_sync_client_data_pv, _sync_client_data_reset_server(pServer, clean_flag); );
}

SyncServer *
sync_client_data_server_add_client(s8 *verno, s8 *globally_identifier, u8 *ip, u16 port)
{
	SyncServer *pServer = NULL;

	if(((ip[0] == 0) && (ip[1] == 0) && (ip[2] == 0) && (ip[3] == 0))
		|| (port == 0))
	{
		SYNCLOG("invalid %s!", ipv4str(ip, port));
		return NULL;
	}

	SAFEZONEv5W(_sync_client_data_pv, { pServer = _sync_client_data_server_add_client(verno, globally_identifier, ip, port); } );

	return pServer;
}

SyncServer *
sync_client_data_server_del_client(s8 *verno, s8 *globally_identifier, u8 *ip, u16 port)
{
	SyncServer *pServer = NULL;

	SAFEZONEv5W(_sync_client_data_pv, { pServer = _sync_client_data_server_del_client(ip, port); } );

	return pServer;
}

SyncServer *
sync_client_data_server_add_child(s32 socket, u8 *ip, u16 port)
{
	SyncServer *pServer = NULL;

	SAFEZONEv5W(_sync_client_data_pv, { pServer = _sync_client_data_server_add_child(socket, ip, port); } );

	return pServer;
}

SyncServer *
sync_client_data_server_del_child(s32 socket)
{
	SyncServer *pServer = NULL;

	SAFEZONEv5W(_sync_client_data_pv, { pServer = _sync_client_data_server_del_child(socket); } );

	return pServer;
}

LinkThread *
sync_client_data_thread_add(SyncServer *pServer, s8 *thread_name)
{
	ub thread_index = sync_client_data_thread_name_to_index(thread_name);
	LinkThread *pThread = NULL;

	SAFEZONEv5W(_sync_client_data_pv, { pThread = _sync_client_data_thread_add(pServer, thread_name, thread_index); } );

	_sync_client_data_thread_check();

	return pThread;
}

LinkThread *
sync_client_data_thread_del(SyncServer *pServer, s8 *thread_name)
{
	ub thread_index = sync_client_data_thread_name_to_index(thread_name);
	LinkThread *pThread = NULL;

	SYNCTRACE("%d/%s thread:%s", pServer->server_type, pServer->verno, thread_name);

	SAFEZONEv5W(_sync_client_data_pv, { pThread = _sync_client_data_thread_del(pServer, thread_name, thread_index); } );

	return pThread;
}

void
sync_client_data_del_server_on_all_thread(SyncServer *pServer)
{
	SYNCTRACE("verno:%s server_type:%d", pServer->verno, pServer->server_type);

	SAFEZONEv5W(_sync_client_data_pv, _sync_client_data_del_server_on_all_thread(pServer); );
}

SyncServer *
sync_client_server(ub server_index)
{
	if(server_index >= SERVER_DATA_MAX)
		return NULL;

	return &_server_data[server_index];
}

LinkThread *
sync_client_thread(ub thread_index)
{
	if(thread_index >= SYNC_THREAD_MAX)
		return NULL;

	return &_link_thread[thread_index];
}

#endif

