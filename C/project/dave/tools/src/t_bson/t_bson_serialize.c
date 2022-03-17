/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "t_bson_define.h"
#include "t_bson_mem.h"
#include "t_bson_add.h"
#include "tools_log.h"

#define SERIALIZE_VERSION 1

static size_t _t_bson_array_to_serialize_(tBsonObject *pBson, unsigned char *serialize_ptr, size_t serialize_len);
static tBsonObject * _t_bson_serialize_to_array_(unsigned char *serialize_ptr, size_t serialize_len);
static size_t _t_bson_object_to_serialize_(tBsonObject *pBson, unsigned char *serialize_ptr, size_t serialize_len);
static tBsonObject * _t_bson_serialize_to_object_(unsigned char *serialize_ptr, size_t serialize_len);

static inline size_t
_t_bson_serialize_to_array_head(tBsonType *type, size_t *value_len, unsigned char *serialize_ptr, size_t serialize_len)
{
	if(serialize_len < 5)
	{
		TOOLSABNOR("find invalid serialize_len:%d", serialize_len);
		*type = tBsonType_null;
		*value_len = 0;
		return serialize_len;
	}

	*type = (tBsonType)serialize_ptr[0];
	*value_len = (size_t)(((unsigned int)(serialize_ptr[1]) << 24) +
		((unsigned int)(serialize_ptr[2]) << 16) +
		((unsigned int)(serialize_ptr[3]) << 8) +
		((unsigned int)(serialize_ptr[4])));
	if(5 + (*value_len) > serialize_len)
	{
		TOOLSABNOR("find invalid serialize_len:%d/%d", serialize_len, *value_len);
		*type = tBsonType_null;
		*value_len = 0;
		return serialize_len;
	}

	return 5;
}

static inline size_t
_t_bson_array_head_to_serialize(tBsonType type, size_t value_len, unsigned char *serialize_ptr, size_t serialize_len)
{
	if(serialize_len < (5 + value_len))
	{
		TOOLSLOG("invalid length:%d/%d", serialize_len, value_len);
		return 0;
	}

	serialize_ptr[0] = type;
	if(value_len <= 0)
	{
		serialize_ptr[1] = 0;
		serialize_ptr[2] = 0;
		serialize_ptr[3] = 0;
		serialize_ptr[4] = 0;
	}
	else
	{
		serialize_ptr[1] = (unsigned char)(((unsigned int)value_len) >> 24);
		serialize_ptr[2] = (unsigned char)(((unsigned int)value_len) >> 16);
		serialize_ptr[3] = (unsigned char)(((unsigned int)value_len) >> 8);
		serialize_ptr[4] = (unsigned char)value_len;
	}

	return 5;
}

static inline void
_t_bson_rebuild_array_head_value_len(size_t value_len, unsigned char *serialize_ptr)
{
	serialize_ptr[1] = (unsigned char)(((unsigned int)value_len) >> 24);
	serialize_ptr[2] = (unsigned char)(((unsigned int)value_len) >> 16);
	serialize_ptr[3] = (unsigned char)(((unsigned int)value_len) >> 8);
	serialize_ptr[4] = (unsigned char)value_len;
}

static inline size_t
_t_bson_serialize_to_head(tBsonType *type, unsigned char **key_ptr, size_t *key_len, size_t *value_len, unsigned char *serialize_ptr, size_t serialize_len)
{
	if(serialize_len < 9)
	{
		TOOLSABNOR("find invalid serialize_len:%d", serialize_len);
		*type = tBsonType_null;
		*key_ptr = NULL;
		*key_len = 0;
		*value_len = 0;
		return serialize_len;
	}

	*type = (tBsonType)serialize_ptr[0];
	if(*type > tBsonType_object)
	{
		TOOLSABNOR("find invalid type:%d", *type);
	}
	*key_len = (size_t)(((unsigned int)(serialize_ptr[1]) << 24) +
		((unsigned int)(serialize_ptr[2]) << 16) +
		((unsigned int)(serialize_ptr[3]) << 8) +
		((unsigned int)(serialize_ptr[4])));
	if(9 + *key_len > serialize_len)
	{
		TOOLSABNOR("find invalid serialize_len:%d/%d %x/%x/%x/%x",
			serialize_len, *key_len,
			serialize_ptr[1], serialize_ptr[2],
			serialize_ptr[3], serialize_ptr[4]);
		*type = tBsonType_null;
		*key_ptr = NULL;
		*key_len = 0;
		*value_len = 0;
		return serialize_len;
	}
	*value_len = (size_t)(((unsigned int)(serialize_ptr[5]) << 24) +
		((unsigned int)(serialize_ptr[6]) << 16) +
		((unsigned int)(serialize_ptr[7]) << 8) +
		((unsigned int)(serialize_ptr[8])));
	if(9 + (*key_len) + (*value_len) > serialize_len)
	{
		TOOLSABNOR("find invalid serialize_len:%d/%d/%d %x/%x/%x/%x",
			serialize_len, *key_len, *value_len,
			serialize_ptr[5], serialize_ptr[6],
			serialize_ptr[7], serialize_ptr[8]);
		*type = tBsonType_null;
		*key_ptr = NULL;
		*key_len = 0;
		*value_len = 0;
		return serialize_len;
	}

	*key_ptr = &serialize_ptr[9];

	return 9 + *key_len;
}

static inline size_t
_t_bson_head_to_serialize(tBsonType type, unsigned char *key_ptr, size_t key_len, size_t value_len, unsigned char *serialize_ptr, size_t serialize_len)
{
	int serialize_index = 0;

	if(serialize_len < (9 + key_len + value_len))
	{
		TOOLSLOG("invalid length:%d/%d/%d", serialize_len, key_len, value_len);
		return 0;
	}

	serialize_ptr[0] = type;
	if((key_ptr == NULL) || (key_len <= 0))
	{
		serialize_ptr[1] = 0;
		serialize_ptr[2] = 0;
		serialize_ptr[3] = 0;
		serialize_ptr[4] = 0;
	}
	else
	{
		serialize_ptr[1] = (unsigned char)(((unsigned int)key_len) >> 24);
		serialize_ptr[2] = (unsigned char)(((unsigned int)key_len) >> 16);
		serialize_ptr[3] = (unsigned char)(((unsigned int)key_len) >> 8);
		serialize_ptr[4] = (unsigned char)key_len;
	}
	if(value_len <= 0)
	{
		serialize_ptr[5] = 0;
		serialize_ptr[6] = 0;
		serialize_ptr[7] = 0;
		serialize_ptr[8] = 0;
	}
	else
	{
		serialize_ptr[5] = (unsigned char)(((unsigned int)value_len) >> 24);
		serialize_ptr[6] = (unsigned char)(((unsigned int)value_len) >> 16);
		serialize_ptr[7] = (unsigned char)(((unsigned int)value_len) >> 8);
		serialize_ptr[8] = (unsigned char)value_len;
	}

	serialize_index += 9;

	if((key_ptr != NULL) && (key_len > 0))
	{
		serialize_index += dave_memcpy(&serialize_ptr[serialize_index], key_ptr, key_len);		
	}

	return serialize_index;
}

static inline void
_t_bson_rebuild_head_value_len(size_t value_len, unsigned char *serialize_ptr)
{
	serialize_ptr[5] = (unsigned char)(((unsigned int)value_len) >> 24);
	serialize_ptr[6] = (unsigned char)(((unsigned int)value_len) >> 16);
	serialize_ptr[7] = (unsigned char)(((unsigned int)value_len) >> 8);
	serialize_ptr[8] = (unsigned char)value_len;
}

static inline size_t
_t_bson_boolean_to_serialize(tBsonData *pData, unsigned char *serialize_ptr, size_t serialize_len)
{
	if(serialize_len < 1)
	{
		TOOLSABNOR("serialize_len:%d is too short!", serialize_len);
		return 0;
	}

	serialize_ptr[0] = pData->value_ptr.bool_value;

	return 1;
}

static inline void
_t_bson_serialize_to_boolean(tBsonObject *pBson, unsigned char *key_ptr, size_t key_len, unsigned char *serialize_ptr, size_t serialize_len)
{
	bool value;

	if(serialize_len < 1)
	{
		TOOLSABNOR("serialize_len:%d is too short!", serialize_len);
		return;
	}

	value = (bool)(serialize_ptr[0]);

	if(pBson->type == tBsonType_array)
		t_bson_array_boolean_add(pBson, value);
	else
		t_bson_boolean_add(pBson, (char *)key_ptr, key_len, value);
}

static inline size_t
_t_bson_int_to_serialize(tBsonData *pData, unsigned char *serialize_ptr, size_t serialize_len)
{
	if(serialize_len < 4)
	{
		TOOLSABNOR("serialize_len:%d is too short!", serialize_len);
		return 0;
	}

	serialize_ptr[0] = (unsigned char)(((unsigned int)(pData->value_ptr.int_value)) >> 24);
	serialize_ptr[1] = (unsigned char)(((unsigned int)(pData->value_ptr.int_value)) >> 16);
	serialize_ptr[2] = (unsigned char)(((unsigned int)(pData->value_ptr.int_value)) >> 8);
	serialize_ptr[3] = (unsigned char)(pData->value_ptr.int_value);

	return 4;
}

static inline void
_t_bson_serialize_to_int(tBsonObject *pBson, unsigned char *key_ptr, size_t key_len, unsigned char *serialize_ptr, size_t serialize_len)
{
	int value;

	if(serialize_len < 4)
	{
		TOOLSABNOR("serialize_len:%d is too short!", serialize_len);
		return;
	}

	value = (int)(((unsigned int)(serialize_ptr[0]) << 24) + 
		((unsigned int)(serialize_ptr[1]) << 16) +
		((unsigned int)(serialize_ptr[2]) << 8) +
		((unsigned int)(serialize_ptr[3])));

	if(pBson->type == tBsonType_array)
		t_bson_array_int_add(pBson, value);
	else
		t_bson_int_add(pBson, (char *)key_ptr, key_len, value);
}

static inline size_t
_t_bson_int64_to_serialize(tBsonData *pData, unsigned char *serialize_ptr, size_t serialize_len)
{
	if(serialize_len < 8)
	{
		TOOLSABNOR("serialize_len:%d is too short!", serialize_len);
		return 0;
	}

	serialize_ptr[0] = (unsigned char)((unsigned long)(pData->value_ptr.int64_value) >> 56);
	serialize_ptr[1] = (unsigned char)((unsigned long)(pData->value_ptr.int64_value) >> 48);
	serialize_ptr[2] = (unsigned char)((unsigned long)(pData->value_ptr.int64_value) >> 40);
	serialize_ptr[3] = (unsigned char)((unsigned long)(pData->value_ptr.int64_value) >> 32);
	serialize_ptr[4] = (unsigned char)((unsigned long)(pData->value_ptr.int64_value) >> 24);
	serialize_ptr[5] = (unsigned char)((unsigned long)(pData->value_ptr.int64_value) >> 16);
	serialize_ptr[6] = (unsigned char)((unsigned long)(pData->value_ptr.int64_value) >> 8);
	serialize_ptr[7] = (unsigned char)((unsigned long)(pData->value_ptr.int64_value));

	return 8;
}

static inline void
_t_bson_serialize_to_int64(tBsonObject *pBson, unsigned char *key_ptr, size_t key_len, unsigned char *serialize_ptr, size_t serialize_len)
{
	u64 value;

	if(serialize_len < 8)
	{
		TOOLSABNOR("serialize_len:%d is too short!", serialize_len);
		return;
	}

	value = (u64)(((unsigned long)(serialize_ptr[0]) << 56) +
		((unsigned long)(serialize_ptr[1]) << 48) +
		((unsigned long)(serialize_ptr[2]) << 40) +
		((unsigned long)(serialize_ptr[3]) << 32) +
		((unsigned long)(serialize_ptr[4]) << 24) + 
		((unsigned long)(serialize_ptr[5]) << 16) +
		((unsigned long)(serialize_ptr[6]) << 8) +
		((unsigned long)(serialize_ptr[7])));

	if(pBson->type == tBsonType_array)
		t_bson_array_int64_add(pBson, value);
	else
		t_bson_int64_add(pBson, (char *)key_ptr, key_len, value);
}

static inline size_t
_t_bson_double_to_serialize(tBsonData *pData, unsigned char *serialize_ptr, size_t serialize_len)
{
	if(serialize_len < 8)
	{
		TOOLSABNOR("serialize_len:%d is too short!", serialize_len);
		return 0;
	}

	serialize_ptr[0] = (unsigned char)((unsigned long)(pData->value_ptr.double_value) >> 56);
	serialize_ptr[1] = (unsigned char)((unsigned long)(pData->value_ptr.double_value) >> 48);
	serialize_ptr[2] = (unsigned char)((unsigned long)(pData->value_ptr.double_value) >> 40);
	serialize_ptr[3] = (unsigned char)((unsigned long)(pData->value_ptr.double_value) >> 32);
	serialize_ptr[4] = (unsigned char)((unsigned long)(pData->value_ptr.double_value) >> 24);
	serialize_ptr[5] = (unsigned char)((unsigned long)(pData->value_ptr.double_value) >> 16);
	serialize_ptr[6] = (unsigned char)((unsigned long)(pData->value_ptr.double_value) >> 8);
	serialize_ptr[7] = (unsigned char)((unsigned long)(pData->value_ptr.double_value));

	return 8;
}

static inline void
_t_bson_serialize_to_double(tBsonObject *pBson, unsigned char *key_ptr, size_t key_len, unsigned char *serialize_ptr, size_t serialize_len)
{
	double value;

	if(serialize_len < 8)
	{
		TOOLSABNOR("serialize_len:%d is too short!", serialize_len);
		return;
	}

	value = (double)(((unsigned long)(serialize_ptr[0]) << 56) +
		((unsigned long)(serialize_ptr[1]) << 48) +
		((unsigned long)(serialize_ptr[2]) << 40) +
		((unsigned long)(serialize_ptr[3]) << 32) +
		((unsigned long)(serialize_ptr[4]) << 24) + 
		((unsigned long)(serialize_ptr[5]) << 16) +
		((unsigned long)(serialize_ptr[6]) << 8) +
		((unsigned long)(serialize_ptr[7])));

	if(pBson->type == tBsonType_array)
		t_bson_array_double_add(pBson, value);
	else
		t_bson_double_add(pBson, (char *)key_ptr, key_len, value);
}

static inline size_t
_t_bson_string_to_serialize(tBsonData *pData, unsigned char *serialize_ptr, size_t serialize_len)
{
	if(serialize_len < pData->value_len)
	{
		TOOLSABNOR("serialize_len:%d/%d is too short!", serialize_len, pData->value_len);
		return 0;
	}

	dave_memcpy(serialize_ptr, pData->value_ptr.mem_value, pData->value_len);

	return pData->value_len;
}

static inline void
_t_bson_serialize_to_string(tBsonObject *pBson, unsigned char *key_ptr, size_t key_len, unsigned char *value_ptr, size_t value_len)
{
	if(value_len <= 0)
	{
		TOOLSABNOR("serialize_len:%d is too short!", value_len);
		return;
	}

	if(pBson->type == tBsonType_array)
		t_bson_array_string_add(pBson, (char *)value_ptr, value_len);
	else
		t_bson_string_add(pBson, (char *)key_ptr, key_len, (char *)value_ptr, value_len);
}

static inline size_t
_t_bson_bin_to_serialize(tBsonData *pData, unsigned char *serialize_ptr, size_t serialize_len)
{
	if(serialize_len < pData->value_len)
	{
		TOOLSABNOR("serialize_len:%d/%d is too short!", serialize_len, pData->value_len);
		return 0;
	}

	dave_memcpy(serialize_ptr, pData->value_ptr.mem_value, pData->value_len);

	return pData->value_len;
}

static inline void
_t_bson_serialize_to_bin(tBsonObject *pBson, unsigned char *key_ptr, size_t key_len, unsigned char *value_ptr, size_t value_len)
{
	if(value_len <= 0)
	{
		return;
	}

	if(pBson->type == tBsonType_array)
		t_bson_array_bin_add(pBson, (char *)value_ptr, value_len);
	else
		t_bson_bin_add(pBson, (char *)key_ptr, key_len, (char *)value_ptr, value_len);
}

static inline size_t
_t_bson_array_to_serialize(tBsonData *pData, unsigned char *serialize_ptr, size_t serialize_len)
{
	if(serialize_len < pData->serialize_estimated_len)
	{
		TOOLSABNOR("serialize_len:%d/%d is too short!", serialize_len, pData->serialize_estimated_len);
		return 0;
	}

	return _t_bson_array_to_serialize_((tBsonObject *)(pData->value_ptr.object_value), serialize_ptr, serialize_len);
}

static inline void
_t_bson_serialize_to_array(tBsonObject *pBson, unsigned char *key_ptr, size_t key_len, unsigned char *value_ptr, size_t value_len)
{
	tBsonObject *pAddArray;

	if(value_len <= 0)
	{
		return;
	}

	pAddArray = _t_bson_serialize_to_array_(value_ptr, value_len);

	if(pBson->type == tBsonType_array)
		t_bson_array_object_add(pBson, pAddArray);
	else
		t_bson_object_add(pBson, (char *)key_ptr, key_len, pAddArray);
}

static inline size_t
_t_bson_object_to_serialize(tBsonData *pData, unsigned char *serialize_ptr, size_t serialize_len)
{
	if(serialize_len < pData->serialize_estimated_len)
	{
		TOOLSABNOR("serialize_len:%d/%d is too short!", serialize_len, pData->serialize_estimated_len);
		return 0;
	}

	return _t_bson_object_to_serialize_((tBsonObject *)(pData->value_ptr.object_value), serialize_ptr, serialize_len);
}

static inline void
_t_bson_serialize_to_object(tBsonObject *pBson, unsigned char *key_ptr, size_t key_len, unsigned char *value_ptr, size_t value_len)
{
	tBsonObject *pAddBson;

	if(value_len <= 0)
	{
		TOOLSABNOR("serialize_len:%d is too short!", value_len);
		return;
	}

	pAddBson = _t_bson_serialize_to_object_(value_ptr, value_len);

	if(pBson->type == tBsonType_array)
		t_bson_array_object_add(pBson, pAddBson);
	else
		t_bson_object_add(pBson, (char *)key_ptr, key_len, pAddBson);
}

static size_t
_t_bson_array_to_serialize_(tBsonObject *pArray, unsigned char *serialize_ptr, size_t serialize_len)
{
	size_t serialize_index, value_len;
	unsigned char *head_ptr;
	tBsonData *pData;

	if(pArray == NULL)
	{
		TOOLSLOG("pBson is NULL!");
		return 0;
	}
	if(pArray->type != tBsonType_array)
	{
		TOOLSLOG("invalid type:%d", pArray->type);
		return 0;
	}

	serialize_index = 0;

	pData = pArray->pDataHead;
	while((pData != NULL) && (serialize_index < serialize_len))
	{
		head_ptr = &serialize_ptr[serialize_index];

		serialize_index += _t_bson_array_head_to_serialize(
			pData->type,
			pData->value_len,
			&serialize_ptr[serialize_index], serialize_len-serialize_index);

		switch(pData->type)
		{
			case tBsonType_boolean:
					value_len = _t_bson_boolean_to_serialize(
						pData,
						&serialize_ptr[serialize_index], serialize_len - serialize_index);
				break;
			case tBsonType_int:
					value_len = _t_bson_int_to_serialize(
						pData,
						&serialize_ptr[serialize_index], serialize_len - serialize_index);
				break;
			case tBsonType_int64:
					value_len = _t_bson_int64_to_serialize(
						pData,
						&serialize_ptr[serialize_index], serialize_len - serialize_index);
				break;
			case tBsonType_double:
					value_len = _t_bson_double_to_serialize(
						pData,
						&serialize_ptr[serialize_index], serialize_len - serialize_index);
				break;
			case tBsonType_string:
					value_len = _t_bson_string_to_serialize(
						pData,
						&serialize_ptr[serialize_index], serialize_len - serialize_index);
				break;
			case tBsonType_bin:
					value_len = _t_bson_bin_to_serialize(
						pData,
						&serialize_ptr[serialize_index], serialize_len - serialize_index);
				break;
			case tBsonType_array:
					value_len = _t_bson_array_to_serialize(
						pData,
						&serialize_ptr[serialize_index], serialize_len - serialize_index);
				break;
			case tBsonType_object:
					value_len = _t_bson_object_to_serialize(
						pData,
						&serialize_ptr[serialize_index], serialize_len - serialize_index);
				break;
			default:
					TOOLSLOG("invalid data type:%d", pData->type);
					value_len = 0;
				break;
		}

		serialize_index += value_len;

		_t_bson_rebuild_array_head_value_len(value_len, head_ptr);

		pData = (tBsonData *)(pData->next);
	}

	return serialize_index;
}

static tBsonObject *
_t_bson_serialize_to_array_(unsigned char *serialize_ptr, size_t serialize_len)
{
	tBsonObject *pArray;
	tBsonType type;
	size_t value_len;
	size_t serialize_index;

	pArray = t_bson_array_malloc();

	serialize_index = 0;

	while(serialize_index < serialize_len)
	{
		serialize_index += _t_bson_serialize_to_array_head(
			&type,
			&value_len,
			&serialize_ptr[serialize_index], serialize_len - serialize_index);
	
		switch(type)
		{
			case tBsonType_boolean:
					_t_bson_serialize_to_boolean(pArray, NULL, -1, &serialize_ptr[serialize_index], value_len);
				break;
			case tBsonType_int:
					_t_bson_serialize_to_int(pArray, NULL, -1, &serialize_ptr[serialize_index], value_len);
				break;
			case tBsonType_int64:
					_t_bson_serialize_to_int64(pArray, NULL, -1, &serialize_ptr[serialize_index], value_len);
				break;
			case tBsonType_double:
					_t_bson_serialize_to_double(pArray, NULL, -1, &serialize_ptr[serialize_index], value_len);
				break;
			case tBsonType_string:
					_t_bson_serialize_to_string(pArray, NULL, -1, &serialize_ptr[serialize_index], value_len);
				break;
			case tBsonType_bin:
					_t_bson_serialize_to_bin(pArray, NULL, -1, &serialize_ptr[serialize_index], value_len);
				break;
			case tBsonType_array:
					_t_bson_serialize_to_array(pArray, NULL, -1, &serialize_ptr[serialize_index], value_len);
				break;
			case tBsonType_object:
					_t_bson_serialize_to_object(pArray, NULL, -1, &serialize_ptr[serialize_index], value_len);
				break;
			default:
				break;
		}

		serialize_index += value_len;
	}

	if(serialize_index != serialize_len)
	{
		TOOLSLOG("serialize len(%d/%d) process failed!", serialize_index, serialize_len);
	}

	return pArray;
}

static size_t
_t_bson_object_to_serialize_(tBsonObject *pBson, unsigned char *serialize_ptr, size_t serialize_len)
{
	size_t serialize_index, value_len;
	unsigned char *head_ptr;
	tBsonData *pData;

	if(pBson == NULL)
	{
		TOOLSLOG("pBson is NULL!");
		return 0;
	}
	if(pBson->type != tBsonType_object)
	{
		TOOLSLOG("invalid type:%d", pBson->type);
		return 0;
	}

	serialize_index = 0;

	pData = pBson->pDataHead;
	while((pData != NULL) && (serialize_index < serialize_len))
	{
		head_ptr = &serialize_ptr[serialize_index];

		serialize_index += _t_bson_head_to_serialize(
			pData->type,
			(unsigned char *)pData->key_ptr, pData->key_len,
			pData->value_len,
			&serialize_ptr[serialize_index], serialize_len-serialize_index);

		switch(pData->type)
		{
			case tBsonType_boolean:
					value_len = _t_bson_boolean_to_serialize(
						pData,
						&serialize_ptr[serialize_index], serialize_len - serialize_index);
				break;
			case tBsonType_int:
					value_len = _t_bson_int_to_serialize(
						pData,
						&serialize_ptr[serialize_index], serialize_len - serialize_index);
				break;
			case tBsonType_int64:
					value_len = _t_bson_int64_to_serialize(
						pData,
						&serialize_ptr[serialize_index], serialize_len - serialize_index);
				break;
			case tBsonType_double:
					value_len = _t_bson_double_to_serialize(
						pData,
						&serialize_ptr[serialize_index], serialize_len - serialize_index);
				break;
			case tBsonType_string:
					value_len = _t_bson_string_to_serialize(
						pData,
						&serialize_ptr[serialize_index], serialize_len - serialize_index);
				break;
			case tBsonType_bin:
					value_len = _t_bson_bin_to_serialize(
						pData,
						&serialize_ptr[serialize_index], serialize_len - serialize_index);
				break;
			case tBsonType_array:
					value_len = _t_bson_array_to_serialize(
						pData,
						&serialize_ptr[serialize_index], serialize_len - serialize_index);
				break;
			case tBsonType_object:
					value_len = _t_bson_object_to_serialize(
						pData,
						&serialize_ptr[serialize_index], serialize_len - serialize_index);
				break;
			default:
					TOOLSLOG("invalid data type:%d", pData->type);
					value_len = 0;
				break;
		}

		serialize_index += value_len;

		_t_bson_rebuild_head_value_len(value_len, head_ptr);

		pData = (tBsonData *)(pData->next);
	}

	return serialize_index;
}

static tBsonObject *
_t_bson_serialize_to_object_(unsigned char *serialize_ptr, size_t serialize_len)
{
	tBsonObject *pBson;
	tBsonType type;
	unsigned char *key_ptr;
	size_t key_len, value_len;
	size_t serialize_index;

	pBson = t_bson_object_malloc();

	serialize_index = 0;

	while(serialize_index < serialize_len)
	{
		serialize_index += _t_bson_serialize_to_head(
			&type,
			&key_ptr, &key_len, &value_len,
			&serialize_ptr[serialize_index], serialize_len - serialize_index);
	
		switch(type)
		{
			case tBsonType_boolean:
					_t_bson_serialize_to_boolean(pBson, key_ptr, key_len, &serialize_ptr[serialize_index], value_len);
				break;
			case tBsonType_int:
					_t_bson_serialize_to_int(pBson, key_ptr, key_len, &serialize_ptr[serialize_index], value_len);
				break;
			case tBsonType_int64:
					_t_bson_serialize_to_int64(pBson, key_ptr, key_len, &serialize_ptr[serialize_index], value_len);
				break;
			case tBsonType_double:
					_t_bson_serialize_to_double(pBson, key_ptr, key_len, &serialize_ptr[serialize_index], value_len);
				break;
			case tBsonType_string:
					_t_bson_serialize_to_string(pBson, key_ptr, key_len, &serialize_ptr[serialize_index], value_len);
				break;
			case tBsonType_bin:
					_t_bson_serialize_to_bin(pBson, key_ptr, key_len, &serialize_ptr[serialize_index], value_len);
				break;
			case tBsonType_array:
					_t_bson_serialize_to_array(pBson, key_ptr, key_len, &serialize_ptr[serialize_index], value_len);
				break;
			case tBsonType_object:
					_t_bson_serialize_to_object(pBson, key_ptr, key_len, &serialize_ptr[serialize_index], value_len);
				break;
			default:
				break;
		}

		serialize_index += value_len;
	}

	if(serialize_index != serialize_len)
	{
		TOOLSLOG("serialize len(%d/%d) process failed!", serialize_index, serialize_len);
	}

	return pBson;
}

// =====================================================================

void
t_bson_serialize_reset(tBsonObject *pBson)
{
	pBson->serialize.estimated_len = pBson->serialize.actual_len = 0;
	pBson->serialize.ptr = NULL;
}

void
t_bson_serialize_clean(tBsonObject *pBson)
{
	t_bson_serialize_free(pBson);
}

size_t
t_bson_serialize(tBsonObject *pBson, unsigned char *serialize_ptr, size_t serialize_len)
{
	unsigned char *head_ptr;
	size_t serialize_index, value_len;

	serialize_index = 0;

	serialize_ptr[serialize_index ++] = SERIALIZE_VERSION;

	head_ptr = &serialize_ptr[serialize_index];

	serialize_index += _t_bson_head_to_serialize(
		tBsonType_object,
		NULL, 0, 0,
		&serialize_ptr[serialize_index], serialize_len-serialize_index);

	value_len = _t_bson_object_to_serialize_(
		pBson,
		&serialize_ptr[serialize_index], serialize_len-serialize_index);
	serialize_index += value_len;

	_t_bson_rebuild_head_value_len(value_len, head_ptr);

	return serialize_index;
}

tBsonObject *
t_serialize_bson(unsigned char *serialize_ptr, size_t serialize_len)
{
	tBsonType type;
	unsigned char *key_ptr;
	size_t key_len, value_len;
	size_t serialize_index;

	if((serialize_ptr == NULL) || (serialize_len <= 0))
		return NULL;

	serialize_index = 0;

	if(serialize_ptr[serialize_index ++] != SERIALIZE_VERSION)
	{
		TOOLSLOG("invalid version:%d", serialize_ptr[0]);
		return NULL;
	}

	serialize_index += _t_bson_serialize_to_head(
		&type,
		&key_ptr, &key_len, &value_len,
		&serialize_ptr[serialize_index], serialize_len - serialize_index);

	if(type != tBsonType_object)
	{
		TOOLSLOG("invalid type:%d", type);
		return NULL;
	}
	if((serialize_index + value_len) != serialize_len)
	{
		TOOLSLOG("invalid len:%d/%d/%d/%d", serialize_index, key_len, value_len, serialize_len);
		return NULL;
	}

	return _t_bson_serialize_to_object_(&serialize_ptr[serialize_index], serialize_len - serialize_index);
}

