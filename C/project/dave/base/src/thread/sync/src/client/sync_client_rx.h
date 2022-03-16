/*
 * ================================================================================
 * (c) Copyright 2020 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.09.03.
 * ================================================================================
 */

#ifndef __SYNC_CLIENT_RX_H__
#define __SYNC_CLIENT_RX_H__

void sync_client_rx_read(SyncServer *pServer, SocketRead *pRead);

void sync_client_rx_event(SyncServer *pServer, SocketRawEvent *pEvent);

#endif

