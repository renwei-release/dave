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
#include "kv_list_slot.h"
#include "kv_list_bloom_filter.h"
#include "kv_list_tools.h"
#include "kv_log.h"

static inline void
_kv_slot_data_del_head(KVSlot *pSlot, KVData *pData)
{
	if(pSlot->slot_data_head == pData)
	{
		pSlot->slot_data_head = ((KVData *)(pSlot->slot_data_head))->for_slot_next;

		if(pSlot->slot_data_head == NULL)
		{
			if(pSlot->slot_data_tail != pData)
			{
				KVABNOR("Arithmetic error! %x/%x", pSlot->slot_data_tail, pData);
			}
			pSlot->slot_data_tail = NULL;
		}
		else
		{
			if(((KVData *)pSlot->slot_data_head)->for_slot_up != pData)
			{
				KVABNOR("Arithmetic error! %x/%x", ((KVData *)pSlot->slot_data_head)->for_slot_up, pData);
			}
			((KVData *)pSlot->slot_data_head)->for_slot_up = NULL;
		}
	}
}

static inline void
_kv_slot_data_del_tail(KVSlot *pSlot, KVData *pData)
{
	if(pSlot->slot_data_tail == pData)
	{
		pSlot->slot_data_tail = ((KVData *)(pSlot->slot_data_tail))->for_slot_up;

		if(pSlot->slot_data_tail == NULL)
		{
			if(pSlot->slot_data_head != pData)
			{
				KVABNOR("Arithmetic error! %x/%x", pSlot->slot_data_head, pData);
			}
			pSlot->slot_data_head = NULL;
		}
		else
		{
			if(((KVData *)pSlot->slot_data_tail)->for_slot_next != pData)
			{
				KVABNOR("Arithmetic error! %x/%x", ((KVData *)pSlot->slot_data_tail)->for_slot_next, pData);
			}
			((KVData *)pSlot->slot_data_tail)->for_slot_next = NULL;
		}
	}
}

static inline void
_kv_slot_data_del_mid(KVSlot *pSlot, KVData *pData)
{
	if(pData->for_slot_up != NULL)
	{
		((KVData *)(pData->for_slot_up))->for_slot_next = pData->for_slot_next;
	}
	
	if(pData->for_slot_next != NULL)
	{
		((KVData *)(pData->for_slot_next))->for_slot_up = pData->for_slot_up;
	}
}

static inline KVSlot *
_kv_slot_slot_malloc(KVSlot *pSlot, KVSlot **up_ppslot)
{
	ub slot_index;

	pSlot->up_ppslot = (void **)up_ppslot;

	for(slot_index=0; slot_index<KV_SLOT_NUM; slot_index++)
	{
		pSlot->slot[slot_index] = NULL;
	}

	kv_bloom_filter_reset(&(pSlot->bloom_filter));
	pSlot->slot_data_number = 0;
	pSlot->slot_data_head = pSlot->slot_data_tail = NULL;

	return pSlot;
}

static inline void
_kv_slot_slot_free(KVSlot *pSlot)
{
	ub slot_index;

	pSlot->up_ppslot = NULL;

	for(slot_index=0; slot_index<KV_SLOT_NUM; slot_index++)
	{
		if(pSlot->slot[slot_index] != NULL)
		{
			kv_slot_free((KVSlot *)(pSlot->slot[slot_index]));

			pSlot->slot[slot_index] = NULL;
		}
	}

	kv_bloom_filter_reset(&(pSlot->bloom_filter));
	pSlot->slot_data_number = 0;
	pSlot->slot_data_head = pSlot->slot_data_tail = NULL;
}

static inline dave_bool
_kv_slot_data_add(KVSlot *pSlot, KVData *pData)
{
	if(pSlot->slot_data_head == NULL)
	{
		pSlot->slot_data_head = pSlot->slot_data_tail = pData;
	}
	else
	{
		((KVData *)(pSlot->slot_data_tail))->for_slot_next = pData;

		pData->for_slot_up = pSlot->slot_data_tail;

		pSlot->slot_data_tail = pData;
	}

	return dave_true;
}

static inline KVData *
_kv_slot_data_inq(KVSlot *pSlot, u8 *key_ptr, ub key_len)
{
	KVData *pDataFind = pSlot->slot_data_head;
	KVData *pData = NULL;
	sb safe_counter;

	if((pDataFind == NULL) || (kv_bloom_filter_check(&(pSlot->bloom_filter), key_ptr, key_len) == dave_false))
	{
		KVDEBUG("key_ptr:%s", key_ptr);
		return NULL;
	}

	safe_counter = 0;

	while((pDataFind != NULL) && ((++ safe_counter) <= pSlot->slot_data_number))
	{
		if(pDataFind->magic_data != KV_DATA_MAGIC_DATA)
		{
			KVLTRACE(60,1,"invalid magic_data:%lx", pDataFind->magic_data);
			break;
		}

		if(kv_is_my_key(pDataFind, key_ptr, key_len) == dave_true)
		{
			pData = pDataFind;
			break;
		}

		pDataFind = pDataFind->for_slot_next;
	}

	if(safe_counter > pSlot->slot_data_number)
	{
		KVLTRACE(60,1,"There was a problem looking up the linked list! %d/%d", safe_counter, pSlot->slot_data_number);
	}

	if((pData != NULL) && (pData->magic_data != KV_DATA_MAGIC_DATA))
	{
		KVLTRACE(60,1,"find invalid magic data:%lx", pData->magic_data);
		pData = NULL;
	}

	return pData;
}

static inline void
_kv_slot_data_del(KVSlot *pSlot, KVData *pData)
{
	if(pSlot->slot_data_head == pData)
	{
		_kv_slot_data_del_head(pSlot, pData);
	}
	else if(pSlot->slot_data_tail == pData)
	{
		_kv_slot_data_del_tail(pSlot, pData);
	}
	else
	{
		_kv_slot_data_del_mid(pSlot, pData);
	}
}

static inline void
_kv_slot_rebuild_bloom_filter(KVSlot *pSlot)
{
	KVData *pData;

	kv_bloom_filter_reset(&(pSlot->bloom_filter));

	pData = pSlot->slot_data_head;

	while(pData != NULL)
	{
		kv_bloom_filter_build(&(pSlot->bloom_filter), pData->key.key_ptr, pData->key.key_len);

		pData = (KVData *)(pData->for_slot_next);
	}
}

static inline dave_bool
_kv_slot_can_free_the_slot(KVSlot *pSlot)
{
	ub slot_index;

	if(pSlot->slot_data_head != NULL)
		return dave_false;

	if(pSlot->slot_data_tail != NULL)
	{
		KVABNOR("Arithmetic error! %x/%x", pSlot->slot_data_head, pSlot->slot_data_tail);
	}

	for(slot_index=0; slot_index<KV_SLOT_NUM; slot_index++)
	{
		if(pSlot->slot[slot_index] != NULL)
			return dave_false;
	}

	return dave_true;
}

static inline void
_kv_slot_recover(KVSlot **ppSlot)
{
	KVSlot **up_ppslot;
	ub safe_counter;

	safe_counter = 0;

	while((ppSlot != NULL) && ((*ppSlot) != NULL) && ((++ safe_counter) <= KV_SLOT_DEPTH))
	{
		up_ppslot = (KVSlot **)((*ppSlot)->up_ppslot);

		if(_kv_slot_can_free_the_slot(*ppSlot) == dave_true)
		{
			KVDEBUG("you can free the slot:%x on %x!", *ppSlot, ppSlot);

			kv_slot_free(*ppSlot);
			*ppSlot = NULL;
		}

		if(up_ppslot == NULL)
			ppSlot = NULL;
		else
			ppSlot = up_ppslot;
	}

	if(safe_counter > KV_SLOT_DEPTH)
	{
		KVLTRACE(60,1,"Found a strange data depth information:%d!", safe_counter);
	}
}

// ====================================================================

KVSlot *
kv_slot_malloc(KVSlot **up_ppslot)
{
	KVSlot *pSlot;

	pSlot = kvm_malloc(sizeof(KVSlot));

	pSlot->magic_data = KV_SLOT_MAGIC_DATA;	

	KVDEBUG("%x", pSlot);

	return _kv_slot_slot_malloc(pSlot, up_ppslot);
}

void
kv_slot_free(KVSlot *pSlot)
{
	if(pSlot->magic_data == KV_SLOT_MAGIC_DATA)
	{
		KVDEBUG("%x", pSlot);

		_kv_slot_slot_free(pSlot);

		pSlot->magic_data = 0x00;

		kvm_free(pSlot);
	}
	else
	{
		KVLOG("find invalid magic data:%lx", pSlot->magic_data);
	}
}

dave_bool
kv_slot_data_add(KVSlot *pSlot, KVData *pData)
{
	kv_bloom_filter_build(&(pSlot->bloom_filter), pData->key.key_ptr, pData->key.key_len);

	pSlot->slot_data_number ++;
	if(pSlot->slot_data_number >= KV_DATA_DEPTH_WARNING)
	{
		KVLTRACE(60,1,"The data has exceeded the expected depth:%d pData:%x!", pSlot->slot_data_number, pData);
	}

	return _kv_slot_data_add(pSlot, pData);
}

KVData *
kv_slot_data_inq(KVSlot *pSlot, u8 *key_ptr, ub key_len)
{
	return _kv_slot_data_inq(pSlot, key_ptr, key_len);
}

KVData *
kv_slot_data_del(KVSlot **ppSlot, u8 *key_ptr, ub key_len)
{
	KVData *pData;
	KVSlot *pSlot = *ppSlot;

	pData = _kv_slot_data_inq(pSlot, key_ptr, key_len);
	if(pData != NULL)
	{
		if(pSlot->slot_data_number <= 0)
		{
			KVLTRACE(60,1,"Algorithm error, abnormal counter found:%d!", pSlot->slot_data_number);
		}
		else
		{
			pSlot->slot_data_number --;
		}

		_kv_slot_data_del(pSlot, pData);

		_kv_slot_rebuild_bloom_filter(pSlot);
	}

	_kv_slot_recover(ppSlot);

	return pData;
}

#endif

