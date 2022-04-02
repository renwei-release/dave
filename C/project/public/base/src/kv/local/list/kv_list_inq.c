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
#include "kv_param.h"
#include "kv_list_data.h"
#include "kv_list_slot.h"
#include "kv_list_hash.h"
#include "kv_list_tools.h"
#include "kv_log.h"

static inline KVSlot *
_kv_list_inq_slot(KVList *pKV, KVHash *pHash, u8 *key_ptr, ub key_len)
{
	return kv_hash_to_slot(pKV->slot, pHash, dave_false);
}

static inline KVData *
_kv_list_inq_data(KVSlot *pSlot, u8 *key_ptr, ub key_len)
{
	return kv_slot_data_inq(pSlot, key_ptr, key_len);
}

static inline ub
_kv_list_inq_index(KV *pKV, sb index, void *value_ptr, ub value_len)
{
	KVData *pData;
	sb inq_index = 0;
	ub inq_len = 0;

	SAFEZONEv5R(pKV->kv_pv, {

		pData = pKV->local.kv_list.list_data_head;

		while((inq_index < index) && (pData != NULL))
		{
			inq_index ++;
			pData = (KVData *)(pData->for_list_next);
		}

		if((inq_index == index) && (pData != NULL))
		{
			KVDEBUG("pData:%s", pData->key.key_ptr);

			inq_len = kv_list_data_copy_to_user(pData, value_ptr, value_len);
		}

	} );

	KVDEBUG("inq_len:%d", inq_len);

	return inq_len;
}

static inline dave_bool
_kv_list_inq_top(KV *pKV, u8 *key_ptr, ub key_len)
{
	KVData *pData;
	dave_bool ret = dave_false;

	dave_memset(key_ptr, 0x00, key_len);

	SAFEZONEv5R(pKV->kv_pv, {

		pData = pKV->local.kv_list.list_data_head;
		if(pData != NULL)
		{
			key_len = key_len - 1;
			if(key_len < pData->key.key_len)
			{
				KVLOG("too short key_len:%d/%d", key_len, pData->key.key_len);
			}
			else
			{
				key_len = pData->key.key_len;
			}
			dave_memcpy(key_ptr, pData->key.key_ptr, key_len);
			ret = dave_true;
		}

	} );

	return ret;
}

static inline ub
_kv_list_inq_normal(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line)
{
	KVHash hash;
	KVSlot *pSlot;
	KVData *pData;
	ub inq_len = 0;

	if(kv_hash(pKV, &hash, key_ptr, key_len, fun, line) == dave_false)
	{
		KVLOG("invalid key_len:%d <%s:%d>", key_len, fun, line);
		return 0;
	}

	SAFEZONEv5R(pKV->kv_pv, {

		pSlot = _kv_list_inq_slot(&(pKV->local.kv_list), &hash, key_ptr, key_len);
		if(pSlot != NULL)
		{
			KVDEBUG("key_ptr:%s pSlot:%x", key_ptr, pSlot);

			pData = _kv_list_inq_data(pSlot, key_ptr, key_len);
			if(pData != NULL)
			{
				KVDEBUG("key_ptr:%s pData:%x", key_ptr, pData);

				inq_len = kv_list_data_copy_to_user(pData, value_ptr, value_len);
			}
		}

	} );

	KVDEBUG("key:%s inq_len:%d", key_ptr, inq_len);

	return inq_len;
}

// ====================================================================

ub
__kv_list_inq__(KV *pKV, sb index, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line)
{
	if((value_ptr == NULL) || (value_len == 0))
	{
		KVLOG("invalid param, key_ptr:%x key_len:%d value_ptr:%x value_len:%d, fun:%s:%d",
			key_ptr, key_len, value_ptr, value_len, fun, line);
		return 0;
	}

	if(index >= 0)
	{
		return _kv_list_inq_index(pKV, index, value_ptr, value_len);
	}
	else if(key_ptr == NULL)
	{
		/* 如果没有给出索引KEY，那么就给出第一个数据 */
		return _kv_list_inq_index(pKV, 0, value_ptr, value_len);
	}
	else if(key_len == 0)
	{
		/* 如果给出索引KEY，但KEY为空，那么不能找到数据 */
		return 0;
	}
	else
	{
		return _kv_list_inq_normal(pKV, key_ptr, key_len, value_ptr, value_len, fun, line);
	}
}

dave_bool
__kv_list_top__(KV *pKV, u8 *key_ptr, ub key_len)
{
	if((key_ptr == NULL) || (key_len <= 1))
	{
		KVLOG("invalid key_ptr:%x or key_len:%d", key_ptr, key_len);
		return dave_false;
	}

	return _kv_list_inq_top(pKV, key_ptr, key_len);
}

#endif

