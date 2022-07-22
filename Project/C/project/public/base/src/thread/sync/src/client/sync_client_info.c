/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_CLIENT)
#include "dave_tools.h"
#include "dave_verno.h"
#include "dave_base.h"
#include "dave_os.h"
#include "base_tools.h"
#include "sync_param.h"
#include "sync_tools.h"
#include "sync_client_data.h"
#include "sync_client_link.h"
#include "sync_client_tools.h"
#include "sync_client_msg_buffer.h"
#include "sync_log.h"

typedef struct {
	s8 product_str[64];
	ub product_number;
} ServerInfoStatistics;

static ub
_sync_client_info_show_server(SyncServer *pServer, s8 *info, ub info_len)
{
	ub current_second = dave_os_time_s(), work_on_second;
	ub info_index = 0;
	s8 second_str[32];

	if(pServer->work_start_second == 0)
		work_on_second = 0;
	else
		work_on_second = current_second - pServer->work_start_second;
	
	info_index += dave_snprintf(&info[info_index], info_len-info_index,
		" %x %s s:%04x %d%d%d%d l:%02d/%02d i:%02x T:%s s-%lu:%lu/r-%lu:%lu %s/%s/%d %s\n",
		pServer,
		sync_client_type_to_str(pServer->server_type), pServer->server_socket,
		pServer->server_connecting, pServer->server_cnt, pServer->server_booting, pServer->server_ready,
		pServer->left_timer, pServer->reconnect_times,
		pServer->server_index,
		sync_work_start_second_str(work_on_second, second_str, sizeof(second_str)),
		pServer->send_data_counter, pServer->server_send_message_counter,
		pServer->recv_data_counter, pServer->server_recv_message_counter,
		pServer->globally_identifier, pServer->verno, pServer->rpc_version,
		pServer->server_type == SyncServerType_child ? ipv4str(pServer->child_ip, pServer->child_port) : ipv4str(pServer->cfg_server_ip, pServer->cfg_server_port));

	return info_index;
}

static void
_sync_client_info_statistics_reset(ServerInfoStatistics *pStatistics_ptr, ub pStatistics_num)
{
	dave_memset(pStatistics_ptr, 0x00, sizeof(ServerInfoStatistics) * pStatistics_num);
}

static ServerInfoStatistics *
_sync_client_info_find_new_statistics(ServerInfoStatistics *pStatistics_ptr, ub pStatistics_num)
{
	ub server_index, pStatistics_index, pStatistics_empty = pStatistics_num;
	SyncServer *pServer;
	s8 product_str[64];

	for(server_index=0; server_index<pStatistics_num; server_index++)
	{
		pServer = sync_client_server(server_index);
		if((pServer->server_ready == dave_true) && (pServer->verno[0] != '\0'))
		{
			dave_verno_product(pServer->verno, product_str, sizeof(product_str));
			if((product_str[0] != '\0')
				&& (dave_strcmp(product_str, "DAVE") == dave_false))
			{
				for(pStatistics_index=0; pStatistics_index<pStatistics_num; pStatistics_index++)
				{
					if(dave_strcmp(pStatistics_ptr[pStatistics_index].product_str, product_str) == dave_true)
					{
						break;
					}

					if(pStatistics_ptr[pStatistics_index].product_str[0] == '\0')
					{
						pStatistics_empty = pStatistics_index;
					}
				}

				if((pStatistics_empty < pStatistics_num)
					&& (pStatistics_index >= pStatistics_num))
				{
					dave_strcpy(pStatistics_ptr[pStatistics_empty].product_str, product_str, sizeof(pStatistics_ptr[pStatistics_empty].product_str));
					pStatistics_ptr[pStatistics_empty].product_number = 0;
					return &pStatistics_ptr[pStatistics_empty];
				}
			}
		}
	}

	return NULL;
}

static ServerInfoStatistics *
_sync_client_info_find_new_number(ServerInfoStatistics *pStatistics)
{
	ub server_index;
	SyncServer *pServer;
	s8 product_str[64];

	pStatistics->product_number = 0;

	for(server_index=0; server_index<SERVER_DATA_MAX; server_index++)
	{
		pServer = sync_client_server(server_index);
		if(pServer->verno[0] != '\0')
		{
			dave_verno_product(pServer->verno, product_str, sizeof(product_str));
			if(dave_strcmp(product_str, pStatistics->product_str) == dave_true)
			{
				pStatistics->product_number ++;
			}
		}
	}

	return pStatistics;
}

static ServerInfoStatistics *
_sync_client_info_find_new(ServerInfoStatistics *pStatistics_ptr, ub pStatistics_num)
{
	ServerInfoStatistics *pStatistics;

	pStatistics = _sync_client_info_find_new_statistics(pStatistics_ptr, pStatistics_num);
	if(pStatistics == NULL)
		return NULL;

	return _sync_client_info_find_new_number(pStatistics);
}

static ub
_sync_client_info_ready_type_show(ServerInfoStatistics *pStatistics, SyncServerType server_type, s8 *info, ub info_len)
{
	ub info_index = 0;
	ub server_index;
	SyncServer *pServer;
	s8 product_str[64];

	for(server_index=0; server_index<SERVER_DATA_MAX; server_index++)
	{
		pServer = sync_client_server(server_index);
		if((pServer->server_socket != INVALID_SOCKET_ID)
			&& (pServer->verno[0] != '\0')
			&& (pServer->server_type == server_type))
		{
			dave_verno_product(pServer->verno, product_str, sizeof(product_str));
			if(dave_strcmp(product_str, pStatistics->product_str) == dave_true)
			{
				info_index += _sync_client_info_show_server(pServer, &info[info_index], info_len-info_index);
			}
		}
	}

	return info_index;
}

static ub
_sync_client_info_ready_show(ServerInfoStatistics *pStatistics, s8 *info, ub info_len)
{
	ub info_index = 0;

	if(pStatistics == NULL)
		return 0;

	info_index += dave_snprintf(&info[info_index], info_len-info_index,
		"%s LINK (%d):\n",
		pStatistics->product_str, pStatistics->product_number);

	info_index += _sync_client_info_ready_type_show(pStatistics, SyncServerType_sync_client, &info[info_index], info_len-info_index);
	info_index += _sync_client_info_ready_type_show(pStatistics, SyncServerType_client, &info[info_index], info_len-info_index);
	info_index += _sync_client_info_ready_type_show(pStatistics, SyncServerType_child, &info[info_index], info_len-info_index);

	return info_index;
}

static ub
_sync_client_info_ready(s8 *info, ub info_len)
{
	ServerInfoStatistics pStatistics_ptr[SERVER_DATA_MAX];
	ServerInfoStatistics *pStatistics;
	ub server_index, info_index = 0;

	_sync_client_info_statistics_reset(pStatistics_ptr, SERVER_DATA_MAX);

	for(server_index=0; server_index<SERVER_DATA_MAX; server_index++)
	{
		pStatistics = _sync_client_info_find_new(pStatistics_ptr, SERVER_DATA_MAX);
		if(pStatistics == NULL)
			break;

		info_index += _sync_client_info_ready_show(pStatistics, &info[info_index], info_len-info_index);
	}

	return info_index;
}

static ub
_sync_client_info_not_ready(s8 *info, ub info_len)
{
	ub server_index, info_index = 0;
	SyncServer *pServer;
	dave_bool has_not_ready = dave_false;

	info_index += dave_snprintf(&info[info_index], info_len-info_index,
		"*** NOT READY LINK:\n");

	for(server_index=0; server_index<SERVER_DATA_MAX; server_index++)
	{
		pServer = sync_client_server(server_index);
		if(((pServer->server_socket != INVALID_SOCKET_ID) && (pServer->verno[0] == '\0'))
			|| (((pServer->server_socket == INVALID_SOCKET_ID) || (pServer->verno[0] == '\0'))
					&& (pServer->server_type == SyncServerType_sync_client))
			|| ((pServer->server_socket != INVALID_SOCKET_ID) && (pServer->server_ready == dave_false)))
		{
			info_index += _sync_client_info_show_server(pServer, &info[info_index], info_len-info_index);

			has_not_ready = dave_true;
		}
	}

	if(has_not_ready == dave_false)
		return 0;

	return info_index;
}

static ub
_sync_client_info_thread(s8 *info, ub info_len)
{
	ub info_index = 0, server_index, thread_index;
	LinkThread *pThread;

	info_index += dave_snprintf(&info[info_index], info_len-info_index, "REMOTE TOTAL THREAD:\n");

	for(thread_index=0; thread_index<SYNC_THREAD_MAX; thread_index++)
	{
		pThread = sync_client_thread(thread_index);
		if(pThread->thread_name[0] != '\0')
		{
			info_index += dave_snprintf(&info[info_index], info_len-info_index,
				" %x/%03x s-%06ld/r-%06ld",
				pThread, pThread->thread_index,
				pThread->thread_send_message_counter, pThread->thread_recv_message_counter);

			for(server_index=0; server_index<SERVER_DATA_MAX; server_index++)
			{
				if(pThread->pServer[server_index] != NULL)
				{
					info_index += dave_snprintf(&info[info_index], info_len-info_index,
						" %x",
						pThread->pServer[server_index]);
				}
			}

			info_index += dave_snprintf(&info[info_index], info_len-info_index, "->%s\n", pThread->thread_name);
		}
	}

	return info_index;
}

static ub
_sync_client_info_data(s8 *info, ub info_len)
{
	ub info_index = 0;

	info_index += _sync_client_info_ready(&info[info_index], info_len-info_index);
	info_index += _sync_client_info_not_ready(&info[info_index], info_len-info_index);
	info_index += _sync_client_info_thread(&info[info_index], info_len-info_index);

	if((info_index > 0) && (info[info_index - 1] != '\n'))
	{
		info_index += dave_snprintf(&info[info_index], info_len-info_index, "\n");
	}

	return info_index;
}

// =====================================================================

ub
sync_client_info(s8 *info, ub info_len)
{
	ub info_index;

	info_index = 0;

	info_index += _sync_client_info_data(&info[info_index], info_len-info_index);
	info_index += sync_client_msg_buffer_info(&info[info_index], info_len-info_index);
	info_index += sync_client_link_info(&info[info_index], info_len-info_index);
	info_index += dave_snprintf(&info[info_index], info_len-info_index, "Globally Identifier:%s\n", globally_identifier());

	return info_index;
}

#endif

