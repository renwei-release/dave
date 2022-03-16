/*
 * ================================================================================
 * (c) Copyright 2021 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2021.01.19.
 * ================================================================================
 */

#ifndef __KV_LOCAL_MULTIMAP_H__
#define __KV_LOCAL_MULTIMAP_H__

void * kv_local_multimap_malloc(KV *pKV);

void kv_local_multimap_free(KV *pKV);

dave_bool kv_local_multimap_add(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line);

ub kv_local_multimap_inq(KV *pKV, sb index, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len);

ub kv_local_multimap_del(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len);

#endif

