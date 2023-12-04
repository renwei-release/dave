/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __REMOTE_CFG_H__
#define __REMOTE_CFG_H__

void base_remote_cfg_init(void);
void base_remote_cfg_exit(void);

dave_bool base_remote_cfg_internal_add(s8 *name, s8 *value);
sb base_remote_cfg_internal_inq(s8 *name, s8 *value_ptr, ub value_len);
dave_bool base_remote_cfg_internal_del(s8 *name);

RetCode base_remote_cfg_set(s8 *name, s8 *value, sb ttl);
sb base_remote_cfg_get(s8 *name, s8 *value_ptr, ub value_len);
dave_bool base_remote_cfg_del(s8 *name);
sb base_remote_cfg_index(ub index, s8 *key_ptr, ub key_len, s8 *value_ptr, ub value_len);

#endif

