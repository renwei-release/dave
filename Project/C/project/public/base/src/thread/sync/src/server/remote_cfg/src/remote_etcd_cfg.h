/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __REMOTE_ETCD_CFG_H__
#define __REMOTE_ETCD_CFG_H__

typedef void (* remote_cfg_get_callback)(dave_bool put_flag, s8 *key, s8 *value);

void remote_etcd_cfg_init(remote_cfg_get_callback get_callback);

void remote_etcd_cfg_exit(void);

dave_bool remote_etcd_cfg_set(
	s8 *verno, s8 *globally_identifier,
	s8 *cfg_name, s8 *cfg_value,
	sb ttl);

void remote_etcd_cfg_get(remote_cfg_get_callback get_callback);

#endif

