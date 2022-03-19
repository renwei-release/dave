/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_CLIENT_SHADOW_INDEX_H__
#define __SYNC_CLIENT_SHADOW_INDEX_H__

void sync_client_shadow_index_init(void);

void sync_client_shadow_index_exit(void);

dave_bool sync_client_shadow_index_add(SyncServer *pServer, LinkThread *pThread);

dave_bool sync_client_shadow_index_del(SyncServer *pServer, LinkThread *pThread);

#endif

