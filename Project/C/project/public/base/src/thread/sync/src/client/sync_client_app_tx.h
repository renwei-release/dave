/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_CLIENT_APP_TX_H__
#define __SYNC_CLIENT_APP_TX_H__
#include "dave_base.h"

dave_bool sync_client_app_tx_all_server(ub msg_id, ub msg_len, void *msg_body);

dave_bool sync_client_app_tx_server(SyncServer *pServer, ub msg_id, ub msg_len, void *msg_body);

dave_bool sync_client_tx_system_state(SyncServer *pServer);

#endif

