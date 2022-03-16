/*
 * ================================================================================
 * (c) Copyright 2020 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.08.06.
 * ================================================================================
 */

#ifndef __SYNC_SERVER_RX_H__
#define __SYNC_SERVER_RX_H__

void sync_server_rx_read(SyncClient *pClient, SocketRead *pRead);

void sync_server_rx_event(SyncClient *pClient, SocketRawEvent *pEvent);

void sync_server_rx_version(SyncClient *pClient);

void sync_server_rx_link_up(SyncClient *pClient);

void sync_server_rx_link_down(SyncClient *pClient);

#endif

