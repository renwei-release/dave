/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_CFG_H__
#define __SYNC_CFG_H__
#include "base_macro.h"

#define CFG_SYNC_ADDRESS "SYNCAddress"
#define CFG_SYNC_PORT "SYNCPort"

#define CFG_SYNC_CLIENT_ADDRESS "SYNCClientAddress"
#define CFG_SYNC_SERVER_DOMAIN "SYNCServerDomain"
#define CFG_SYNC_SERVER_PORT "SYNCServerPort"

void sync_cfg_external_incoming_sync_domain(s8 *sync_domain);

dave_bool sync_cfg_get_syncs_ip_and_port(u8 ip[DAVE_IP_V4_ADDR_LEN], u16 *port);

u16 sync_cfg_get_syncs_port(void);

dave_bool sync_cfg_get_local_ip(u8 ip[DAVE_IP_V4_ADDR_LEN]);

#endif

