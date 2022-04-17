/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SOCKET_EXTERNAL_H__
#define __SOCKET_EXTERNAL_H__
#include "base_macro.h"
#include "socket_core.h"

void socket_external_init(void);

void socket_external_exit(void);

SocketCore * socket_external_creat_service(ThreadId src, SocNetInfo *pNetInfo, void *user_ptr);

SocketCore * socket_external_connect_service(ThreadId src, SocNetInfo *pNetInfo, void *user_ptr, SOCKETINFO *ConnectInfo);

RetCode socket_external_close(ThreadId src, s32 socket);

dave_bool socket_external_send(ThreadId src, s32 socket, IPBaseInfo *pIPInfo, MBUF *data, SOCKETINFO snd_flag);

void socket_external_notify(SocketNotify *pNotify);

void socket_external_event(SocketRawEvent *pEvent);

#endif

