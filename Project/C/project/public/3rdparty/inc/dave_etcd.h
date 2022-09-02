/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_ETCD_H__
#define __DAVE_ETCD_H__

typedef void (* etcd_watcher_fun)(dave_bool put_flag, s8 *key, s8 *value);

void dave_etcd_init(s8 *url, s8 *watcher_dir, etcd_watcher_fun watcher_fun);

void dave_etcd_exit(void);

dave_bool dave_etcd_set(s8 *key, s8 *value, sb ttl);

void * dave_etcd_get(s8 *key, ub limit);

#endif
