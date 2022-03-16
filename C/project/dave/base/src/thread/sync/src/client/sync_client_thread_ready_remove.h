/*
 * ================================================================================
 * (c) Copyright 2021 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2021.09.30.
 * ================================================================================
 */

#ifndef __SYNC_CLIENT_THREAD_READY_REMOVE_H__
#define __SYNC_CLIENT_THREAD_READY_REMOVE_H__

void sync_client_thread_ready_remove_init(void);

void sync_client_thread_ready_remove_exit(void);

void sync_client_thread_ready(SyncServer *pServer, LinkThread *pThread);

void sync_client_thread_remove(SyncServer *pServer, LinkThread *pThread);

#endif

