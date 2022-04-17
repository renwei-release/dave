/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RAMKV_LIST_PARAM_H__
#define __RAMKV_LIST_PARAM_H__

/*
 * Adjust this to achieve a balance between memory and speed.
 * the max slot number is KV_SLOT_NUM^KV_SLOT_DEPTH
 */
#define KV_SLOT_NUM 16
#define KV_SLOT_DEPTH 6

#define KV_DATA_DEPTH_MAX 2048
#define KV_DATA_DEPTH_WARNING KV_DATA_DEPTH_MAX
#define KV_BLOOM_FILTER_MAX (KV_DATA_DEPTH_MAX / (sizeof(u64) * 8))

#define KV_SLOT_MAGIC_DATA 0xad3964bc123
#define KV_DATA_MAGIC_DATA 0xad3ADD4bc12AD

typedef struct {
	u64 bloom_filter[KV_BLOOM_FILTER_MAX];
} KVBloomFilter;

typedef struct {
	ub hash_slot[KV_SLOT_DEPTH];
} KVHash;

typedef struct {
	ub key_len;
	u8 key_ptr[KV_KEY_MAX];
} KVKey;

typedef struct {
	ub value_len;
	void *value_ptr;
	ub value_checksum;
} KVValue;

typedef struct {
	ub magic_data;

	KVKey key;
	KVValue value;

	void *for_slot_up;			// KVData
	void *for_slot_next;		// 记录KVData在本槽位的上与下的KVData信息

	void *for_list_up;			// KVData
	void *for_list_next;		// 记录KVData在KVList的total_head与total_tail中的上与下的KVData信息
} KVData;

typedef struct {
	ub magic_data;

	void **up_ppslot;
	void *slot[KV_SLOT_NUM];	// KVSlot

	KVBloomFilter bloom_filter;
	sb slot_data_number;
	KVData *slot_data_head;		// KVData
	KVData *slot_data_tail;		// 记录KVData在本槽位的首与尾的KVData信息
} KVSlot;

typedef struct {
	KVSlot *slot[KV_SLOT_NUM];

	KVData *list_data_head;
	KVData *list_data_tail;
} KVList;

#endif

