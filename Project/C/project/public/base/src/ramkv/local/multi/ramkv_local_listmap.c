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
#include "ramkv_local_basemap.h"
#include "ramkv_log.h"

static inline void
_ramkv_local_listmap_del_head(KVLocalMultiMap *pMultiMap, KVLocalMultiBaseMap *pBaseMap)
{
	if(pMultiMap->head == pBaseMap)
	{
		if(pMultiMap->head->next_node != pBaseMap->next_node)
		{
			KVABNOR("find Arithmetic error! %x/%x", pMultiMap->head->next_node, pBaseMap->next_node);
		}

		if(pMultiMap->tail == pMultiMap->head)
		{
			if(pMultiMap->head->next_node != NULL)
			{
				KVABNOR("Arithmetic error! %lx/%lx", pMultiMap->head, pMultiMap->head->next_node);
			}
			pMultiMap->tail = pMultiMap->head = NULL;
		}
		else
		{
			pMultiMap->head = pMultiMap->head->next_node;
			if(pMultiMap->head == NULL)
			{
				pMultiMap->tail = NULL;
			}
			else
			{
				pMultiMap->head->up_node = NULL;				
			}
		}
	}
}

static inline void
_ramkv_local_listmap_del_tail(KVLocalMultiMap *pMultiMap, KVLocalMultiBaseMap *pBaseMap)
{
	if(pMultiMap->tail == pBaseMap)
	{
		if(pMultiMap->tail->up_node != pBaseMap->up_node)
		{
			KVABNOR("find Arithmetic error! %x/%x", pMultiMap->tail->up_node, pBaseMap->up_node);
		}
	
		if(pMultiMap->head == pMultiMap->tail)
		{
			if(pMultiMap->head->next_node != NULL)
			{
				KVABNOR("Arithmetic error! %lx/%lx", pMultiMap->head, pMultiMap->head->next_node);
			}
			pMultiMap->tail = pMultiMap->head = NULL;
		}
		else
		{
			pMultiMap->tail = pMultiMap->tail->up_node;
			if(pMultiMap->tail == NULL)
			{
				pMultiMap->head = NULL;
			}
			else
			{
				pMultiMap->tail->next_node = NULL;				
			}
		}
	}
}

static inline void
_ramkv_local_listmap_del_mid(KVLocalMultiMap *pMultiMap, KVLocalMultiBaseMap *pBaseMap)
{
	if((pMultiMap->head == NULL) || (pMultiMap->tail == NULL))
	{
		KVABNOR("find Arithmetic error! %x/%x/%x/%x",
			pMultiMap->head, pMultiMap->tail,
			pBaseMap->up_node, pBaseMap->next_node);
	}
	if((pBaseMap->up_node == NULL) || (pBaseMap->next_node == NULL))
	{
		KVABNOR("find Arithmetic error! %x/%x/%x/%x",
			pMultiMap->head, pMultiMap->tail,
			pBaseMap->up_node, pBaseMap->next_node);
	}

	if(pBaseMap->up_node != NULL)
	{
		((KVLocalMultiBaseMap *)(pBaseMap->up_node))->next_node = pBaseMap->next_node;
	}
	if(pBaseMap->next_node != NULL)
	{
		((KVLocalMultiBaseMap *)(pBaseMap->next_node))->up_node = pBaseMap->up_node;
	}
}

static inline ub
_ramkv_local_listmap_inq(KVLocalMultiMap *pMultiMap, sb index, void *value_ptr, ub value_len)
{
	KVLocalMultiBaseMap *pBaseMap;
	sb safe_counter;
	dave_bool fast_flag;

	KVDEBUG("index:%d/%d number:%d/%d",
		index, pMultiMap->current_index,
		pMultiMap->ramkv_number, pMultiMap->current_ramkv_number);

	if((pMultiMap == NULL) || (pMultiMap->ramkv_number == 0))
	{
		return 0;
	}

	if((index < 0) || (index >= pMultiMap->ramkv_number))
	{
		return 0;
	}

	if((pMultiMap->ramkv_number < 256)
		|| (pMultiMap->current_ramkv_number != pMultiMap->ramkv_number)
		|| (pMultiMap->current_index > index)
		|| (pMultiMap->current_pBaseMap == NULL))
	{
		pBaseMap = pMultiMap->head;
		safe_counter = 0;
		fast_flag = dave_false;
	}
	else
	{
		pBaseMap = pMultiMap->current_pBaseMap;
		safe_counter = pMultiMap->current_index;
		fast_flag = dave_true;
	}

	while((pBaseMap != NULL) && ((safe_counter ++) < index))
	{
		pBaseMap = (KVLocalMultiBaseMap *)(pBaseMap->next_node);
	}

	if(pBaseMap == NULL)
	{
		if(index < pMultiMap->ramkv_number)
		{
			KVLOG("find Arithmetic error! %d/%d %d/%d/%x fast_flag:%d",
				index, pMultiMap->ramkv_number,
				pMultiMap->current_index, pMultiMap->current_ramkv_number,
				pMultiMap->current_pBaseMap,
				fast_flag);

			pMultiMap->current_ramkv_number = 0;
			pMultiMap->current_index = 0;
			pMultiMap->current_pBaseMap = NULL;
		}
		return 0;
	}
	else
	{
		pMultiMap->current_ramkv_number = pMultiMap->ramkv_number;
		pMultiMap->current_index = index;
		pMultiMap->current_pBaseMap = pBaseMap;

		return ramkv_local_basemap_copy_value_to_user(pBaseMap, value_ptr, value_len);		
	}
}

// ====================================================================

void
ramkv_local_listmap_add(KVLocalMultiMap *pMultiMap, KVLocalMultiBaseMap *pBaseMap, s8 *fun, ub line)
{
	if((pMultiMap->head == pBaseMap) || (pMultiMap->tail == pBaseMap))
	{
		KVDEBUG("Repeated addition has occurred on %s:%d!", fun, line);
		return;
	}

	if((pBaseMap->up_node != NULL) || (pBaseMap->next_node != NULL))
	{
		KVABNOR("node(%x/%x) not NULL, why?", pBaseMap->up_node, pBaseMap->next_node);
		return;
	}

	if(pMultiMap->head == NULL)
	{
		pMultiMap->head = pMultiMap->tail = pBaseMap;
	}
	else
	{
		if((pBaseMap->up_node != NULL) || (pBaseMap->next_node != NULL))
		{
			KVABNOR("Arithmetic error! %x/%x/%x", pBaseMap, pBaseMap->up_node, pBaseMap->next_node);
		}
		pBaseMap->up_node = pMultiMap->tail;

		if(pMultiMap->tail->next_node != NULL)
		{
			KVABNOR("Arithmetic error! %x/%x", pMultiMap->tail, pMultiMap->tail->next_node);
		}
		pMultiMap->tail->next_node = pBaseMap;

		pMultiMap->tail = pBaseMap;
	}

	pMultiMap->ramkv_number ++;
}

ub
ramkv_local_listmap_inq(KVLocalMultiMap *pMultiMap, sb index, void *value_ptr, ub value_len)
{
	return _ramkv_local_listmap_inq(pMultiMap, index, value_ptr, value_len);
}

void
ramkv_local_listmap_del(KVLocalMultiMap *pMultiMap, KVLocalMultiBaseMap *pBaseMap)
{
	KVDEBUG("index:%d number:%d/%d",
		pMultiMap->current_index,
		pMultiMap->ramkv_number, pMultiMap->current_ramkv_number);

	if(pMultiMap->head == pBaseMap)
	{
		_ramkv_local_listmap_del_head(pMultiMap, pBaseMap);
	}
	else if(pMultiMap->tail == pBaseMap)
	{
		_ramkv_local_listmap_del_tail(pMultiMap, pBaseMap);
	}
	else
	{
		_ramkv_local_listmap_del_mid(pMultiMap, pBaseMap);
	}

	pMultiMap->current_ramkv_number = 0;
	pMultiMap->current_index = 0;
	pMultiMap->current_pBaseMap = NULL;

	pBaseMap->up_node = pBaseMap->next_node = NULL;

	if((-- pMultiMap->ramkv_number) < 0)
	{
		KVLOG("find Arithmetic error! %d", pMultiMap->ramkv_number);
		pMultiMap->ramkv_number = 0;
	}
}

#endif

