/*
 * ================================================================================
 * (c) Copyright 2018 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2018.07.23.
 * ================================================================================
 */

#ifndef __SYNC_CLIENT_THREAD_H__
#define __SYNC_CLIENT_THREAD_H__

void sync_client_thread_init(void);

void sync_client_thread_exit(void);

void sync_client_thread_add(SyncServer *pServer, s8 *thread_name);

dave_bool sync_client_thread_del(SyncServer *pServer, s8 *thread_name);

#endif

