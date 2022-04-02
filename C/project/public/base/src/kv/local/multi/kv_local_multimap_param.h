/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __KV_LOCAL_MULTIMAP_PARAM_H__
#define __KV_LOCAL_MULTIMAP_PARAM_H__

#define BASE_MAP_MAX 256

typedef struct {
	ub value_len;
	void *value_ptr;
	ub value_checksum;

	void *base_map[BASE_MAP_MAX];

	void *up_node;			// 为了通过index向上做索引
	void *next_node;		// 为了通过index向下做索引
} KVLocalMultiBaseMap;

typedef struct {
	KVLocalMultiBaseMap *base_map[BASE_MAP_MAX];

	sb kv_number;
	KVLocalMultiBaseMap *head;
	KVLocalMultiBaseMap *tail;

	sb current_kv_number;
	sb current_index;
	KVLocalMultiBaseMap *current_pBaseMap;
} KVLocalMultiMap;

#endif

