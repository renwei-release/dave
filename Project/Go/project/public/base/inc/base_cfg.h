/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __BASE_CFG_H__
#define __BASE_CFG_H__

#define CFG_LOG_SERVER_DOMAIN "LogServerDomain"
#define CFG_BASE_CHAIN_ENABLE "BaseChainEnable"
#define CFG_BASE_CHAIN_TYPE "BaseChainType"
#define CFG_IO_DOMAIN "IODomain"

void base_cfg_init(void);
void base_cfg_exit(void);

RetCode base_cfg_local_set(s8 *dir, s8 *name, u8 *value_ptr, ub value_len);
dave_bool base_cfg_local_get(s8 *dir, s8 *name, u8 *value_ptr, ub value_len);
s8 * base_cfg_local_get_by_default(s8 *dir, s8 *name, s8 *value_ptr, ub value_len, s8 *default_value);
RetCode base_cfg_remote_set(s8 *name, s8 *value, sb ttl);
sb base_cfg_remote_get(s8 *name, s8 *value_ptr, ub value_len);
dave_bool base_cfg_remote_del(s8 *name);
sb base_cfg_remote_index(ub index, s8 *key_ptr, ub key_len, s8 *value_ptr, ub value_len);
dave_bool base_cfg_remote_internal_add(s8 *name, s8 *value);
dave_bool base_cfg_remote_internal_del(s8 *name);

RetCode base_cfg_set_str(s8 *cfg_name, s8 *str_value);
s8 * base_cfg_get_str(s8 *cfg_name, s8 *buf_ptr, ub buf_len, s8 *default_value);
RetCode base_cfg_set_ub(s8 *cfg_name, ub ub_value);
ub base_cfg_get_ub(s8 *cfg_name, ub default_value);
RetCode base_cfg_set_bool(s8 *cfg_name, dave_bool bool_value);
dave_bool base_cfg_get_bool(s8 *cfg_name, dave_bool default_value);

#define base_cfg_set(name, value_ptr, value_len) base_cfg_local_set((s8 *)((void *)NULL), name, (u8 *)value_ptr, value_len)
#define base_cfg_get(name, value_ptr, value_len) base_cfg_local_get((s8 *)((void *)NULL), name, (u8 *)value_ptr, value_len)
#define base_cfg_get_by_default(name, value_ptr, value_len, default_value) base_cfg_local_get_by_default((s8 *)((void *)NULL), name, value_ptr, value_len, default_value)
#define base_rcfg_set(name, value, ttl) base_cfg_remote_set(name, value, ttl)
#define base_rcfg_get(name, value_ptr, value_len) base_cfg_remote_get(name, value_ptr, value_len)
#define base_rcfg_del(name) base_cfg_remote_del(name)
#define base_rcfg_index(index, key_ptr, key_len, value_ptr, value_len) base_cfg_remote_index(index, key_ptr, key_len, value_ptr, value_len)

#define cfg_set base_cfg_set
#define cfg_get base_cfg_get
#define cfg_get_by_default base_cfg_get_by_default
#define cfg_set_str base_cfg_set_str
#define cfg_get_str base_cfg_get_str
#define cfg_set_ub base_cfg_set_ub
#define cfg_get_ub base_cfg_get_ub
#define cfg_set_bool base_cfg_set_bool
#define cfg_get_bool base_cfg_get_bool

#define rcfg_set base_rcfg_set
#define rcfg_get base_rcfg_get
#define rcfg_del base_rcfg_del
#define rcfg_index base_rcfg_index

#endif

