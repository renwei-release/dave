/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __REMOTE_CFG_FILE_H__
#define __REMOTE_CFG_FILE_H__

void remote_cfg_file_init(void);
void remote_cfg_file_exit(void);

dave_bool remote_cfg_file_set(s8 *name, s8 *value);
dave_bool remote_cfg_file_del(s8 *name);
sb remote_cfg_file_get(s8 *name, s8 *value_ptr, ub value_len);
sb remote_cfg_file_index(ub index, s8 *key_ptr, ub key_len, s8 *value_ptr, ub value_len);

#endif

