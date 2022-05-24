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

void __sync_server_sync_link__(SyncClient *pClient, dave_bool up_flag, s8 *fun, ub line);
#define sync_server_sync_link(pClient, up_flag) __sync_server_sync_link__(pClient, up_flag, (s8 *)__func__, (ub)__LINE__)

void sync_server_sync_auto_link(SyncClient *pClient);

#endif

