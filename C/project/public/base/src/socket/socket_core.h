/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SOCKET_CORE_H__
#define __SOCKET_CORE_H__
#include "base_macro.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "socket_parameters.h"

typedef struct {
	IPBaseInfo IPInfo;
	MBUF *data;
	SOCKETINFO snd_flag;

	void *next;
} SokcetSndList;

typedef struct {
	TLock opt_pv_for_snd;

	dave_bool snd_token;
	sb snd_list_depth;
	SokcetSndList *snd_head;
	SokcetSndList *snd_tail;
} SocketCoreSndList;

typedef struct {
	dave_bool use_flag;

	TLock opt_pv;

	s32 socket_external_index;
	s32 socket_internal_index;

	SOCKETTYPE type;
	ThreadId owner;
	void *user_ptr;
	s32 os_socket;
	SocNetInfo NetInfo;

	SocketCoreSndList snd_list;

	ub data_recv_length;
	ub data_send_length;

	ub tcp_recv_buf_length;
	dave_bool bind_or_connect_rsp_flag;	// 用来BIND或CONNECT的应答信号与PlugIn信号的同步。
	dave_bool wait_close;
} SocketCore;

void socket_core_init(void);

void socket_core_exit(void);

SocketCore * __socket_core_malloc__(ThreadId owner, SOCKETTYPE type, SocNetInfo *pNetInfo, void *user_ptr, s32 os_socket, s8 *fun, ub line);
#define socket_core_malloc(owner, type, pNetInfo, user_ptr, os_socket) __socket_core_malloc__(owner, type, pNetInfo, user_ptr, os_socket, (s8 *)__func__, (ub)__LINE__)

void __socket_core_free__(SocketCore *pCore, s8 *fun, ub line);
#define socket_core_free(pCore) __socket_core_free__(pCore, (s8 *)__func__, (ub)__LINE__)

SocketCore * socket_core_find(s32 socket_external_index);

SocketCore * socket_core_external_find(s32 socket_external_index, s32 os_socket);

ub socket_core_info(s8 *info_ptr, ub info_len, ub page_id, s8 *page_thread);

#endif

