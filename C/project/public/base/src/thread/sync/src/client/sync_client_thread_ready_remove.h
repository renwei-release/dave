/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_CLIENT_THREAD_READY_REMOVE_H__
#define __SYNC_CLIENT_THREAD_READY_REMOVE_H__

void sync_client_thread_ready_remove_init(void);

void sync_client_thread_ready_remove_exit(void);

void sync_client_thread_ready(SyncServer *pServer, LinkThread *pThread);

void sync_client_thread_remove(SyncServer *pServer, LinkThread *pThread);

#endif

