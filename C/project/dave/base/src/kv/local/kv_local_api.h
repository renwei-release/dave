/*
 * ================================================================================
 * (c) Copyright 2020 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.10.22.
 * ================================================================================
 */

#ifndef __KV_LOCAL_API_H__
#define __KV_LOCAL_API_H__

dave_bool kv_local_add(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line);

ub kv_local_inq(KV *pKV, sb index, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line);

ub kv_local_del(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line);

dave_bool kv_local_top(KV *pKV, u8 *key_ptr, ub key_len);

#endif

