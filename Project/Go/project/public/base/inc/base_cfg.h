/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __BASE_CFG_H__
#define __BASE_CFG_H__

RetCode base_cfg_dir_set(s8 *dir, s8 *name, u8 *value_ptr, ub value_len);
dave_bool base_cfg_dir_get(s8 *dir, s8 *name, u8 *value_ptr, ub value_len);
RetCode base_cfg_set_ub(s8 *cfg_name, ub ub_value);
ub base_cfg_get_ub(s8 *cfg_name);
RetCode base_cfg_set_bool(s8 *cfg_name, dave_bool bool_value);
dave_bool base_cfg_get_bool(s8 *cfg_name, dave_bool default_value);

#define base_cfg_set(name, value_ptr, value_len) base_cfg_dir_set((s8 *)((void *)NULL), name, (u8 *)value_ptr, value_len)
#define base_cfg_get(name, value_ptr, value_len) base_cfg_dir_get((s8 *)((void *)NULL), name, (u8 *)value_ptr, value_len)

#define cfg_set base_cfg_set
#define cfg_get base_cfg_get
#define cfg_set_ub base_cfg_set_ub
#define cfg_get_ub base_cfg_get_ub
#define cfg_set_bool base_cfg_set_bool
#define cfg_get_bool base_cfg_get_bool

#endif

