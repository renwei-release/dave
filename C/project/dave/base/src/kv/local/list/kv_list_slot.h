/*
 * ================================================================================
 * (c) Copyright 2020 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.10.22.
 * ================================================================================
 */

#ifndef __KV_LIST_SLOT_H__
#define __KV_LIST_SLOT_H__

KVSlot * kv_slot_malloc(KVSlot **up_ppslot);

void kv_slot_free(KVSlot *pSlot);

dave_bool kv_slot_data_add(KVSlot *pSlot, KVData *pData);

KVData * kv_slot_data_inq(KVSlot *pSlot, u8 *key_ptr, ub key_len);

KVData * kv_slot_data_del(KVSlot **ppSlot, u8 *key_ptr, ub key_len);

#endif
