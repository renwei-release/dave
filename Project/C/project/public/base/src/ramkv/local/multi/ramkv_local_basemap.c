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
#include "ramkv_local_multimap_param.h"
#include "ramkv_log.h"

static inline void
_ramkv_local_basemap_build_value_checksum(KVLocalMultiBaseMap *pBaseMap)
{
	pBaseMap->value_checksum = pBaseMap->value_len + ((ub)(pBaseMap->value_ptr));
}

static inline dave_bool
_ramkv_local_basemap_check_value_checksum(KVLocalMultiBaseMap *pBaseMap)
{
	if(pBaseMap->value_checksum != (pBaseMap->value_len + ((ub)(pBaseMap->value_ptr))))
	{
		return dave_false;
	}

	return dave_true;
}

static inline void
_ramkv_local_basemap_copy_value_from_user(KVLocalMultiBaseMap *pBaseMap, void *value_ptr, ub value_len)
{
	if((value_ptr == NULL) || (value_len == 0))
	{
		if(pBaseMap->value_len > sizeof(void *))
		{
			if(pBaseMap->value_ptr != NULL)
			{
				ramkvm_free(pBaseMap->value_ptr);
			}
		}

		pBaseMap->value_len = 0;
		pBaseMap->value_ptr = NULL;
	}
	else
	{
		if(value_len > sizeof(void *))
		{
			if(pBaseMap->value_len != value_len)
			{
				if(pBaseMap->value_ptr != NULL)
				{
					ramkvm_free(pBaseMap->value_ptr);
				}
				pBaseMap->value_len = value_len;
				pBaseMap->value_ptr = ramkvm_malloc(pBaseMap->value_len);
			}
			dave_memcpy(pBaseMap->value_ptr, value_ptr, pBaseMap->value_len);
		}
		else
		{
			pBaseMap->value_len = value_len;
			dave_memcpy(&(pBaseMap->value_ptr), value_ptr, pBaseMap->value_len);
			KVDEBUG("%lx", pBaseMap->value_ptr);
		}
	}

	_ramkv_local_basemap_build_value_checksum(pBaseMap);
}

static inline ub
_ramkv_local_basemap_copy_value_to_user(KVLocalMultiBaseMap *pBaseMap, void *value_ptr, ub value_len)
{
	if(pBaseMap->value_len == 0)
		return 0;

	if(_ramkv_local_basemap_check_value_checksum(pBaseMap) == dave_false)
	{
		KVLTRACE(60,1,"value check failed:%lx/%lx!", pBaseMap->value_ptr, pBaseMap->value_checksum);
	}

	if(value_len > pBaseMap->value_len)
	{
		value_len = pBaseMap->value_len;
	}

	if(pBaseMap->value_len > sizeof(void *))
	{
		dave_memcpy(value_ptr, pBaseMap->value_ptr, value_len);
	}
	else
	{
		KVDEBUG("%lx", pBaseMap->value_ptr);
		dave_memcpy(value_ptr, &(pBaseMap->value_ptr), value_len);
	}

	return value_len;
}

static inline void
_ramkv_local_basemap_clean_value(KVLocalMultiBaseMap *pBaseMap)
{
	if(pBaseMap->value_len > sizeof(void *))
	{
		if(pBaseMap->value_ptr != NULL)
		{
			ramkvm_free(pBaseMap->value_ptr);
		}
	}
	pBaseMap->value_len = 0;
	pBaseMap->value_ptr = NULL;
}

static inline KVLocalMultiBaseMap *
_ramkv_local_basemap_malloc(void *value_ptr, ub value_len)
{
	KVLocalMultiBaseMap *pBaseMap = ramkvm_malloc(sizeof(KVLocalMultiBaseMap));
	ub map_index;

	pBaseMap->value_len = 0;
	pBaseMap->value_ptr = NULL;
	pBaseMap->value_checksum = 0;

	_ramkv_local_basemap_copy_value_from_user(pBaseMap, value_ptr, value_len);

	for(map_index=0; map_index<BASE_MAP_MAX; map_index++)
	{
		pBaseMap->base_map[map_index] = NULL;
	}

	pBaseMap->up_node = pBaseMap->next_node = NULL;

	return pBaseMap;
}

static inline void
_ramkv_local_basemap_free(KVLocalMultiBaseMap *pBaseMap)
{
	ub map_index;

	if(pBaseMap == NULL)
	{
		return;
	}

	for(map_index=0; map_index<BASE_MAP_MAX; map_index++)
	{
		_ramkv_local_basemap_clean_value(pBaseMap);

		if(pBaseMap->base_map[map_index] != NULL)
		{
			_ramkv_local_basemap_free((KVLocalMultiBaseMap *)(pBaseMap->base_map[map_index]));
			pBaseMap->base_map[map_index] = NULL;
		}

		pBaseMap->up_node = pBaseMap->next_node = NULL;
	}

	ramkvm_free(pBaseMap);
}

static inline dave_bool
_ramkv_local_basemap_empty(KVLocalMultiBaseMap *pBaseMap)
{
	ub map_index;

	for(map_index=0; map_index<BASE_MAP_MAX; map_index++)
	{
		if(pBaseMap->base_map[map_index] != NULL)
			return dave_false;
	}

	return dave_true;
}

static inline KVLocalMultiBaseMap *
_ramkv_local_basemap_inq(KVLocalMultiBaseMap **ppBaseMap, u8 *key_ptr, ub key_len, dave_bool inq_for_add)
{
	ub key_index, map_index;

	key_index = 0;

	while(key_index < key_len)
	{
		map_index = key_ptr[key_index];

		if(ppBaseMap[map_index] == NULL)
		{
			if(inq_for_add == dave_false)
			{
				return NULL;
			}

			ppBaseMap[map_index] = _ramkv_local_basemap_malloc(NULL, 0);
		}

		if((++ key_index) >= key_len)
		{
			return (KVLocalMultiBaseMap *)(ppBaseMap[map_index]);
		}

		ppBaseMap = (KVLocalMultiBaseMap **)(ppBaseMap[map_index]->base_map);
	}

	return NULL;
}

static inline KVLocalMultiBaseMap *
_ramkv_local_basemap_add(KVLocalMultiBaseMap **ppBaseMap, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len)
{
	ub key_index, map_index;

	key_index = 0;

	while(key_index < key_len)
	{
		map_index = key_ptr[key_index];

		/*
		 * 建立中继索引节点。
		 */
		if(ppBaseMap[map_index] == NULL)
		{
			ppBaseMap[map_index] = _ramkv_local_basemap_malloc(NULL, 0);
		}		

		/*
		 * 该节点是否已经是最后一个索引节点？
		 */
		if((++ key_index) >= key_len)
		{
			_ramkv_local_basemap_copy_value_from_user(ppBaseMap[map_index], value_ptr, value_len);

			return ppBaseMap[map_index];
		}

		ppBaseMap = (KVLocalMultiBaseMap **)(ppBaseMap[map_index]->base_map);
	}

	return NULL;
}

static inline KVLocalMultiBaseMap *
_ramkv_local_basemap_del(KVLocalMultiBaseMap **ppBaseMap, u8 *key_ptr, ub key_len)
{
	KVLocalMultiBaseMap *pBaseMap;
	ub key_index, map_index;

	key_index = 0;

	while(key_index < key_len)
	{
		map_index = key_ptr[key_index];
		if(ppBaseMap[map_index] == NULL)
		{
			return NULL;
		}

		if((++ key_index) >= key_len)
		{
			pBaseMap = ppBaseMap[map_index];
			if(_ramkv_local_basemap_empty(pBaseMap) == dave_true)
			{
				/*
				 * 如果这个基础节点没有下级指向了，
				 * 那么需要清理掉这个基础节点，
				 * 等待ramkv_local_basemap_clean调用后释放空间。
				 * 同时在ramkv_local_basemap_clean里面也需要判断是否为空，
				 * 是空才释放空间。
				 */
				ppBaseMap[map_index] = NULL;
			}

			return pBaseMap;
		}

		ppBaseMap = (KVLocalMultiBaseMap **)(ppBaseMap[map_index]->base_map);
	}

	return NULL;
}

// ====================================================================

KVLocalMultiBaseMap *
ramkv_local_basemap_malloc(void *value_ptr, ub value_len)
{
	return _ramkv_local_basemap_malloc(value_ptr, value_len);
}

void
ramkv_local_basemap_free(KVLocalMultiBaseMap *pBaseMap)
{
	_ramkv_local_basemap_free(pBaseMap);
}

KVLocalMultiBaseMap *
ramkv_local_basemap_inq(KVLocalMultiBaseMap **ppBaseMap, u8 *key_ptr, ub key_len, dave_bool inq_for_add)
{
	if(ppBaseMap == NULL)
	{
		return NULL;
	}

	return _ramkv_local_basemap_inq(ppBaseMap, key_ptr, key_len, inq_for_add);
}

KVLocalMultiBaseMap *
ramkv_local_basemap_add(KVLocalMultiBaseMap **ppBaseMap, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len)
{
	return _ramkv_local_basemap_add(ppBaseMap, key_ptr, key_len, value_ptr, value_len);
}

KVLocalMultiBaseMap *
ramkv_local_basemap_del(KVLocalMultiBaseMap **ppBaseMap, u8 *key_ptr, ub key_len)
{
	return _ramkv_local_basemap_del(ppBaseMap, key_ptr, key_len);
}

void
ramkv_local_basemap_clean(KVLocalMultiBaseMap *pBaseMap)
{
	_ramkv_local_basemap_clean_value(pBaseMap);

	if(_ramkv_local_basemap_empty(pBaseMap) == dave_true)
	{
		ramkvm_free(pBaseMap);
	}
}

void
ramkv_local_basemap_copy_value_from_user(KVLocalMultiBaseMap *pBaseMap, void *value_ptr, ub value_len)
{
	_ramkv_local_basemap_copy_value_from_user(pBaseMap, value_ptr, value_len);
}

ub
ramkv_local_basemap_copy_value_to_user(KVLocalMultiBaseMap *pBaseMap, void *value_ptr, ub value_len)
{
	return _ramkv_local_basemap_copy_value_to_user(pBaseMap, value_ptr, value_len);
}

#endif

