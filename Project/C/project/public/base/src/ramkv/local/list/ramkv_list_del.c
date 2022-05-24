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

// ====================================================================

ub
__ramkv_list_del__(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line)
{
	KVHash hash;
	KVSlot **ppSlot;
	KVData *pData;
	ub ret;

	if((key_ptr == NULL) || (key_len == 0))
	{
		KVDEBUG("invalid param, key_ptr:%x key_len:%d", key_ptr, key_len);
		return dave_false;
	}

	if(ramkv_hash(pKV, &hash, key_ptr, key_len, fun, line) == dave_false)
	{
		KVLOG("invalid key_len:%d <%s:%d>", key_len, fun, line);
		return dave_false;
	}

	ret = 0;

	SAFECODEv2W(pKV->ramkv_pv, {

		ppSlot = ramkv_hash_to_pslot(pKV->local.ramkv_list.slot, &hash, dave_false);
		if(ppSlot != NULL)
		{
			KVDEBUG("key_ptr:%s pSlot:%x", key_ptr, ppSlot);

			pData = ramkv_slot_data_del(ppSlot, key_ptr, key_len);

			if(pData != NULL)
			{
				KVDEBUG("pData:%x %d[%x %x]",
					pData,
					pData->key.key_len, pData->key.key_ptr[0], pData->key.key_ptr[1]);

				ret = ramkv_list_data_copy_to_user(pData, value_ptr, value_len);

				ramkv_list_data_del(&(pKV->local.ramkv_list), pData);
			}
		}

	} );

	return ret;
}

#endif

