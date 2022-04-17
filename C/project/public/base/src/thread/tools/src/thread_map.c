/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "dave_tools.h"
#include "thread_tools.h"
#include "thread_struct.h"
#include "thread_quit.h"
#include "thread_parameter.h"
#include "thread_map.h"
#include "thread_log.h"

#define NAME_THREAD_MAP_MAX (THREAD_MAX)

typedef struct {
	s8 thread_name[THREAD_NAME_MAX];
	ThreadStruct *pThread;
} NameThreadMap;

static TLock _thread_map_pv;
static NameThreadMap _name_thread_map[NAME_THREAD_MAP_MAX];

static void
_thread_map_name_reset(NameThreadMap *pMap)
{
	dave_memset(pMap, 0x00, sizeof(NameThreadMap));

	pMap->thread_name[0] = '\0';
	pMap->pThread = NULL;
}

static void
_thread_map_name_all_reset(void)
{
	ub map_index;

	for(map_index=0; map_index<NAME_THREAD_MAP_MAX; map_index++)
	{
		_thread_map_name_reset(&_name_thread_map[map_index]);
	}
}

static NameThreadMap *
_thread_map_name_find(s8 *thread_name, ThreadStruct *pThread, dave_bool find_new)
{
	ub map_index, name_index, safe_counter;
	NameThreadMap *pMap;

	for(map_index=0,name_index=0; name_index<THREAD_NAME_MAX; name_index++)
	{
		if(thread_name[name_index] == '\0')
			break;

		map_index *= 10;

		map_index += (ub)(thread_name[name_index]);
	}

	map_index = map_index % NAME_THREAD_MAP_MAX;

	if(find_new == dave_true)
	{
		pMap = NULL;

		for(safe_counter=0; safe_counter<NAME_THREAD_MAP_MAX; safe_counter++)
		{
			if(map_index >= NAME_THREAD_MAP_MAX)
			{
				map_index = 0;
			}

			if((_name_thread_map[map_index].thread_name[0] == '\0')
				|| ((_name_thread_map[map_index].pThread != NULL) && (dave_strcmp(_name_thread_map[map_index].thread_name, thread_name) == dave_true)))
			{
				pMap = &_name_thread_map[map_index];

				dave_strcpy(pMap->thread_name, thread_name, THREAD_NAME_MAX);
				pMap->pThread = pThread;
				break;
			}

			map_index ++;
		}

		return pMap;
	}
	else
	{
		for(safe_counter=0; safe_counter<NAME_THREAD_MAP_MAX; safe_counter++)
		{
			if(map_index >= NAME_THREAD_MAP_MAX)
				map_index = 0;

			if((_name_thread_map[map_index].pThread != NULL) && (dave_strcmp(_name_thread_map[map_index].thread_name, thread_name) == dave_true))
			{
				return &_name_thread_map[map_index];
			}

			map_index ++;
		}
	}

	return NULL;
}

// =====================================================================

void
thread_map_init(void)
{
	t_lock_reset(&_thread_map_pv);

	_thread_map_name_all_reset();
}

void
thread_map_exit(void)
{

}

dave_bool
__thread_map_name_add__(s8 *thread_name, ThreadStruct *pThread, s8 *fun, ub line)
{
	NameThreadMap *pMap;
	dave_bool ret = dave_true;;

	if((thread_name == NULL) || (pThread == NULL))
	{
		THREADABNOR("invalid param:%s/%x <%s:%d>", thread_name, pThread, fun, line);
		return dave_false;
	}

	SAFECODEv1(_thread_map_pv, {
		pMap = _thread_map_name_find(thread_name, pThread, dave_true);
		if(pMap == NULL)
		{
			THREADABNOR("can't find the thread:%s map!", thread_name);
			ret = dave_false;
		}
	} );

	return ret;
}

void
__thread_map_name_del__(s8 *thread_name, s8 *fun, ub line)
{
	NameThreadMap *pMap;

	if(thread_name == NULL)
	{
		THREADABNOR("invalid param:%s <%s:%d>", thread_name, fun, line);
		return;
	}

	SAFECODEv1(_thread_map_pv, {
		pMap = _thread_map_name_find(thread_name, NULL, dave_false);
		if(pMap != NULL)
		{
			_thread_map_name_reset(pMap);
		}
	} );
}

ThreadStruct *
__thread_map_name__(s8 *thread_name, s8 *fun, ub line)
{
	NameThreadMap *pMap;

	if((thread_name == NULL) || (thread_name[0] == '\0'))
	{
		THREADLOG("invalid param:%s <%s:%d>", thread_name, fun, line);
		return NULL;
	}

	pMap = _thread_map_name_find(thread_name, NULL, dave_false);
	if(pMap != NULL)
	{
		return pMap->pThread;
	}

	THREADDEBUG("can't find the thread:%s map!", thread_name);

	return NULL;
}

ThreadId
__thread_map_id__(s8 *thread_name, s8 *fun, ub line)
{
	ThreadStruct *pThread;

	pThread = __thread_map_name__(thread_name, fun, line);
	if(pThread == NULL)
		return INVALID_THREAD_ID;

	return pThread->thread_id;
}

#endif

