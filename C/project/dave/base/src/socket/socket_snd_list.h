/*
 * ================================================================================
 * (c) Copyright 2021 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2021.12.03.
 * ================================================================================
 */

#ifndef __SOCKET_SND_LIST_H__
#define __SOCKET_SND_LIST_H__
#include "base_macro.h"
#include "socket_core.h"

void socket_snd_list_reset(SocketCore *pCore);

void socket_snd_list_init(SocketCore *pCore);

void socket_snd_list_exit(SocketCore *pCore);

void socket_snd_list_clean(SocketCore *pCore);

dave_bool socket_snd_list_catch_snd_token(SocketCore *pCore, IPBaseInfo *pIPInfo, MBUF *data, SOCKETINFO snd_flag);

void socket_snd_list_release_snd_token(SocketCore *pCore);

SokcetSndList * socket_snd_list_catch_data(SocketCore *pCore);

void socket_snd_list_release_data(SokcetSndList *pSndData);

#endif

