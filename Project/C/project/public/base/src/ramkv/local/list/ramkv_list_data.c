/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_os.h"
#include "dave_tools.h"
#include "ramkv_param.h"
#include "ramkv_list_data.h"
#include "ramkv_list_tools.h"
#include "ramkv_log.h"

static inline void
_ramkv_list_data_copy_from_user(KVValue *pValue, void *value_ptr, ub value_len)
{
	if((value_ptr == NULL) || (value_len == 0))
	{
		if(pValue->value_len > sizeof(void *))
		{
			if(pValue->value_ptr != NULL)
			{
				ramkvm_free(pValue->value_ptr);
			}
		}

		pValue->value_len = 0;
		pValue->value_ptr = NULL;
	}
	else
	{
		if(value_len > sizeof(void *))
		{
			if(pValue->value_len != value_len)
			{
				if((pValue->value_ptr != NULL) && (pValue->value_len > sizeof(void *)))
				{
					ramkvm_free(pValue->value_ptr);
				}
				pValue->value_len = value_len;
				pValue->value_ptr = ramkvm_malloc(pValue->value_len);
			}
			dave_memcpy(pValue->value_ptr, value_ptr, pValue->value_len);
		}
		else
		{
			if((pValue->value_ptr != NULL) && (pValue->value_len > sizeof(void *)))
			{
				ramkvm_free(pValue->value_ptr);
			}
			pValue->value_len = value_len;
			dave_memcpy(&(pValue->value_ptr), value_ptr, pValue->value_len);
		}
	}

	ramkv_list_build_value_checksum(pValue);
}

static inline ub
_ramkv_list_data_copy_to_user(KVValue *pValue, void *value_ptr, ub value_len)
{
	ub copy_value_len;

	if(value_ptr != NULL)
	{
		((s8 *)value_ptr)[0] = '\0';
	}

	if(pValue->value_len == 0)
	{
		return 0;
	}

	if(ramkv_list_check_value_checksum(pValue) == dave_false)
	{
		KVLTRACE(60,1,"value check failed:%lx/%lx!", pValue->value_ptr, pValue->value_checksum);
	}

	copy_value_len = value_len;
	if(copy_value_len > pValue->value_len)
	{
		copy_value_len = pValue->value_len;
	}

	if(pValue->value_len > sizeof(void *))
	{
		dave_memcpy(value_ptr, pValue->value_ptr, copy_value_len);
	}
	else
	{
		dave_memcpy(value_ptr, &(pValue->value_ptr), copy_value_len);
	}

	if(value_len > copy_value_len)
	{
		((s8 *)value_ptr)[copy_value_len] = '\0';
	}

	return copy_value_len;
}

static inline void
_ramkv_list_data_clean(KVValue *pValue)
{
	if(pValue->value_len > sizeof(void *))
	{
		if(pValue->value_ptr != NULL)
		{
			ramkvm_free(pValue->value_ptr);
		}
	}
	pValue->value_len = 0;
	pValue->value_ptr = NULL;
}

static inline void
_ramkv_list_data_key_malloc(KVKey *pKey, u8 *key_ptr, ub key_len)
{
	if(key_len >= RAMKV_KEY_MAX)
	{
		KVLOG("too longer key:%d/%s", key_len);
		key_len = (RAMKV_KEY_MAX - 1);
	}
	pKey->key_len = key_len;
	dave_memcpy(pKey->key_ptr, key_ptr, pKey->key_len);
	pKey->key_ptr[RAMKV_KEY_MAX - 1] = '\0';
}

static inline void
_ramkv_list_data_key_free(KVKey *pKey)
{
	pKey->key_len = 0;
}

static inline void
_ramkv_list_data_value_malloc(KVValue *pValue, void *value_ptr, ub value_len)
{
	_ramkv_list_data_copy_from_user(pValue, value_ptr, value_len);
}

static inline void
_ramkv_list_data_value_free(KVValue *pValue)
{
	_ramkv_list_data_clean(pValue);
}

static inline KVData *
_ramkv_list_data_malloc(s8 *fun, ub line)
{
	KVData *pData;

	pData = ramkvm_malloc_line(sizeof(KVData), fun, line);

	pData->magic_data = KV_DATA_MAGIC_DATA;

	pData->key.key_len = 0;
	pData->value.value_len = 0;
	pData->value.value_ptr = NULL;
	pData->value.value_checksum = 0;

	pData->for_slot_up = pData->for_slot_next = NULL;
	pData->for_list_up = pData->for_list_next = NULL;

	return pData;
}

static inline void
_ramkv_list_data_free(KVData *pData)
{
	ramkvm_free(pData);
}

static inline KVData *
_ramkv_list_data_malloc_(u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line)
{
	KVData *pData;

	pData = _ramkv_list_data_malloc(fun, line);

	_ramkv_list_data_key_malloc(&(pData->key), key_ptr, key_len);

	_ramkv_list_data_value_malloc(&(pData->value), value_ptr, value_len);

	return pData;
}

static inline void
_ramkv_list_data_free_(KVData *pData)
{
	_ramkv_list_data_key_free(&(pData->key));
	
	_ramkv_list_data_value_free(&(pData->value));
	
	_ramkv_list_data_free(pData);
}

static inline void
_ramkv_list_data_del_head(KVList *pKV, KVData *pData)
{
	if(pKV->list_data_head == pData)
	{
		pKV->list_data_head = pKV->list_data_head->for_list_next;

		if(pKV->list_data_head == NULL)
		{
			if(pKV->list_data_tail != pData)
			{
				KVABNOR("Arithmetic error! %x/%x", pKV->list_data_tail, pData);
			}
			pKV->list_data_tail = NULL;
		}
		else
		{
			if(pKV->list_data_head->for_list_up != pData)
			{
				KVABNOR("Arithmetic error! %x/%x", pKV->list_data_head->for_list_up, pData);
			}
			pKV->list_data_head->for_list_up = NULL;
		}
	}
}

static inline void
_ramkv_list_data_del_tail(KVList *pKV, KVData *pData)
{
	if(pKV->list_data_tail == pData)
	{
		pKV->list_data_tail = pKV->list_data_tail->for_list_up;

		if(pKV->list_data_tail == NULL)
		{
			if(pKV->list_data_head != pData)
			{
				KVABNOR("Arithmetic error! %x/%x", pKV->list_data_head, pData);
			}
			pKV->list_data_head = NULL;
		}
		else
		{
			if(pKV->list_data_tail->for_list_next != pData)
			{
				KVABNOR("Arithmetic error! %x/%x", pKV->list_data_tail->for_list_next, pData);
			}
			pKV->list_data_tail->for_list_next = NULL;
		}
	}
}

static inline void
_ramkv_list_data_del_mid(KVList *pKV, KVData *pData)
{
	if(pData->for_list_up != NULL)
	{
		((KVData *)(pData->for_list_up))->for_list_next = pData->for_list_next;
	}

	if(pData->for_list_next != NULL)
	{
		((KVData *)(pData->for_list_next))->for_list_up = pData->for_list_up;
	}
}

// ====================================================================

KVData *
__ramkv_list_data_malloc__(u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line)
{
	return _ramkv_list_data_malloc_(key_ptr, key_len, value_ptr, value_len, fun, line);
}

void
ramkv_list_data_free(KVData *pData)
{
	KVData *pTemp;

	while(pData != NULL)
	{
		pTemp = pData->for_list_next;

		_ramkv_list_data_free_(pData);

		pData = pTemp;
	}
}

void
ramkv_list_data_add(KVList *pKV, KVData *pData)
{
	if(pKV->list_data_head == NULL)
	{
		pKV->list_data_head = pKV->list_data_tail = pData;
	}
	else
	{
		pData->for_list_up = pKV->list_data_tail;

		pKV->list_data_tail->for_list_next = pData;

		pKV->list_data_tail = pData;
	}
}

void
ramkv_list_data_del(KVList *pKV, KVData *pData)
{
	if(pKV->list_data_head == pData)
	{
		_ramkv_list_data_del_head(pKV, pData);
	}
	else if(pKV->list_data_tail == pData)
	{
		_ramkv_list_data_del_tail(pKV, pData);
	}
	else
	{
		_ramkv_list_data_del_mid(pKV, pData);
	}

	_ramkv_list_data_free_(pData);
}

void
ramkv_list_data_copy_from_user(KVData *pData, void *value_ptr, ub value_len)
{
	_ramkv_list_data_copy_from_user(&(pData->value), value_ptr, value_len);
}

ub
ramkv_list_data_copy_to_user(KVData *pData, void *value_ptr, ub value_len)
{
	if((value_ptr == NULL) || (value_len == 0))
		return 0;
	return _ramkv_list_data_copy_to_user(&(pData->value), value_ptr, value_len);
}

#endif

