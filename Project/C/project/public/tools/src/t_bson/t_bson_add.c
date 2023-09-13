/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <string.h>
#include "dave_tools.h"
#include "t_bson_define.h"
#include "t_bson_mem.h"
#include "t_bson_bloom.h"
#include "tools_log.h"

static inline bool
_t_bson_add_replace(tBsonObject *pBson, tBsonData *pData)
{
	tBsonData *pCheck;

	pCheck = pBson->pDataHead;

	while(pCheck != NULL)
	{
		if((pCheck->key_len == pData->key_len)
			&& (memcmp(pCheck->key_ptr, pData->key_ptr, pData->key_len) == 0))
		{
			if(pCheck == pBson->pDataHead)
			{
				pData->up = NULL;
				pData->next = pBson->pDataHead->next;
				pBson->pDataHead = pData;
				if(pCheck == pBson->pDataTail)
					pBson->pDataTail = pBson->pDataHead;
			}
			else if(pCheck == pBson->pDataTail)
			{
				((tBsonData *)(pBson->pDataTail->up))->next = pData;

				pData->up = pBson->pDataTail->up;
				pData->next = NULL;
				pBson->pDataTail = pData;
			}
			else
			{
				((tBsonData *)(pCheck->up))->next = pData;
				((tBsonData *)(pCheck->next))->up = pData;

				pData->up = pCheck->up;
				pData->next = pCheck->next;
			}

			t_bson_data_free(pCheck);

			return true;
		}

		pCheck = (tBsonData *)(pCheck->next);
	}

	return false;
}

static inline void
_t_bson_add_new(tBsonObject *pBson, tBsonData *pData)
{
	if(pBson->pDataHead == NULL)
	{
		pData->list_index = 0;

		pBson->pDataHead = pBson->pDataTail = pData;
	}
	else
	{
		pData->list_index = pBson->pDataTail->list_index + 1;

		pBson->pDataTail->next = pData;
		pData->up = pBson->pDataTail;
		pBson->pDataTail = pData;
	}
}

static inline void
_t_bson_add(tBsonObject *pBson, tBsonData *pData)
{
	u64 bloom_filter;

	if(pData == NULL)
		return;

	if(pBson->type == tBsonType_array)
	{
		_t_bson_add_new(pBson, pData);
	}
	else
	{
		bloom_filter = t_bson_bloom(pData->key_ptr, pData->key_len);
		if((bloom_filter & pBson->key_bloom_filter) == 0x00)
		{
			pBson->key_bloom_filter |= bloom_filter;
			_t_bson_add_new(pBson, pData);
		}
		else
		{
			if(_t_bson_add_replace(pBson, pData) == false)
			{
				_t_bson_add_new(pBson, pData);
			}
		}
	}

	pBson->serialize.estimated_len += pData->serialize_estimated_len;
}

// =====================================================================

void
t_bson_boolean_add(tBsonObject *pBson, char *key_ptr, size_t key_len, bool value)
{
	tBsonData *pData;

	if(pBson->type != tBsonType_object)
	{
		TOOLSABNOR("invalid type:%d", pBson->type);
		return;
	}

	pData = t_bson_boolean_build(key_ptr, key_len, value);

	_t_bson_add(pBson, pData);
}

void
t_bson_int_add(tBsonObject *pBson, char *key_ptr, size_t key_len, int value)
{
	tBsonData *pData;

	if(pBson->type != tBsonType_object)
	{
		TOOLSABNOR("invalid type:%d", pBson->type);
		return;
	}

	pData = t_bson_int_build(key_ptr, key_len, value);

	_t_bson_add(pBson, pData);
}

void
t_bson_int64_add(tBsonObject *pBson, char *key_ptr, size_t key_len, u64 value)
{
	tBsonData *pData;

	if(pBson->type != tBsonType_object)
	{
		TOOLSABNOR("invalid type:%d", pBson->type);
		return;
	}

	pData = t_bson_int64_build(key_ptr, key_len, value);

	_t_bson_add(pBson, pData);
}

void
t_bson_double_add(tBsonObject *pBson, char *key_ptr, size_t key_len, double value)
{
	tBsonData *pData;

	if(pBson->type != tBsonType_object)
	{
		TOOLSABNOR("invalid type:%d", pBson->type);
		return;
	}

	pData = t_bson_double_build(key_ptr, key_len, value);

	_t_bson_add(pBson, pData);
}

void
t_bson_string_add(tBsonObject *pBson, char *key_ptr, size_t key_len, char *value_ptr, size_t value_len)
{
	tBsonData *pData;

	if(pBson->type != tBsonType_object)
	{
		TOOLSABNOR("invalid type:%d", pBson->type);
		return;
	}

	pData = t_bson_string_build(key_ptr, key_len, value_ptr, value_len);

	_t_bson_add(pBson, pData);
}

void
t_bson_bin_add(tBsonObject *pBson, char *key_ptr, size_t key_len, char *valur_ptr, size_t value_len)
{
	tBsonData *pData;

	if(pBson->type != tBsonType_object)
	{
		TOOLSABNOR("invalid type:%d", pBson->type);
		return;
	}

	pData = t_bson_bin_build(key_ptr, key_len, valur_ptr, value_len);

	_t_bson_add(pBson, pData);
}

void
t_bson_bin_ins(tBsonObject *pBson, char *key_ptr, size_t key_len, char *valur_ptr, size_t value_len)
{
	tBsonData *pData;

	if(pBson->type != tBsonType_object)
	{
		TOOLSABNOR("invalid type:%d", pBson->type);
		return;
	}

	pData = t_bson_bin_insertion(key_ptr, key_len, valur_ptr, value_len);

	_t_bson_add(pBson, pData);
}

void
t_bson_mbuf_add(tBsonObject *pBson, char *key_ptr, size_t key_len, MBUF *mbuf_data)
{
	tBsonData *pData;

	if(pBson->type != tBsonType_object)
	{
		TOOLSABNOR("invalid type:%d", pBson->type);
		return;
	}

	pData = t_bson_mbuf_build(key_ptr, key_len, mbuf_data);

	_t_bson_add(pBson, pData);
}

void
t_bson_mbuf_ins(tBsonObject *pBson, char *key_ptr, size_t key_len, MBUF *mbuf_data)
{
	tBsonData *pData;

	if(pBson->type != tBsonType_object)
	{
		TOOLSABNOR("invalid type:%d", pBson->type);
		return;
	}

	pData = t_bson_mbuf_insertion(key_ptr, key_len, mbuf_data);

	_t_bson_add(pBson, pData);
}

void
t_bson_object_add(tBsonObject *pBson, char *key_ptr, size_t key_len, tBsonObject *pAddBson)
{
	tBsonData *pData;

	if(pBson->type != tBsonType_object)
	{
		TOOLSABNOR("invalid type:%d", pBson->type);
		return;
	}

	if(pAddBson == NULL)
	{
		return;
	}

	if((pAddBson->type != tBsonType_array) && (pAddBson->type != tBsonType_object))
	{
		TOOLSABNOR("invalid type:%d", pAddBson->type);
		return;
	}

	pData = t_bson_object_build(key_ptr, key_len, pAddBson);

	_t_bson_add(pBson, pData);
}

void
t_bson_array_boolean_add(tBsonObject *pBson, bool value)
{
	tBsonData *pData;

	if(pBson->type != tBsonType_array)
	{
		TOOLSABNOR("invalid type:%d", pBson->type);
		return;
	}

	pData = t_bson_boolean_build(NULL, 0, value);

	_t_bson_add(pBson, pData);
}

void
t_bson_array_int_add(tBsonObject *pBson, int value)
{
	tBsonData *pData;

	if(pBson->type != tBsonType_array)
	{
		TOOLSABNOR("invalid type:%d", pBson->type);
		return;
	}

	pData = t_bson_int_build(NULL, 0, value);

	_t_bson_add(pBson, pData);
}

void
t_bson_array_int64_add(tBsonObject *pBson, u64 value)
{
	tBsonData *pData;

	if(pBson->type != tBsonType_array)
	{
		TOOLSABNOR("invalid type:%d", pBson->type);
		return;
	}

	pData = t_bson_int64_build(NULL, 0, value);

	_t_bson_add(pBson, pData);
}

void
t_bson_array_double_add(tBsonObject *pBson, double value)
{
	tBsonData *pData;

	if(pBson->type != tBsonType_array)
	{
		TOOLSABNOR("invalid type:%d", pBson->type);
		return;
	}

	pData = t_bson_double_build(NULL, 0, value);

	_t_bson_add(pBson, pData);
}

void
t_bson_array_string_add(tBsonObject *pBson, char *value_ptr, size_t value_len)
{
	tBsonData *pData;

	if(pBson->type != tBsonType_array)
	{
		TOOLSABNOR("invalid type:%d", pBson->type);
		return;
	}

	pData = t_bson_string_build(NULL, 0, value_ptr, value_len);

	_t_bson_add(pBson, pData);
}

void
t_bson_array_bin_add(tBsonObject *pBson, char *valur_ptr, size_t value_len)
{
	tBsonData *pData;

	if(pBson->type != tBsonType_array)
	{
		TOOLSABNOR("invalid type:%d", pBson->type);
		return;
	}

	pData = t_bson_bin_build(NULL, 0, valur_ptr, value_len);

	_t_bson_add(pBson, pData);
}

void
t_bson_array_bin_ins(tBsonObject *pBson, char *valur_ptr, size_t value_len)
{
	tBsonData *pData;

	if(pBson->type != tBsonType_array)
	{
		TOOLSABNOR("invalid type:%d", pBson->type);
		return;
	}

	pData = t_bson_bin_insertion(NULL, 0, valur_ptr, value_len);

	_t_bson_add(pBson, pData);
}

void
t_bson_array_mbuf_add(tBsonObject *pBson, MBUF *mbuf_data)
{
	tBsonData *pData;

	if(pBson->type != tBsonType_array)
	{
		TOOLSABNOR("invalid type:%d", pBson->type);
		return;
	}

	pData = t_bson_mbuf_build(NULL, 0, mbuf_data);

	_t_bson_add(pBson, pData);
}

void
t_bson_array_mbuf_ins(tBsonObject *pBson, MBUF *mbuf_data)
{
	tBsonData *pData;

	if(pBson->type != tBsonType_array)
	{
		TOOLSABNOR("invalid type:%d", pBson->type);
		return;
	}

	pData = t_bson_mbuf_insertion(NULL, 0, mbuf_data);

	_t_bson_add(pBson, pData);
}

void
t_bson_array_object_add(tBsonObject *pBson, tBsonObject *pAddBson)
{
	tBsonData *pData;

	if(pBson->type != tBsonType_array)
	{
		TOOLSABNOR("invalid type:%d", pBson->type);
		return;
	}

	if((pAddBson->type != tBsonType_array) && (pAddBson->type != tBsonType_object))
	{
		TOOLSABNOR("invalid type:%d", pAddBson->type);
		return;
	}

	pData = t_bson_object_build(NULL, 0, pAddBson);

	_t_bson_add(pBson, pData);
}

