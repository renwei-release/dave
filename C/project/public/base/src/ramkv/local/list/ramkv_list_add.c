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
#include "ramkv_list_slot.h"
#include "ramkv_list_hash.h"
#include "ramkv_list_tools.h"
#include "ramkv_log.h"

static inline void
_ramkv_add_list(KVList *pKV, KVData *pData)
{
	KVDEBUG("key_ptr:%s pData:%x", pData->key.key_ptr, pData);

	ramkv_list_data_add(pKV, pData);
}

static inline dave_bool
_ramkv_add_slot(KVList *pKV, KVHash *pHash, KVData *pData)
{
	KVSlot *pSlot;
	dave_bool ret;

	pSlot = ramkv_hash_to_slot(pKV->slot, pHash, dave_true);

	if(pSlot != NULL)
	{
		KVDEBUG("key_ptr:%s pSlot:%x", pData->key.key_ptr, pSlot);

		ret = ramkv_slot_data_add(pSlot, pData);
	}
	else
	{
		ret = dave_false;
	}

	return ret;
}

// ====================================================================

dave_bool
__ramkv_list_add__(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line)
{
	KVHash hash;
	KVData *pData, *pOldData = NULL;
	KVSlot *pSlot;
	dave_bool ret = dave_false;
	dave_bool free_data_flag = dave_false;

	if((key_ptr == NULL) || (key_len == 0) || (value_ptr == NULL) || (value_len == 0))
	{
		KVLOG("invalid param, key_ptr:%x key_len:%d value_ptr:%x value_len:%d <%s:%d>",
			key_ptr, key_len, value_ptr, value_len,
			fun, line);
		return dave_false;
	}

	if(ramkv_hash(pKV, &hash, key_ptr, key_len, fun, line) == dave_false)
	{
		KVLOG("invalid key_len:%d <%s:%d>", key_len, fun, line);
		return dave_false;
	}

	pData = ramkv_list_data_malloc(key_ptr, key_len, value_ptr, value_len);

	SAFECODEv2W(pKV->ramkv_pv, {

		pSlot = ramkv_hash_to_slot(pKV->local.ramkv_list.slot, &hash, dave_false);
		if(pSlot != NULL)
		{
			pOldData = ramkv_slot_data_inq(pSlot, key_ptr, key_len);
		}

		if(pOldData == NULL)
		{
			_ramkv_add_list(&(pKV->local.ramkv_list), pData);

			ret = _ramkv_add_slot(&(pKV->local.ramkv_list), &hash, pData);
		}
		else
		{
			free_data_flag = dave_true;

			ramkv_list_data_copy_from_user(pOldData, value_ptr, value_len);

			ret = dave_true;
		}

	});

	if(free_data_flag == dave_true)
	{
		ramkv_list_data_free(pData);
	}

	return ret;
}

#endif

