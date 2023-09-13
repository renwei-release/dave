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

static inline tBsonData *
_t_bson_key_inq(tBsonObject *pBson, char *key_ptr, size_t key_len)
{
	u64 bloom_filter;
	tBsonData *pInq;

	if(pBson == NULL)
		return NULL;

	if(key_len <= 0)
		key_len = strlen(key_ptr);

	bloom_filter = t_bson_bloom(key_ptr, key_len);
	if((bloom_filter & pBson->key_bloom_filter) == 0x00)
	{
		return NULL;
	}

	if(pBson->pCurKeyFind != NULL)
	{
		pInq = pBson->pCurKeyFind;

		if((pInq->key_len == key_len)
			&& (memcmp(pInq->key_ptr, key_ptr, key_len) == 0))
		{
			pBson->pCurKeyFind = (tBsonData *)(pInq->next);

			return pInq;
		}		
	}

	pInq = pBson->pDataHead;

	while(pInq != NULL)
	{
		if((pInq->key_len == key_len)
			&& (memcmp(pInq->key_ptr, key_ptr, key_len) == 0))
		{
			pBson->pCurKeyFind = (tBsonData *)(pInq->next);

			return pInq;
		}

		pInq = (tBsonData *)(pInq->next);
	}

	return NULL;
}

static inline tBsonData *
_t_bson_index_inq(tBsonObject *pBson, size_t inq_index)
{
	tBsonData *pInq;
	size_t index;

	if(pBson->pCurIndexFind == NULL)
	{
		pInq = pBson->pDataHead;
		index = 0;
	}
	else if(inq_index >= pBson->pCurIndexFind->list_index)
	{
		pInq = pBson->pCurIndexFind;
		index = pBson->pCurIndexFind->list_index;
	}
	else
	{
		TOOLSLOG("There is a reverse lookup, and the algorithm needs to be optimized %x %d/%d",
			pBson, inq_index, pBson->pCurIndexFind->list_index);
		pInq = pBson->pDataHead;
		index = 0;
	}

	while(pInq != NULL)
	{
		if(index == inq_index)
		{
			pBson->pCurIndexFind = (tBsonData *)(pInq->next);

			return pInq;
		}

		pInq = (tBsonData *)(pInq->next);

		index ++;
	}

	return NULL;
}

static inline bool
_t_bson_bin_inq(tBsonData *pData, char **ppBinValue, size_t *pBinLen)
{
	if(pData->value_ptr.mem_value == NULL)
		return false;

	*ppBinValue = pData->value_ptr.mem_value;
	*pBinLen = pData->value_len;

	return true;
}

// =====================================================================

bool
t_bson_boolean_inq(tBsonObject *pBson, char *key_ptr, size_t key_len, bool *value)
{
	tBsonData *pData = _t_bson_key_inq(pBson, key_ptr, key_len);

	*value = false;

	if(pData == NULL)
		return false;

	if(pData->type != tBsonType_boolean)
	{
		TOOLSLOG("invalid type:%d key:%d/%s",
			pData->type, pData->key_len, pData->key_ptr);
		return false;
	}

	*value = pData->value_ptr.bool_value;

	return true;
}

bool
t_bson_int_inq(tBsonObject *pBson, char *key_ptr, size_t key_len, int *value)
{
	tBsonData *pData = _t_bson_key_inq(pBson, key_ptr, key_len);

	*value = 0;

	if(pData == NULL)
		return false;

	if(pData->type != tBsonType_int)
	{
		TOOLSLOG("invalid type:%d key:%d/%s",
			pData->type, pData->key_len, pData->key_ptr);
		return false;
	}

	*value = pData->value_ptr.int_value;

	return true;
}

bool
t_bson_int64_inq(tBsonObject *pBson, char *key_ptr, size_t key_len, u64 *value)
{
	tBsonData *pData = _t_bson_key_inq(pBson, key_ptr, key_len);

	*value = 0;

	if(pData == NULL)
		return false;

	if(pData->type != tBsonType_int64)
	{
		TOOLSLOG("invalid type:%d key:%d/%s",
			pData->type, pData->key_len, pData->key_ptr);
		return false;
	}

	*value = pData->value_ptr.int64_value;

	return true;
}

bool
t_bson_double_inq(tBsonObject *pBson, char *key_ptr, size_t key_len, double *value)
{
	tBsonData *pData = _t_bson_key_inq(pBson, key_ptr, key_len);

	*value = 0;

	if(pData == NULL)
		return false;

	if(pData->type != tBsonType_double)
	{
		TOOLSLOG("invalid type:%d key:%d/%s",
			pData->type, pData->key_len, pData->key_ptr);
		return false;
	}

	*value = pData->value_ptr.double_value;

	return true;
}

bool
t_bson_string_inq(tBsonObject *pBson, char *key_ptr, size_t key_len, char **ppStringValue, size_t *pStringLen)
{
	tBsonData *pData = _t_bson_key_inq(pBson, key_ptr, key_len);

	*ppStringValue = NULL;
	*pStringLen = 0;

	if(pData == NULL)
		return false;

	if(pData->type != tBsonType_string)
	{
		TOOLSLOG("invalid type:%d key:%d/%s",
			pData->type, pData->key_len, pData->key_ptr);
		return false;
	}

	if(pData->value_ptr.mem_value == NULL)
		return false;

	*ppStringValue = pData->value_ptr.mem_value;
	if(pStringLen != NULL)
		*pStringLen = pData->value_len;

	return true;
}

bool
t_bson_bin_inq(tBsonObject *pBson, char *key_ptr, size_t key_len, char **ppBinValue, size_t *pBinLen)
{
	tBsonData *pData = _t_bson_key_inq(pBson, key_ptr, key_len);

	*ppBinValue = NULL;
	*pBinLen = 0;

	if(pData == NULL)
		return false;

	if((pData->type == tBsonType_bin_insertion)
		|| (pData->type == tBsonType_bin))
	{
		return _t_bson_bin_inq(pData, ppBinValue, pBinLen);
	}
	else
	{
		TOOLSLOG("invalid type:%d key:%d/%s",
			pData->type, pData->key_len, pData->key_ptr);
		return false;
	}
}

bool
t_bson_object_inq(tBsonObject *pBson, char *key_ptr, size_t key_len, tBsonObject **ppObject)
{
	tBsonData *pData = _t_bson_key_inq(pBson, key_ptr, key_len);

	*ppObject = NULL;

	if(pData == NULL)
		return false;

	if((pData->type != tBsonType_array) && (pData->type != tBsonType_object))
	{
		TOOLSLOG("invalid type:%d key:%d/%s",
			pData->type, pData->key_len, pData->key_ptr);
		return false;
	}

	if(pData->value_ptr.object_value == NULL)
		return false;

	*ppObject = pData->value_ptr.object_value;

	return true;
}

size_t
t_bson_array_number_inq(tBsonObject *pBson)
{
	size_t number;
	tBsonData *pInq;

	if(pBson == NULL)
		return 0;

	if(pBson->type != tBsonType_array)
	{
		TOOLSLOG("invalid type:%d",
			pBson->type);
		return 0;
	}

	number = 0;

	pInq = pBson->pDataHead;

	while(pInq != NULL)
	{
		number ++;
		pInq = (tBsonData *)(pInq->next);
	}

	return number;
}

bool
t_bson_array_boolean_inq(tBsonObject *pBson, size_t index, bool *value)
{
	tBsonData *pData = _t_bson_index_inq(pBson, index);

	*value = false;

	if(pData == NULL)
		return false;

	if(pData->type != tBsonType_boolean)
	{
		TOOLSLOG("invalid type:%d key:%d/%s",
			pData->type, pData->key_len, pData->key_ptr);
		return false;
	}

	*value = pData->value_ptr.bool_value;

	return true;
}

bool
t_bson_array_int_inq(tBsonObject *pBson, size_t index, int *value)
{
	tBsonData *pData = _t_bson_index_inq(pBson, index);

	*value = 0;

	if(pData == NULL)
		return false;

	if(pData->type != tBsonType_int)
	{
		TOOLSLOG("invalid type:%d key:%d/%s",
			pData->type, pData->key_len, pData->key_ptr);
		return false;
	}

	*value = pData->value_ptr.int_value;

	return true;
}

bool
t_bson_array_int64_inq(tBsonObject *pBson, size_t index, u64 *value)
{
	tBsonData *pData = _t_bson_index_inq(pBson, index);

	*value = 0;

	if(pData == NULL)
		return false;

	if(pData->type != tBsonType_int64)
	{
		TOOLSLOG("invalid type:%d key:%d/%s",
			pData->type, pData->key_len, pData->key_ptr);
		return false;
	}

	*value = pData->value_ptr.int64_value;

	return true;
}

bool
t_bson_array_double_inq(tBsonObject *pBson, size_t index, double *value)
{
	tBsonData *pData = _t_bson_index_inq(pBson, index);

	*value = 0;

	if(pData == NULL)
		return false;

	if(pData->type != tBsonType_double)
	{
		TOOLSLOG("invalid type:%d key:%d/%s",
			pData->type, pData->key_len, pData->key_ptr);
		return false;
	}

	*value = pData->value_ptr.double_value;

	return true;
}

bool
t_bson_array_string_inq(tBsonObject *pBson, size_t index, char **ppStringValue, size_t *pStringLen)
{
	tBsonData *pData = _t_bson_index_inq(pBson, index);

	*ppStringValue = NULL;
	*pStringLen = 0;

	if(pData == NULL)
		return false;

	if(pData->type != tBsonType_string)
	{
		TOOLSLOG("invalid type:%d key:%d/%s",
			pData->type, pData->key_len, pData->key_ptr);
		return false;
	}

	if(pData->value_ptr.mem_value == NULL)
		return false;

	*ppStringValue = pData->value_ptr.mem_value;
	*pStringLen = pData->value_len;

	return true;
}

bool
t_bson_array_bin_inq(tBsonObject *pBson, size_t index, char **ppBinValue, size_t *pBinLen)
{
	tBsonData *pData = _t_bson_index_inq(pBson, index);

	*ppBinValue = NULL;
	*pBinLen = 0;

	if(pData == NULL)
		return false;

	if((pData->type == tBsonType_bin_insertion)
		|| (pData->type == tBsonType_bin))
	{
		return _t_bson_bin_inq(pData, ppBinValue, pBinLen);
	}
	else
	{
		TOOLSLOG("invalid type:%d key:%d/%s",
			pData->type, pData->key_len, pData->key_ptr);
		return false;
	}
}

bool
t_bson_array_object_inq(tBsonObject *pBson, size_t index, tBsonObject **ppObject)
{
	tBsonData *pData = _t_bson_index_inq(pBson, index);

	*ppObject = NULL;

	if(pData == NULL)
		return false;

	if(pData->type != tBsonType_object)
	{
		TOOLSLOG("invalid type:%d key:%d/%s",
			pData->type, pData->key_len, pData->key_ptr);
		return false;
	}

	if(pData->value_ptr.object_value == NULL)
		return false;

	*ppObject = pData->value_ptr.object_value;

	return true;
}

