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
#include "kv_list_slot.h"
#include "kv_list_data.h"
#include "kv_local_multimap.h"
#include "kv_log.h"

static void
_kv_struct_multimap_reset(KV *pKV)
{
	pKV->local.pMultiMap = kv_local_multimap_malloc(pKV);
}

static void
_kv_struct_multimap_clear(KV *pKV)
{
	if(pKV->local.pMultiMap != NULL)
	{
		kv_local_multimap_free(pKV);
	}
}

static void
_kv_struct_slot_reset(KVList *pKV)
{
	ub slot_index;

	for(slot_index=0; slot_index<KV_SLOT_NUM; slot_index++)
	{
		pKV->slot[slot_index] = NULL;
	}
}

static void
_kv_struct_slot_clear(KVList *pKV)
{
	ub slot_index;

	for(slot_index=0; slot_index<KV_SLOT_NUM; slot_index++)
	{
		if(pKV->slot[slot_index] != NULL)
		{
			kv_slot_free((KVSlot *)(pKV->slot[slot_index]));

			pKV->slot[slot_index] = NULL;
		}
	}
}

static void
_kv_struct_data_reset(KVList *pKV)
{
	pKV->list_data_head = NULL;
	pKV->list_data_tail = NULL;
}

static void
_kv_struct_data_clear(KVList *pKV)
{
	kv_list_data_free((KVData *)(pKV->list_data_head));
}

static void
_kv_malloc_local(KV *pKV)
{
	_kv_struct_multimap_reset(pKV);

	_kv_struct_slot_reset(&(pKV->local.kv_list));

	_kv_struct_data_reset(&(pKV->local.kv_list));
}

static void
_kv_free_local(KV *pKV)
{
	_kv_struct_multimap_clear(pKV);

	_kv_struct_slot_clear(&(pKV->local.kv_list));

	_kv_struct_data_clear(&(pKV->local.kv_list));
}

// ====================================================================

void
kv_malloc_local(KV *pKV)
{
	SAFEZONEv5W( pKV->kv_pv, _kv_malloc_local(pKV); );
}

void
kv_free_local(KV *pKV)
{
	SAFEZONEv5W( pKV->kv_pv, _kv_free_local(pKV); );
}

#endif

