/*
 * ================================================================================
 * (c) Copyright 2021 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2021.09.29.
 * ================================================================================
 */

#ifndef __SYNC_CLIENT_SHADOW_INDEX_H__
#define __SYNC_CLIENT_SHADOW_INDEX_H__

void sync_client_shadow_index_init(void);

void sync_client_shadow_index_exit(void);

dave_bool sync_client_shadow_index_add(SyncServer *pServer, LinkThread *pThread);

dave_bool sync_client_shadow_index_del(SyncServer *pServer, LinkThread *pThread);

#endif

