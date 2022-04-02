/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __KV_LOCAL_API_H__
#define __KV_LOCAL_API_H__

dave_bool kv_local_add(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line);

ub kv_local_inq(KV *pKV, sb index, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line);

ub kv_local_del(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line);

dave_bool kv_local_top(KV *pKV, u8 *key_ptr, ub key_len);

#endif

