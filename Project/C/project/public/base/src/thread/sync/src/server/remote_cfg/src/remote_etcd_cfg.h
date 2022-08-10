/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __REMOTE_ETCD_CFG_H__
#define __REMOTE_ETCD_CFG_H__

void remote_etcd_cfg_init(void);

void remote_etcd_cfg_exit(void);

dave_bool remote_etcd_cfg_set(s8 *verno, s8 *globally_identifier, s8 *cfg_name, s8 *cfg_value);

#endif

