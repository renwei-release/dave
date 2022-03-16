/*
 * ================================================================================
 * (c) Copyright 2020 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.10.22.
 * ================================================================================
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

static inline void
_kv_add_list(KVList *pKV, KVData *pData)
{
	KVDEBUG("key_ptr:%s pData:%x", pData->key.key_ptr, pData);

	kv_list_data_add(pKV, pData);
}

static inline dave_bool
_kv_add_slot(KVList *pKV, KVHash *pHash, KVData *pData)
{
	KVSlot *pSlot;
	dave_bool ret;

	pSlot = kv_hash_to_slot(pKV->slot, pHash, dave_true);

	if(pSlot != NULL)
	{
		KVDEBUG("key_ptr:%s pSlot:%x", pData->key.key_ptr, pSlot);

		ret = kv_slot_data_add(pSlot, pData);
	}
	else
	{
		ret = dave_false;
	}

	return ret;
}

// ====================================================================

dave_bool
__kv_list_add__(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line)
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

	if(kv_hash(pKV, &hash, key_ptr, key_len, fun, line) == dave_false)
	{
		KVLOG("invalid key_len:%d <%s:%d>", key_len, fun, line);
		return dave_false;
	}

	pData = kv_list_data_malloc(key_ptr, key_len, value_ptr, value_len);

	SAFEZONEv5W(pKV->kv_pv, {

		pSlot = kv_hash_to_slot(pKV->local.kv_list.slot, &hash, dave_false);
		if(pSlot != NULL)
		{
			pOldData = kv_slot_data_inq(pSlot, key_ptr, key_len);
		}

		if(pOldData == NULL)
		{
			_kv_add_list(&(pKV->local.kv_list), pData);

			ret = _kv_add_slot(&(pKV->local.kv_list), &hash, pData);
		}
		else
		{
			free_data_flag = dave_true;

			kv_list_data_copy_from_user(pOldData, value_ptr, value_len);

			ret = dave_true;
		}

	});

	if(free_data_flag == dave_true)
	{
		kv_list_data_free(pData);
	}

	return ret;
}

#endif

