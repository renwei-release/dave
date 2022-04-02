/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_CLIENT_THREAD_H__
#define __SYNC_CLIENT_THREAD_H__

void sync_client_thread_init(void);

void sync_client_thread_exit(void);

void sync_client_thread_add(SyncServer *pServer, s8 *thread_name);

dave_bool sync_client_thread_del(SyncServer *pServer, s8 *thread_name);

#endif

