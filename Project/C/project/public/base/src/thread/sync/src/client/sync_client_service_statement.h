/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_CLIENT_SERVICE_STATEMENT_H__
#define __SYNC_CLIENT_SERVICE_STATEMENT_H__

void sync_client_service_statement_reset(SyncServer *pServer);

void sync_client_service_statement_rx(SyncServer *pServer, ub frame_len, s8 *frame_ptr);

void sync_client_service_statement_tx(SyncServer *pServer);

ub sync_client_service_statement_info(s8 *info_ptr, ub info_len, SyncServer *pServer);

#endif

