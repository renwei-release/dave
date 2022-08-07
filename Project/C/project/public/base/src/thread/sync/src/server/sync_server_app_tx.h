/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_SERVER_APP_TX_H__
#define __SYNC_SERVER_APP_TX_H__

void sync_server_app_tx_all_client(ub msg_id, ub msg_len, void *msg_body);

void sync_server_app_tx_client(SyncClient *pClient, ub msg_id, ub msg_len, void *msg_body);

#endif

