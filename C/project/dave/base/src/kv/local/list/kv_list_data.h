/*
 * ================================================================================
 * (c) Copyright 2020 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.10.22.
 * ================================================================================
 */

#ifndef __KV_LIST_DATA_H__
#define __KV_LIST_DATA_H__

KVData * kv_list_data_malloc(u8 *key_ptr, ub key_len, void *value_ptr, ub value_len);

void kv_list_data_free(KVData *pData);

void kv_list_data_add(KVList *pKV, KVData *pData);

void kv_list_data_del(KVList *pKV, KVData *pData);

void kv_list_data_copy_from_user(KVData *pData, void *value_ptr, ub value_len);

ub kv_list_data_copy_to_user(KVData *pData, void *value_ptr, ub value_len);

#endif

