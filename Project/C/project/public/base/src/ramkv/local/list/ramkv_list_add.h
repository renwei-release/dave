/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RAMKV_LIST_ADD_H__
#define __RAMKV_LIST_ADD_H__

dave_bool __ramkv_list_add__(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line);
#define ramkv_list_add(pKV, key_ptr, key_len, value_ptr, value_len) __ramkv_list_add__(pKV, key_ptr, key_len, value_ptr, value_len, (s8 *)__func__, (ub)__LINE__)

#endif

