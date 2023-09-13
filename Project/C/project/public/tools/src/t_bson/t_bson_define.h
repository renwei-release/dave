/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __T_BSON_DEFINE_H__
#define __T_BSON_DEFINE_H__

#include <stddef.h>
#include <stdbool.h>

typedef enum {
	tBsonType_null = 0,
	tBsonType_boolean = 1,
	tBsonType_int = 2,
	tBsonType_int64 = 3,
	tBsonType_double = 4,
	tBsonType_string = 5,
	tBsonType_bin = 6,
	tBsonType_array = 7,
	tBsonType_object = 8,
	tBsonType_bin_insertion = 9,
} tBsonType;

typedef struct {
	size_t estimated_len;
	size_t actual_len;
	char *ptr;
} tBsonSerialize;

typedef union {
	unsigned long size_value;
	bool bool_value;
	int int_value;
	u64 int64_value;
	double double_value;
	char *mem_value;
	void *object_value;
} tBsonDataUnion;

typedef struct {
	tBsonType type;
	size_t key_len;
	char *key_ptr;
	size_t value_len;
	tBsonDataUnion value_ptr;

	size_t list_index;
	void *up;
	void *next;

	size_t serialize_estimated_len;
} tBsonData;

typedef struct {
	tBsonType type;

	tBsonData *pDataHead;
	tBsonData *pDataTail;

	tBsonData *pCurKeyFind;
	tBsonData *pCurIndexFind;

	u64 key_bloom_filter;

	tBsonSerialize serialize;
} tBsonObject;

#endif

