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
#include "ramkv_list_slot.h"
#include "ramkv_list_data.h"
#include "ramkv_local_multimap.h"
#include "ramkv_log.h"

static void
_ramkv_struct_multimap_reset(KV *pKV)
{
	pKV->local.pMultiMap = ramkv_local_multimap_malloc(pKV);
}

static void
_ramkv_struct_multimap_clear(KV *pKV)
{
	if(pKV->local.pMultiMap != NULL)
	{
		ramkv_local_multimap_free(pKV);
	}
}

static void
_ramkv_struct_slot_reset(KVList *pKV)
{
	ub slot_index;

	for(slot_index=0; slot_index<KV_SLOT_NUM; slot_index++)
	{
		pKV->slot[slot_index] = NULL;
	}
}

static void
_ramkv_struct_slot_clear(KVList *pKV)
{
	ub slot_index;

	for(slot_index=0; slot_index<KV_SLOT_NUM; slot_index++)
	{
		if(pKV->slot[slot_index] != NULL)
		{
			ramkv_slot_free((KVSlot *)(pKV->slot[slot_index]));

			pKV->slot[slot_index] = NULL;
		}
	}
}

static void
_ramkv_struct_data_reset(KVList *pKV)
{
	pKV->list_data_head = NULL;
	pKV->list_data_tail = NULL;
}

static void
_ramkv_struct_data_clear(KVList *pKV)
{
	ramkv_list_data_free((KVData *)(pKV->list_data_head));
}

static void
_ramkv_malloc_local(KV *pKV)
{
	_ramkv_struct_multimap_reset(pKV);

	_ramkv_struct_slot_reset(&(pKV->local.ramkv_list));

	_ramkv_struct_data_reset(&(pKV->local.ramkv_list));
}

static void
_ramkv_free_local(KV *pKV)
{
	_ramkv_struct_multimap_clear(pKV);

	_ramkv_struct_slot_clear(&(pKV->local.ramkv_list));

	_ramkv_struct_data_clear(&(pKV->local.ramkv_list));
}

// ====================================================================

void
ramkv_malloc_local(KV *pKV)
{
	SAFECODEv2W( pKV->ramkv_pv, _ramkv_malloc_local(pKV); );
}

void
ramkv_free_local(KV *pKV)
{
	SAFECODEv2W( pKV->ramkv_pv, _ramkv_free_local(pKV); );
}

#endif

