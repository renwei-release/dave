/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RAMKV_LIST_SLOT_H__
#define __RAMKV_LIST_SLOT_H__

KVSlot * ramkv_slot_malloc(KVSlot **up_ppslot);

void ramkv_slot_free(KVSlot *pSlot);

dave_bool ramkv_slot_data_add(KVSlot *pSlot, KVData *pData);

KVData * ramkv_slot_data_inq(KVSlot *pSlot, u8 *key_ptr, ub key_len);

KVData * ramkv_slot_data_del(KVSlot **ppSlot, u8 *key_ptr, ub key_len);

#endif

