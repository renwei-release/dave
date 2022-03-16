/*
 * ================================================================================
 * (c) Copyright 2020 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.10.22.
 * ================================================================================
 */

#ifndef __KV_LIST_TOOLS_H__
#define __KV_LIST_TOOLS_H__

dave_bool kv_is_my_key(KVData *pData, u8 *key_ptr, ub key_len);

KVSlot * kv_hash_to_slot(KVSlot **ppSlot, KVHash *pHash, dave_bool creat_flag);

KVSlot ** kv_hash_to_pslot(KVSlot **ppSlot, KVHash *pHash, dave_bool creat_flag);

void kv_list_build_value_checksum(KVValue *pValue);

dave_bool kv_list_check_value_checksum(KVValue *pValue);

void __kv_list_show__(KVSlot **slot, s8 *fun, ub line);
#define kv_list_show(slot) __kv_list_show__(slot, (s8 *)__func__, (ub)__LINE__)

#endif

