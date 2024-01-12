/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_SERVER_SYNC_H__
#define __SYNC_SERVER_SYNC_H__

void sync_server_sync_thread_next(SyncClient *pClient);

void sync_server_sync_thread_booting(SyncClient *pClient);

void sync_server_sync_link(SyncClient *pClient, dave_bool up_flag);

#endif

