/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __BASE_CFG_H__
#define __BASE_CFG_H__

#define CFG_SYNC_ADDRESS (s8 *)"SYNCAddress"
#define CFG_SYNC_PORT (s8 *)"SYNCPort"
#define CFG_LOCALHOST (s8 *)"LOCALHOST"
#define CFG_SYNC_CLIENT_ADDRESS (s8 *)"SYNCClientAddress"
#define CFG_LOG_SERVER_IP_V4 (s8 *)"LOGSerIPV4"
#define CFG_REDIS_ADDRESS (s8 *)"REDISADDRESS"
#define CFG_REDIS_PORT (s8 *)"REDISPORT"
#define CFG_REDIS_PWD (s8 *)"REDISPASSWORD"

ErrCode base_cfg_dir_set(s8 *dir, s8 *name, u8 *value_ptr, ub value_len);
dave_bool base_cfg_dir_get(s8 *dir, s8 *name, u8 *value_ptr, ub value_len);
ErrCode base_cfg_set_ub(s8 *cfg_name, ub ub_value);
ub base_cfg_get_ub(s8 *cfg_name);

#define base_cfg_set(name, value_ptr, value_len) base_cfg_dir_set(NULL, name, value_ptr, value_len)
#define base_cfg_get(name, value_ptr, value_len) base_cfg_dir_get(NULL, name, value_ptr, value_len)

#define cfg_set base_cfg_set
#define cfg_get base_cfg_get

#endif

