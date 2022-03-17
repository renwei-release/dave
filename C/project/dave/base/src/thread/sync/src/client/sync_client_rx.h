/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_CLIENT_RX_H__
#define __SYNC_CLIENT_RX_H__

void sync_client_rx_read(SyncServer *pServer, SocketRead *pRead);

void sync_client_rx_event(SyncServer *pServer, SocketRawEvent *pEvent);

#endif

