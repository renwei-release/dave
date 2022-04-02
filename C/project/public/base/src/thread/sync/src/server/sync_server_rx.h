/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_SERVER_RX_H__
#define __SYNC_SERVER_RX_H__

void sync_server_rx_read(SyncClient *pClient, SocketRead *pRead);

void sync_server_rx_event(SyncClient *pClient, SocketRawEvent *pEvent);

void sync_server_rx_version(SyncClient *pClient);

void sync_server_rx_link_up(SyncClient *pClient);

void sync_server_rx_link_down(SyncClient *pClient);

#endif

