/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_SERVER_CONFIG_H__
#define __SYNC_SERVER_CONFIG_H__

void sync_server_config_init(void);

void sync_server_config_exit(void);

dave_bool sync_server_config_set(s8 *key, s8 *value);

void sync_server_config_tell_client(SyncClient *pClient);

#endif

