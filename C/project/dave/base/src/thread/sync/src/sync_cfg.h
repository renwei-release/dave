/*
 * ================================================================================
 * (c) Copyright 2017 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2017.04.26.
 * The synchronization service in the future may try to connect to all within
 * the framework of the server.
 * ================================================================================
 */

#ifndef __SYNC_CFG_H__
#define __SYNC_CFG_H__
#include "base_macro.h"

void sync_cfg_get_syncs_ip(u8 ip[DAVE_IP_V4_ADDR_LEN]);

u16 sync_cfg_get_syncs_port(void);

dave_bool sync_cfg_get_local_ip(u8 ip[DAVE_IP_V4_ADDR_LEN]);

#endif

