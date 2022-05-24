/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_SERVER_TOOLS_H__
#define __SYNC_SERVER_TOOLS_H__

dave_bool sync_server_client_on_thread(SyncThread *pThread, SyncClient *pClient);

dave_bool sync_server_are_they_brothers(SyncClient *pClientA, SyncClient *pClientB);

dave_bool sync_server_still_have_ready_brothers(SyncClient *pClient);

dave_bool sync_server_still_have_blocks_brothers(SyncClient *pClient);

dave_bool sync_server_default_blocks_flag(SyncClient *pClient);

dave_bool sync_server_client_on_work(SyncClient *pClient);

#endif

