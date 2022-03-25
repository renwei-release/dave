/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <string.h>
#include "dave_base.h"
#include "dave_os.h"
#include "t_bson_define.h"
#include "t_bson_mem.h"
#include "t_bson_serialize.h"
#include "tools_log.h"

#define SERIALIZE_BASE_LEN 32

#if defined(__DAVE_BASE__)
// #define DAVEMEM_ENABLE
#endif
#if defined(PERFTOOLS_3RDPARTY)
 #define TCMALLOC_ENABLE
#endif
#if defined(JEMALLOC_3RDPARTY)
 #define JEMALLOC_ENABLE
#endif

#if defined(DAVEMEM_ENABLE)
#define BSON_MALLOC(len) dave_malloc(len)
#define BSON_FREE(ptr) dave_free(ptr)
#elif defined(TCMALLOC_ENABLE)
#define BSON_MALLOC(len) dave_perftools_malloc(len)
#define BSON_FREE(ptr) dave_perftools_free(ptr)
#elif defined(JEMALLOC_ENABLE)
#define BSON_MALLOC(len) dave_jemalloc(len)
#define BSON_FREE(ptr) dave_jefree(ptr)
#else
#define BSON_MALLOC(len) dave_os_malloc(len)
#define BSON_FREE(ptr) dave_os_free(ptr)
#endif

static inline void *
_t_bson_mem_malloc(size_t len)
{
	return BSON_MALLOC(len);
}

static inline void
_t_bson_mem_free(void *ptr)
{
	if(ptr != NULL)
	{
		BSON_FREE(ptr);
	}
}

static inline char *
_t_bson_string_to_mem(char *string_ptr, size_t *string_len_ptr)
{
	size_t string_len;
	char *mem_ptr;

	if(string_ptr == NULL)
		return NULL;

	if(*string_len_ptr <= 0)
		*string_len_ptr = string_len = strlen(string_ptr);
	else
		string_len = *string_len_ptr;
	if(string_len == 0)
		return NULL;

	mem_ptr = _t_bson_mem_malloc(string_len + 1);
	memcpy(mem_ptr, string_ptr, string_len);
	mem_ptr[string_len] = '\0';

	return mem_ptr;
}

static inline char *
_t_bson_bin_to_mem(char *bin_ptr, size_t bin_len)
{
	char *mem_ptr;

	mem_ptr = _t_bson_mem_malloc(bin_len + 1);
	memcpy(mem_ptr, bin_ptr, bin_len);
	mem_ptr[bin_len] = '\0';

	return mem_ptr;
}

static inline tBsonData *
_t_bson_data_malloc(char *key_ptr, size_t key_len)
{
	char *key_mem_ptr;
	tBsonData *pData;

	if(key_ptr != NULL)
	{
		key_mem_ptr = _t_bson_string_to_mem(key_ptr, &(key_len));
		if(key_mem_ptr == NULL)
			return NULL;
	}
	else
	{
		key_len = 0;
		key_mem_ptr = NULL;
	}

	pData = (tBsonData *)_t_bson_mem_malloc(sizeof(tBsonData));
	pData->type = tBsonType_null;

	pData->key_len = key_len;
	pData->key_ptr = key_mem_ptr;

	pData->list_index = -1;
	pData->up = pData->next = NULL;

	pData->serialize_estimated_len = SERIALIZE_BASE_LEN + key_len;

	return pData;
}

static inline void
_t_bson_data_free(tBsonData *pData)
{
	if(pData->key_ptr != NULL)
		_t_bson_mem_free(pData->key_ptr);

	if((pData->type == tBsonType_string) || (pData->type == tBsonType_bin))
		_t_bson_mem_free(pData->value_ptr.mem_value);
	else if(pData->type == tBsonType_array)
		t_bson_array_free(pData->value_ptr.object_value);
	else if(pData->type == tBsonType_object)
		t_bson_object_free(pData->value_ptr.object_value);

	_t_bson_mem_free(pData);
}

static inline tBsonObject *
_t_bson_object_malloc(void)
{
	tBsonObject *pBson = (tBsonObject *)_t_bson_mem_malloc(sizeof(tBsonObject));

	pBson->type = tBsonType_null;

	pBson->pDataHead = pBson->pDataTail = NULL;

	pBson->pCurFind = NULL;
	pBson->key_bloom_filter = 0;

	t_bson_serialize_reset(pBson);

	return pBson;
}

static inline void
_t_bson_object_free(tBsonObject *pBson)
{
	tBsonData *pData, *pNext;

	pData = pBson->pDataHead;
	while(pData != NULL)
	{
		pNext = (tBsonData *)(pData->next);	

		_t_bson_data_free(pData);

		pData = pNext;
	}

	t_bson_serialize_clean(pBson);

	_t_bson_mem_free(pBson);
}

// =====================================================================

tBsonObject *
t_bson_object_malloc(void)
{
	tBsonObject *pBson = _t_bson_object_malloc();

	pBson->type = tBsonType_object;

	return pBson;
}

void
t_bson_object_free(tBsonObject *pBson)
{
	if(pBson == NULL)
		return;

	if(pBson->type != tBsonType_object)
	{
		TOOLSABNOR("invalid bson type:%d", pBson->type);
		return;
	}

	TOOLSDEBUG("serialize.estimated_len:%d", pBson->serialize.estimated_len);

	_t_bson_object_free(pBson);
}

tBsonObject *
t_bson_array_malloc(void)
{
	tBsonObject *pBson = _t_bson_object_malloc();

	pBson->type = tBsonType_array;

	return pBson;
}

void
t_bson_array_free(tBsonObject *pBson)
{
	if(pBson == NULL)
		return;

	if(pBson->type != tBsonType_array)
	{
		TOOLSABNOR("invalid bson type:%d", pBson->type);
		return;
	}

	_t_bson_object_free(pBson);
}

tBsonData *
t_bson_data_malloc(char *key_ptr, size_t key_len)
{
	return _t_bson_data_malloc(key_ptr, key_len);
}

void
t_bson_data_free(tBsonData *pData)
{
	_t_bson_data_free(pData);
}

tBsonSerialize
t_bson_serialize_malloc(tBsonObject *pBson)
{
	t_bson_serialize_free(pBson);

	if(pBson->serialize.estimated_len <= 0)
	{
		TOOLSABNOR("invalid estimated_len:%d", pBson->serialize.estimated_len);
	}
	else
	{
		pBson->serialize.ptr = _t_bson_mem_malloc(pBson->serialize.estimated_len);
		pBson->serialize.actual_len = 0;
	}

	return pBson->serialize;
}

void
t_bson_serialize_free(tBsonObject *pBson)
{
	if(pBson->serialize.ptr != NULL)
		_t_bson_mem_free(pBson->serialize.ptr);

	pBson->serialize.actual_len = 0;
	pBson->serialize.ptr = NULL;
}

tBsonData *
t_bson_boolean_build(char *key_ptr, size_t key_len, bool value)
{
	tBsonData *pData;

	pData = _t_bson_data_malloc(key_ptr, key_len);
	if(pData == NULL)
		return NULL;

	pData->type = tBsonType_boolean;
	pData->value_len = sizeof(bool);
	pData->value_ptr.bool_value = value;

	return pData;
}

tBsonData *
t_bson_int_build(char *key_ptr, size_t key_len, int value)
{
	tBsonData *pData;

	pData = _t_bson_data_malloc(key_ptr, key_len);
	if(pData == NULL)
		return NULL;

	pData->type = tBsonType_int;
	pData->value_len = sizeof(int);
	pData->value_ptr.int_value = value;

	return pData;
}

tBsonData *
t_bson_int64_build(char *key_ptr, size_t key_len, u64 value)
{
	tBsonData *pData;

	pData = _t_bson_data_malloc(key_ptr, key_len);
	if(pData == NULL)
		return NULL;

	pData->type = tBsonType_int64;
	pData->value_len = sizeof(u64);
	pData->value_ptr.int64_value = value;

	return pData;
}

tBsonData *
t_bson_double_build(char *key_ptr, size_t key_len, double value)
{
	tBsonData *pData;

	pData = _t_bson_data_malloc(key_ptr, key_len);
	if(pData == NULL)
		return NULL;

	pData->type = tBsonType_double;
	pData->value_len = sizeof(double);
	pData->value_ptr.double_value = value;

	return pData;
}

tBsonData *
t_bson_string_build(char *key_ptr, size_t key_len, char *value_ptr, size_t value_len)
{
	tBsonData *pData;

	pData = _t_bson_data_malloc(key_ptr, key_len);
	if(pData == NULL)
		return NULL;

	pData->type = tBsonType_string;
	pData->value_len = value_len;
	pData->value_ptr.mem_value = _t_bson_string_to_mem(value_ptr, &(pData->value_len));

	pData->serialize_estimated_len += pData->value_len;

	return pData;
}

tBsonData *
t_bson_bin_build(char *key_ptr, size_t key_len, char *valur_ptr, size_t value_len)
{
	tBsonData *pData;

	pData = _t_bson_data_malloc(key_ptr, key_len);
	if(pData == NULL)
		return NULL;

	pData->type = tBsonType_bin;
	pData->value_len = value_len;
	pData->value_ptr.mem_value = _t_bson_bin_to_mem(valur_ptr, pData->value_len);

	pData->serialize_estimated_len += pData->value_len;

	return pData;
}

tBsonData *
t_bson_object_build(char *key_ptr, size_t key_len, tBsonObject *pBson)
{
	tBsonData *pData;

	if((pBson->type != tBsonType_array) && (pBson->type != tBsonType_object))
	{
		TOOLSABNOR("%s build invalid object:%d", key_ptr, pBson->type);
		return NULL;
	}

	pData = _t_bson_data_malloc(key_ptr, key_len);
	if(pData == NULL)
		return NULL;

	pData->type = pBson->type;
	pData->value_len = sizeof(void *);
	pData->value_ptr.object_value = (void *)pBson;

	pData->serialize_estimated_len += pBson->serialize.estimated_len;

	return pData;
}

