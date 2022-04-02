/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __BASE_CFG_H__
#define __BASE_CFG_H__

#define CFG_SYNC_ADDRESS "SYNCAddress"
#define CFG_SYNC_PORT "SYNCPort"
#define CFG_LOCALHOST "LOCALHOST"
#define CFG_SYNC_CLIENT_ADDRESS "SYNCClientAddress"
#define CFG_LOG_SERVER_IP_V4 "LOGSerIPV4"
#define CFG_REDIS_ADDRESS "REDISADDRESS"
#define CFG_REDIS_PORT "REDISPORT"
#define CFG_REDIS_PWD "REDISPASSWORD"
#define CFG_SSL_CERTIFICATE_PATH "ssl_certificate"
#define CFG_SSL_CERTIFICATE_KEY_PATH "ssl_certificate_key"

ErrCode base_cfg_dir_set(s8 *dir, s8 *name, u8 *value_ptr, ub value_len);
dave_bool base_cfg_dir_get(s8 *dir, s8 *name, u8 *value_ptr, ub value_len);
ErrCode base_cfg_set_ub(s8 *cfg_name, ub ub_value);
ub base_cfg_get_ub(s8 *cfg_name);

#define base_cfg_set(name, value_ptr, value_len) base_cfg_dir_set(NULL, name, value_ptr, value_len)
#define base_cfg_get(name, value_ptr, value_len) base_cfg_dir_get(NULL, name, value_ptr, value_len)
#define cfg_set base_cfg_set
#define cfg_get base_cfg_get

#endif

