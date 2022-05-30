/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __JSON_CFG_H__
#define __JSON_CFG_H__

RetCode base_json_cfg_dir_set(s8 *dir, s8 *name, u8 *value_ptr, ub value_len);
dave_bool base_json_cfg_dir_get(s8 *dir, s8 *name, u8 *value_ptr, ub value_len);

#endif

