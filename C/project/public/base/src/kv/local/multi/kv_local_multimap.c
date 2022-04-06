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
#include "kv_local_multimap_param.h"
#include "kv_local_basemap.h"
#include "kv_local_listmap.h"
#include "kv_log.h"

static inline KVLocalMultiMap *
_kv_local_multimap_malloc(void)
{
	KVLocalMultiMap *pMultiMap = kvm_malloc(sizeof(KVLocalMultiMap));
	ub map_index;

	for(map_index=0; map_index<BASE_MAP_MAX; map_index++)
	{
		pMultiMap->base_map[map_index] = NULL;
	}

	pMultiMap->kv_number = 0;
	pMultiMap->head = pMultiMap->tail = NULL;

	return pMultiMap;
}

static inline void
_kv_local_multimap_free(KVLocalMultiMap *pMultiMap)
{
	ub map_index;

	if(pMultiMap == NULL)
	{
		KVABNOR("pMultiMap is empty!");
		return;
	}

	for(map_index=0; map_index<BASE_MAP_MAX; map_index++)
	{
		if(pMultiMap->base_map[map_index] != NULL)
		{
			kv_local_basemap_free(pMultiMap->base_map[map_index]);
			pMultiMap->base_map[map_index] = NULL;
		}
	}

	pMultiMap->kv_number = 0;
	pMultiMap->head = pMultiMap->tail = NULL;

	pMultiMap->current_kv_number = 0;
	pMultiMap->current_index = 0;
	pMultiMap->current_pBaseMap = NULL;

	kvm_free(pMultiMap);
}

static inline dave_bool
_kv_local_multimap_add(KVLocalMultiMap *pMultiMap, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line)
{
	KVLocalMultiBaseMap *pBaseMap;

	pBaseMap = kv_local_basemap_inq((KVLocalMultiBaseMap **)(pMultiMap->base_map), key_ptr, key_len, dave_true);
	if(pBaseMap != NULL)
	{
		KVDEBUG("%s : %s", key_ptr, value_ptr);

		if((pBaseMap->up_node == NULL) && (pBaseMap->next_node == NULL))
		{
			kv_local_listmap_add(pMultiMap, pBaseMap, fun, line);
		}

		kv_local_basemap_copy_value_from_user(pBaseMap, value_ptr, value_len);
	}
	else
	{
		/*
		 * 查找的时候会直接创建新节点，
		 * 这里都无需调用
		 * kv_local_basemap_add
		 */
		KVDEBUG("Maybe need to call kv_local_basemap_add function!");
	}

	return dave_true;
}

static inline ub
_kv_local_multimap_inq(KVLocalMultiMap *pMultiMap, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len)
{
	KVLocalMultiBaseMap *pBaseMap;

	if(pMultiMap == NULL)
	{
		return 0;
	}

	pBaseMap = kv_local_basemap_inq((KVLocalMultiBaseMap **)(pMultiMap->base_map), key_ptr, key_len, dave_false);
	if(pBaseMap == NULL)
	{
		KVDEBUG("key:%s is NULL!", key_ptr);
		return 0;
	}

	if((pBaseMap->value_ptr == NULL) || (pBaseMap->value_len == 0))
	{
		KVDEBUG("key:%s value is NULL!", key_ptr);
		return 0;
	}

	KVDEBUG("%s", key_ptr);

	return kv_local_basemap_copy_value_to_user(pBaseMap, value_ptr, value_len);
}

static inline ub
_kv_local_multimap_del(KVLocalMultiMap *pMultiMap, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len)
{
	KVLocalMultiBaseMap *pBaseMap;

	pBaseMap = kv_local_basemap_del(pMultiMap->base_map, key_ptr, key_len);
	if(pBaseMap == NULL)
		return 0;

	if((pBaseMap->value_ptr == NULL) || (pBaseMap->value_len == 0))
	{
		// 一个空的中继节点。
		return 0;
	}

	value_len = kv_local_basemap_copy_value_to_user(pBaseMap, value_ptr, value_len);

	kv_local_listmap_del(pMultiMap, pBaseMap);

	kv_local_basemap_clean(pBaseMap);

	return value_len;
}

// ====================================================================

void *
kv_local_multimap_malloc(KV *pKV)
{
	if(pKV->local.pMultiMap == NULL)
	{
		pKV->local.pMultiMap = (void *)_kv_local_multimap_malloc();
	}
	else
	{
		KVLOG("repeat malloc:%x!", pKV->local.pMultiMap);
	}

	return pKV->local.pMultiMap;
}

void
kv_local_multimap_free(KV *pKV)
{
	KVLocalMultiMap *pMultiMap = (KVLocalMultiMap *)(pKV->local.pMultiMap);

	_kv_local_multimap_free(pMultiMap);
}

dave_bool
kv_local_multimap_add(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line)
{
	dave_bool ret = dave_false;

	KVDEBUG("key:%s value_ptr:%lx value_len:%d", key_ptr, *((ub *)value_ptr), value_len);

	if(key_len >= KV_KEY_MAX)
	{
		KVABNOR("The index KEY(%d/%d) should not be too long to avoid deep recursive calls!",
			key_len, KV_KEY_MAX);
		return dave_false;
	}

	SAFEZONEv5W(pKV->kv_pv, {

		ret = _kv_local_multimap_add(pKV->local.pMultiMap, key_ptr, key_len, value_ptr, value_len, fun, line);

	} );

	return ret;
}

ub
kv_local_multimap_inq(KV *pKV, sb index, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len)
{
	ub inq_value_len = 0;

	SAFEZONEv5R(pKV->kv_pv, {

		if((index < 0) && (key_ptr != NULL) && (key_len > 0))
		{
			inq_value_len = _kv_local_multimap_inq(pKV->local.pMultiMap, key_ptr, key_len, value_ptr, value_len);
		}
		else if(index >= 0)
		{
			inq_value_len = kv_local_listmap_inq(pKV->local.pMultiMap, index, value_ptr, value_len);
		}
		else if(key_ptr == NULL)
		{
			inq_value_len = kv_local_listmap_inq(pKV->local.pMultiMap, 0, value_ptr, value_len);
		}

	} );

	KVDEBUG("key:%s value_ptr:%lx value_len:%d", key_ptr, *((ub *)value_ptr), inq_value_len);

	return inq_value_len;
}

ub
kv_local_multimap_del(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len)
{
	ub ret = 0;

	KVDEBUG("key:%s", key_ptr);

	SAFEZONEv5W(pKV->kv_pv, {

		ret = _kv_local_multimap_del(pKV->local.pMultiMap, key_ptr, key_len, value_ptr, value_len);

	} );

	return ret;
}

#endif

