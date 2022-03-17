/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_CLIENT_LINK_H__
#define __SYNC_CLIENT_LINK_H__

void sync_client_link_init(void);

void sync_client_link_exit(void);

void sync_client_link_start(void);

void sync_client_link_stop(void);

void sync_client_link_up(s8 *verno, s8 *globally_identifier, u8 *ip, u16 port);

void sync_client_link_down(s8 *verno, s8 *globally_identifier, u8 *ip, u16 port);

SyncServer * sync_client_link_plugin(SocketPlugIn *pPlugIn);

SyncServer * sync_client_link_plugout(SocketPlugOut *pPlugOut);

ub sync_client_link_info(s8 *info, ub info_len);

void sync_client_link_tell_sync_server(u8 *detect_my_ip);

#endif

