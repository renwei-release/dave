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
#include "base_tools.h"
#include "sync_param.h"
#include "sync_tools.h"
#include "sync_server_param.h"
#include "sync_server_data.h"
#include "sync_server_blocks.h"
#include "sync_server_msg_buffer.h"
#include "sync_server_broadcadt.h"
#include "sync_log.h"

typedef struct {
	s8 product_str[64];
	ub product_number;
} ClientInfoStatistics;

static ub
_sync_server_info_show_client(SyncClient *pClient, s8 *info, ub info_len)
{
	ub current_second = dave_os_time_s(), work_on_second;
	ub info_index = 0;
	s8 second_str[32];

	if(pClient->work_start_second == 0)
		work_on_second = 0;
	else
		work_on_second = current_second - pClient->work_start_second;
	
	info_index += dave_snprintf(&info[info_index], info_len-info_index,
		" B:%02d %d%d%d%d%d T:%s s-%lu:%lu/r-%lu:%lu %s/%s/%d C:%s L:%s %d/%d\n",
		sync_server_blocks_index_to_blocks_id(pClient->client_index),
		pClient->receive_thread_done, pClient->sync_thread_flag, pClient->ready_flag, pClient->blocks_flag, pClient->client_flag,
		sync_work_start_second_str(work_on_second, second_str, sizeof(second_str)),
		pClient->send_data_counter, pClient->send_msg_counter,
		pClient->recv_data_counter, pClient->recv_msg_counter,
		pClient->globally_identifier, pClient->verno, pClient->rpc_version,
		ipv4str(pClient->NetInfo.addr.ip.ip_addr, pClient->NetInfo.port),
		ipv4str2(pClient->link_ip, pClient->link_port),
		pClient->left_timer, pClient->sync_timer);

	return info_index;
}

static void
_sync_server_info_statistics_reset(ClientInfoStatistics *pStatistics_ptr, ub pStatistics_num)
{
	dave_memset(pStatistics_ptr, 0x00, sizeof(ClientInfoStatistics) * pStatistics_num);
}

static ClientInfoStatistics *
_sync_server_info_find_new_statistics(ClientInfoStatistics *pStatistics_ptr, ub pStatistics_num)
{
	ub client_index, pStatistics_index, pStatistics_empty = pStatistics_num;
	SyncClient *pClient;
	s8 product_str[64];

	for(client_index=0; client_index<pStatistics_num; client_index++)
	{
		pClient = sync_server_client(client_index);
		if(pClient->verno[0] != '\0')
		{
			dave_verno_product(pClient->verno, product_str, sizeof(product_str));
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

static ClientInfoStatistics *
_sync_server_info_find_new_number(ClientInfoStatistics *pStatistics)
{
	ub client_index;
	SyncClient *pClient;
	s8 product_str[64];

	pStatistics->product_number = 0;

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		pClient = sync_server_client(client_index);
		if(pClient->verno[0] != '\0')
		{
			dave_verno_product(pClient->verno, product_str, sizeof(product_str));
			if(dave_strcmp(product_str, pStatistics->product_str) == dave_true)
			{
				pStatistics->product_number ++;
			}
		}
	}

	return pStatistics;
}

static ClientInfoStatistics *
_sync_server_info_find_new(ClientInfoStatistics *pStatistics_ptr, ub pStatistics_num)
{
	ClientInfoStatistics *pStatistics;

	pStatistics = _sync_server_info_find_new_statistics(pStatistics_ptr, pStatistics_num);
	if(pStatistics == NULL)
		return NULL;

	return _sync_server_info_find_new_number(pStatistics);
}

static ub
_sync_server_info_ready_show(ClientInfoStatistics *pStatistics, s8 *info, ub info_len)
{
	ub info_index = 0;
	ub client_index;
	SyncClient *pClient;
	s8 product_str[64];

	if(pStatistics == NULL)
		return 0;

	info_index += dave_snprintf(&info[info_index], info_len-info_index,
		"%s INFORMATION (%d):\n",
		pStatistics->product_str, pStatistics->product_number);

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		pClient = sync_server_client(client_index);
		if((pClient->client_socket != INVALID_SOCKET_ID)
			&& (pClient->verno[0] != '\0'))
		{
			dave_verno_product(pClient->verno, product_str, sizeof(product_str));
			if(dave_strcmp(product_str, pStatistics->product_str) == dave_true)
			{
				info_index += _sync_server_info_show_client(pClient, &info[info_index], info_len-info_index);
			}
		}
	}

	return info_index;
}

static ub
_sync_server_info_ready(s8 *info, ub info_len)
{
	ClientInfoStatistics pStatistics_ptr[SYNC_CLIENT_MAX];
	ClientInfoStatistics *pStatistics;
	ub client_index, info_index = 0;

	_sync_server_info_statistics_reset(pStatistics_ptr, SYNC_CLIENT_MAX);

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		pStatistics = _sync_server_info_find_new(pStatistics_ptr, SYNC_CLIENT_MAX);
		if(pStatistics == NULL)
			break;

		info_index += _sync_server_info_ready_show(pStatistics, &info[info_index], info_len-info_index);
	}

	return info_index;
}

static ub
_sync_server_info_not_ready(s8 *info, ub info_len)
{
	ub client_index, info_index = 0;
	SyncClient *pClient;
	dave_bool has_not_ready = dave_false;

	info_index += dave_snprintf(&info[info_index], info_len-info_index,
		"*** NOT READY INFORMATION:\n");

	for(client_index=0; client_index<SYNC_CLIENT_MAX; client_index++)
	{
		pClient = sync_server_client(client_index);
		if((pClient->client_socket != INVALID_SOCKET_ID)
			&& (pClient->verno[0] == '\0'))
		{
			info_index += _sync_server_info_show_client(pClient, &info[info_index], info_len-info_index);

			has_not_ready = dave_true;
		}
	}

	if(has_not_ready == dave_false)
		return 0;

	return info_index;
}

static ub
_sync_server_info_thread(s8 *info, ub info_len)
{
	ub info_index = 0, thread_index;
	ub printf_length;
	SyncThread *pThread;

	info_index += dave_snprintf(&info[info_index], info_len-info_index, "SYSTEM TOTAL THREAD:\n");

	for(thread_index=0; thread_index<SYNC_THREAD_MAX; thread_index++)
	{
		pThread = sync_server_thread(thread_index);

		if(pThread->thread_name[0] != '\0')
		{
			printf_length = dave_snprintf(&info[info_index], info_len-info_index, " %s", pThread->thread_name);
			info_index += printf_length;
			
			info_index += dave_snprintf(&info[info_index], info_len-info_index, "\t%s", printf_length < 8 ? "\t" : "");

			info_index += dave_snprintf(&info[info_index], info_len-info_index,
				"s-%09lu/r-%09lu\n",
				pThread->thread_send_message_counter,
				pThread->thread_recv_message_counter);
		}
	}

	return info_index;
}

static ub
_sync_server_info_data(s8 *info, ub info_len)
{
	ub info_index = 0;

	info_index += _sync_server_info_ready(&info[info_index], info_len-info_index);
	info_index += _sync_server_info_not_ready(&info[info_index], info_len-info_index);
	info_index += _sync_server_info_thread(&info[info_index], info_len-info_index);

	if((info_index > 0) && (info[info_index - 1] != '\n'))
	{
		info_index += dave_snprintf(&info[info_index], info_len-info_index, "\n");
	}

	return info_index;
}

// =====================================================================

ub
sync_server_info(s8 *info, ub info_len)
{
	ub info_index;

	info_index = 0;

	info_index += _sync_server_info_data(&info[info_index], info_len-info_index);
	info_index += sync_server_msg_buffer_info(&info[info_index], info_len-info_index);
	info_index += sync_server_broadcadt_info(&info[info_index], info_len-info_index);
	info_index += dave_snprintf(&info[info_index], info_len-info_index, "Globally Identifier:%s", globally_identifier());

	return info_index;
}

#endif

