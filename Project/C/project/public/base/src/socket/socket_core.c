/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_tools.h"
#include "base_tools.h"
#include "socket_core.h"
#include "socket_tools.h"
#include "socket_parameters.h"
#include "socket_snd_list.h"
#include "socket_log.h"

typedef struct {
	s32 os_socket;

	SocketCore *pCore;
} SocketMap;

static volatile s32 _malloc_socket_index = 0;
static TLock _socket_opt_pv;
static SocketCore _socket_core[SOCKET_MAX];
static void *_socket_map_ramkv = NULL;

static inline ub
_socket_core_total_number(s8 *page_thread)
{
	ub socket_internal_index;
	SocketCore *pCore;
	ub total_number = 0;

	for(socket_internal_index=0; socket_internal_index<SOCKET_MAX; socket_internal_index++)
	{
		pCore = &_socket_core[socket_internal_index];

		if(pCore->use_flag == dave_true)
		{
			if((page_thread == NULL)
				|| (dave_strcmp(thread_name(pCore->owner), page_thread) == dave_true))
			{
				total_number ++;
			}
		}
	}

	return total_number;
}

static inline ub
_socket_core_map_number(s8 *page_thread)
{
	ub index, map_number;
	SocketMap *pMap;
	SocketCore *pCore;

	map_number = 0;

	for(index=0; index<SOCKET_MAX*2; index++)
	{
		pMap = base_ramkv_inq_index_ptr(_socket_map_ramkv, index);
		if(pMap == NULL)
			break;

		pCore = pMap->pCore;

		if(pCore->use_flag == dave_true)
		{
			if((page_thread == NULL)
				|| (dave_strcmp(thread_name(pCore->owner), page_thread) == dave_true))
			{
				map_number ++;
			}
		}

	}

	return map_number;
}

static inline void
_socket_core_reset(SocketCore *pCore)
{
	s32 socket_internal_index = pCore->socket_internal_index;

	pCore->use_flag = dave_false;

	pCore->socket_internal_index = socket_internal_index;
	pCore->type = SOCKET_TYPE_MAX;
	pCore->owner = INVALID_THREAD_ID;
	pCore->user_ptr = NULL;
	pCore->os_socket = INVALID_SOCKET_ID;
	dave_memset(&(pCore->NetInfo), 0x00, sizeof(SocNetInfo));

	socket_snd_list_reset(pCore);

	pCore->data_recv_length = pCore->data_send_length = 0;

	pCore->tcp_recv_buf_length = SOCKET_TCP_RECV_MIN_BUF;
	pCore->bind_or_connect_rsp_flag = dave_false;
	pCore->wait_close = dave_false;
}

static inline void
_socket_core_map_free(s32 os_socket, SocketCore *pCore)
{
	SocketMap *pMap;

	if(os_socket >= 0)
	{
		pMap = base_ramkv_inq_ub_ptr(_socket_map_ramkv, (ub)os_socket);
		if((pMap != NULL) && ((pMap->pCore == pCore) || (pCore == NULL)))
		{
			pMap = base_ramkv_del_ub_ptr(_socket_map_ramkv, (ub)os_socket);
			if(pMap != NULL)
			{
				dave_free(pMap);
			}
		}
	}
}

static inline void
_socket_core_map_malloc(s32 os_socket, SocketCore *pCore)
{
	SocketMap *pMap;

	if(os_socket >= 0)
	{
		pMap = base_ramkv_inq_ub_ptr(_socket_map_ramkv, (ub)os_socket);
		if(pMap != NULL)
		{
			/*
			 * 有可能会发生系统套接字os_socket已经无效，
			 * 但socket线程还没来得及处理时，
			 * 系统又把os_socket套接字分配给新请求了。
			 */
			SOCKETLOG("os_socket:%d/%d has pMap:%x pCore:%x",
				os_socket, pMap->pCore == NULL ? INVALID_SOCKET_ID : pMap->pCore->os_socket,
				pMap, pMap->pCore);

			_socket_core_map_free(os_socket, NULL);
		}

		pMap = dave_malloc(sizeof(SocketMap));
		pMap->os_socket = os_socket;
		pMap->pCore = pCore;

		if(pCore->os_socket != INVALID_SOCKET_ID)
		{
			SOCKETLOG("the socket:%d/%d has os_socket:%d/%d",
				pCore->socket_external_index, pCore->socket_internal_index,
				pCore->os_socket, os_socket);
		}

		pCore->os_socket = os_socket;

		base_ramkv_add_ub_ptr(_socket_map_ramkv, (ub)os_socket, pMap);
	}
}

static inline SocketMap *
_socket_core_map_find(s32 os_socket)
{
	if(os_socket < 0)
		return NULL;

	return (SocketMap *)base_ramkv_inq_ub_ptr(_socket_map_ramkv, (ub)os_socket);
}

static inline RetCode
_socket_core_map_clean(void *ramkv, s8 *key)
{
	SocketMap *pMap = base_ramkv_del_key_ptr(_socket_map_ramkv, key);

	if(pMap == NULL)
	{
		return RetCode_empty_data;
	}

	dave_free(pMap);

	return RetCode_OK;
}

static inline SocketCore *
_socket_core_malloc(ThreadId owner, SOCKETTYPE type, SocNetInfo *pNetInfo, void *user_ptr, s32 os_socket, s8 *fun, ub line)
{
	s32 socket_external_index, socket_internal_index;
	ub safe_counter;
	SocketCore *pCore = NULL;

	for(safe_counter=0; safe_counter<SOCKET_MAX; safe_counter++)
	{
		if(_malloc_socket_index >= 0x7fffffff)
		{
			_malloc_socket_index = 0;
		}
		socket_external_index = _malloc_socket_index ++;
		socket_internal_index = socket_external_index % SOCKET_MAX;

		if(_socket_core[socket_internal_index].use_flag == dave_false)
		{
			pCore = &(_socket_core[socket_internal_index]);
			break;
		}
	}

	if((safe_counter >= SOCKET_MAX) || (pCore == NULL))
	{
		SOCKETABNOR("malloc socket fail(%d)! os_socket:%d <%s:%d>",
			SOCKET_MAX, os_socket,
			fun, line);
		return NULL;
	}

	SAFECODEv2W(pCore->opt_pv, {

		pCore->use_flag = dave_true;

		/*
		 * 反馈给外部的socket可以是
		 * socket_external_index （范围大，可以防止冲突，但不方便应用按socket来做索引）
		 * 也可以是
		 * socket_internal_index （范围小，方便应用按socket来做索引）
		 */
		pCore->socket_external_index = socket_external_index;

		pCore->type = type;
		pCore->owner = owner;
		pCore->user_ptr = user_ptr;
		pCore->os_socket = INVALID_SOCKET_ID;
		T_CopyNetInfo(&(pCore->NetInfo), pNetInfo);

		socket_snd_list_init(pCore);

		pCore->data_recv_length = pCore->data_send_length = 0;

		pCore->tcp_recv_buf_length = SOCKET_TCP_RECV_MIN_BUF;
		if(type == SOCKET_TYPE_SERVER_CHILD)
			pCore->bind_or_connect_rsp_flag = dave_true;
		else
			pCore->bind_or_connect_rsp_flag = dave_false;
		pCore->wait_close = dave_false;

		_socket_core_map_malloc(os_socket, pCore);

	} );

	SOCKETDEBUG("owner:%s socket:%d/%d os_socket:%d <%s:%d>",
		thread_name(owner),
		pCore->socket_external_index, pCore->socket_internal_index,
		pCore->os_socket,
		fun, line);

	return pCore;
}

static inline void
_socket_core_free(SocketCore *pCore, s8 *fun, ub line)
{
	if(pCore != NULL)
	{
		SOCKETDEBUG("owner:%s socket:%d/%d os_socket:%d <%s:%d>",
			thread_name(pCore->owner),
			pCore->socket_external_index, pCore->socket_internal_index,
			pCore->os_socket,
			fun, line);

		if(pCore->use_flag != dave_true)
		{
			SOCKETLOG("unuse socket:%d/%d! <%s:%d>",
				pCore->socket_external_index, pCore->socket_internal_index,
				fun, line);
		}
		else
		{
			socket_snd_list_exit(pCore);

			_socket_core_map_free(pCore->os_socket, pCore);

			_socket_core_reset(pCore);
		}
	}
}

static inline SocketCore *
_socket_core_find(s32 socket_external_index)
{
	s32 socket_internal_index = socket_external_index % SOCKET_MAX;
	SocketCore *pCore;

	pCore = &_socket_core[socket_internal_index];

	if((pCore->use_flag == dave_false)
		&& (pCore->socket_external_index != socket_external_index))
	{
		return NULL;
	}

	return pCore;
}

static inline SocketCore *
_socket_core_external_find(s32 socket_external_index, s32 os_socket)
{
	SocketMap *pMap;

	pMap = _socket_core_map_find(os_socket);
	if(pMap == NULL)
	{
		SOCKETTRACE("can't find the os_socket:%d", os_socket);
		return NULL;
	}

	if(pMap->pCore == NULL)
	{
		SOCKETLOG("os_socket:%d is close!", os_socket);
		return NULL;
	}

	if(pMap->pCore->use_flag == dave_false)
	{
		SOCKETLOG("os_socket:%d not use:%d/%d!",
			os_socket,
			pMap->pCore->use_flag, pMap->pCore->wait_close);
		return NULL;
	}

	if((socket_external_index >= 0) && (pMap->pCore->socket_external_index != socket_external_index))
	{
		SOCKETLOG("socket:%d/%d is close! os_socket:%d/%d/%d",
			pMap->pCore->socket_external_index, pMap->pCore->socket_internal_index,
			pMap->os_socket, pMap->pCore->os_socket, os_socket);
		return NULL;
	}

	if((pMap->os_socket != os_socket) || (pMap->pCore->os_socket != os_socket))
	{
		SOCKETLOG("socket:%d/%d is close! os_socket:%d/%d/%d",
			pMap->pCore->socket_external_index, pMap->pCore->socket_internal_index,
			pMap->os_socket, pMap->pCore->os_socket, os_socket);
		return NULL;
	}

	return pMap->pCore;
}

static inline SocketCore *
_socket_core_safe_malloc(ThreadId owner, SOCKETTYPE type, SocNetInfo *pNetInfo, void *user_ptr, s32 os_socket, s8 *fun, ub line)
{
	SocketCore *pCore = NULL;

	SAFECODEv2W(_socket_opt_pv, { pCore = _socket_core_malloc(owner, type, pNetInfo, user_ptr, os_socket, fun, line); } );

	return pCore;
}

static inline void
_socket_core_safe_free(SocketCore *pCore, s8 *fun, ub line)
{
	SAFECODEv2W(_socket_opt_pv, { _socket_core_free(pCore, fun, line); } );
}

static inline SocketCore *
_socket_core_safe_find(s32 socket_external_index)
{
	SocketCore *pCore = NULL;

	if(socket_external_index < 0)
	{
		SOCKETABNOR("invalid socket:%d", socket_external_index);
		return NULL;
	}

	SAFECODEv2R(_socket_opt_pv, { pCore = _socket_core_find(socket_external_index); } );

	return pCore;
}

static inline SocketCore *
_socket_core_external_safe_find(s32 socket_external_index, s32 os_socket)
{
	SocketCore *pCore = NULL;

	SAFECODEv2R(_socket_opt_pv, { pCore = _socket_core_external_find(socket_external_index, os_socket); } );

	return pCore;
}

// =====================================================================

void
socket_core_init(void)
{
	s32 socket_internal_index;

	_malloc_socket_index = 0;

	t_lock_reset(&_socket_opt_pv);

	for(socket_internal_index=0; socket_internal_index<SOCKET_MAX; socket_internal_index++)
	{
		dave_memset(&_socket_core[socket_internal_index], 0x00, sizeof(SocketCore));

		t_lock_reset(&(_socket_core[socket_internal_index].opt_pv));
		t_lock_reset(&(_socket_core[socket_internal_index].snd_list.opt_pv_for_snd));

		_socket_core[socket_internal_index].socket_internal_index = socket_internal_index;

		_socket_core_reset(&_socket_core[socket_internal_index]);
	}

	_socket_map_ramkv = kv_malloc((s8 *)"socketmap", 0, NULL);
}

void
socket_core_exit(void)
{
	s32 socket_internal_index;

	SAFECODEv2W(_socket_opt_pv, {

		for(socket_internal_index=0; socket_internal_index<SOCKET_MAX; socket_internal_index++)
		{
			_socket_core_map_free(_socket_core[socket_internal_index].os_socket, NULL);
		}

	} );

	base_ramkv_free(_socket_map_ramkv, _socket_core_map_clean);

	_socket_map_ramkv = NULL;
}

SocketCore *
__socket_core_malloc__(ThreadId owner, SOCKETTYPE type, SocNetInfo *pNetInfo, void *user_ptr, s32 os_socket, s8 *fun, ub line)
{
	SocketCore *pCore;

	pCore = _socket_core_safe_malloc(owner, type, pNetInfo, user_ptr, os_socket, fun, line);

	if(pCore == NULL)
	{
		SOCKETABNOR("error on owner:%s type:%d os_socket:%d <%s:%d>",
			thread_name(owner), type, os_socket,
			fun, line);
		return NULL;
	}

	return pCore;
}

void
__socket_core_free__(SocketCore *pCore, s8 *fun, ub line)
{
	if(pCore == NULL)
	{
		return;
	}

	_socket_core_safe_free(pCore, fun, line);
}

SocketCore *
socket_core_find(s32 socket_external_index)
{
	return _socket_core_safe_find(socket_external_index);
}

SocketCore *
socket_core_external_find(s32 socket_external_index, s32 os_socket)
{
	return _socket_core_external_safe_find(socket_external_index, os_socket);
}

ub
socket_core_info(s8 *info_ptr, ub info_len, ub page_id, s8 *page_thread)
{
#define PAGE_ID_MAX 32
	ub info_index = 0;
	ub total_number, map_number, index_number, current_page_id, page_number, socket_internal_index;
	SocketCore *pCore;

	total_number = _socket_core_total_number(page_thread);
	map_number = _socket_core_map_number(page_thread);
	index_number = 0;
	current_page_id = 0;
	page_number = total_number / PAGE_ID_MAX + ((total_number % PAGE_ID_MAX) == 0 ? 0 : 1);
	if(page_id > page_number)
	{
		page_id = page_number;
	}

	info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
		"SOCKET INFORMATION:\n");

	for(socket_internal_index=0; socket_internal_index<SOCKET_MAX; socket_internal_index++)
	{
		pCore = &_socket_core[socket_internal_index];

		if((pCore->use_flag == dave_true)
			&& ((page_thread == NULL) || (dave_strcmp(thread_name(pCore->owner), page_thread) == dave_true)))
		{
			index_number ++;
			current_page_id = index_number / PAGE_ID_MAX + ((index_number % PAGE_ID_MAX) == 0 ? 0 : 1);
			if((page_id != 0) && (current_page_id > page_id))
			{
				break;
			}
		
			if((page_id == 0) || (page_id == current_page_id))
			{
				info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
					" %x %x %s s-%08ld/r-%08ld %08d/%05d/%05d %06d %06d %s/%s %s\n",
					pCore, _socket_core_map_find(pCore->os_socket),
					socket_SOCKETTYPE_str(pCore->type),
					pCore->data_send_length, pCore->data_recv_length,
					pCore->socket_external_index, pCore->socket_internal_index, pCore->os_socket,
					pCore->snd_list.snd_list_depth, pCore->tcp_recv_buf_length,
					ipv4str(pCore->NetInfo.addr.ip.ip_addr, pCore->NetInfo.port),
					ipv4str2(pCore->NetInfo.src_ip.ip_addr, pCore->NetInfo.src_port),
					thread_name(pCore->owner));
			}
		}
	}
	info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
		" -------------------\n");
	if(page_id > 0)
	{
		info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
			" page:%d/%d total:%d/%d\n",
			page_id, page_number, total_number, map_number);
	}
	else
	{
		info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
			" total:%d/%d\n",
			total_number, map_number);
	}

	return info_index;
}

#endif

