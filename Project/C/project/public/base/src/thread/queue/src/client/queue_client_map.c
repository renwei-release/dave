/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_queue.h"
#if defined(QUEUE_STACK_CLIENT)
#include "dave_tools.h"
#include "queue_client_map.h"
#include "queue_log.h"

static void *_map_kv = NULL;

static QueueClientMap *
_queue_client_map_malloc(s8 *thread_name)
{
	QueueClientMap *pMap = dave_ralloc(sizeof(QueueClientMap));

	dave_strcpy(pMap->thread_name, thread_name, sizeof(pMap->thread_name));
	pMap->queue_index = 0;
	pMap->queue_number = 0;

	t_lock_reset(&(pMap->pv));
	return pMap;
}

static void
_queue_client_map_free(QueueClientMap *pMap)
{
	dave_free(pMap);
}

static RetCode
_queue_client_map_recycle(void *ramkv, s8 *key)
{
	QueueClientMap *pMap = kv_del_key_ptr(_map_kv, key);

	if(pMap == NULL)
	{
		return RetCode_empty_data;
	}

	_queue_client_map_free(pMap);

	return RetCode_OK;
}

static QueueClientMap *
_queue_client_map_inq(s8 *thread_name)
{
	QueueClientMap *pMap = kv_inq_key_ptr(_map_kv, thread_name);

	return pMap;
}

static QueueClientMap *
_queue_client_map_add(s8 *thread_name)
{
	QueueClientMap *pMap = _queue_client_map_inq(thread_name);

	if(pMap == NULL)
	{
		pMap = _queue_client_map_malloc(thread_name);

		kv_add_key_ptr(_map_kv, thread_name, pMap);
	}

	return pMap;
}

static void
_queue_client_map_del(s8 *thread_name)
{
	_queue_client_map_recycle(_map_kv, thread_name);
}

static void
_queue_client_map_queue_add(QueueClientMap *pMap, s8 *queue_gid)
{
	ub index;

	for(index=0; index<QUEUE_CLIENT_MAP_MAX; index++)
	{
		if(pMap->queue_gid[index][0] == '\0')
		{
			if((++ pMap->queue_number) > QUEUE_CLIENT_MAP_MAX)
			{
				QUEUELOG("invalid queue_number:%d", pMap->queue_number);
				pMap->queue_number = QUEUE_CLIENT_MAP_MAX;
			}
			dave_strcpy(pMap->queue_gid[index], queue_gid, DAVE_GLOBALLY_IDENTIFIER_LEN);
			return;
		}
	
		if(dave_strcmp(pMap->queue_gid[index], queue_gid) == dave_true)
		{
			return;
		}
	}

	QUEUEABNOR("add %s to %s failed! limited resources of the map.",
		queue_gid, pMap->thread_name);
}

static void
_queue_client_map_queue_del(QueueClientMap *pMap, s8 *queue_gid)
{
	ub index, copy_index;

	for(index=0; index<QUEUE_CLIENT_MAP_MAX; index++)
	{
		if(pMap->queue_gid[index][0] == '\0')
			break;

		if(dave_strcmp(pMap->queue_gid[index], queue_gid) == dave_true)
		{
			pMap->queue_gid[index][0] = '\0';
		}
	}

	copy_index = 0;

	for(index=0; index<QUEUE_CLIENT_MAP_MAX; index++)
	{
		if(pMap->queue_gid[index][0] != '\0')
		{
			dave_strcpy(pMap->queue_gid[copy_index ++], pMap->queue_gid[index], DAVE_GLOBALLY_IDENTIFIER_LEN);
		}
	}
	pMap->queue_number = copy_index;
	if(pMap->queue_number > QUEUE_CLIENT_MAP_MAX)
	{
		QUEUELOG("invalid queue_number:%d", pMap->queue_number);
		pMap->queue_number = QUEUE_CLIENT_MAP_MAX;
	}
	for(copy_index=0; copy_index<QUEUE_CLIENT_MAP_MAX; copy_index++)
	{
		pMap->queue_gid[copy_index][0] = '\0';
	}
}

// =====================================================================

void
queue_client_map_init(void)
{
	_map_kv = kv_malloc("queue-client-map", 0, NULL);
}

void
queue_client_map_exit(void)
{
	kv_free(_map_kv, _queue_client_map_recycle);
}

void
queue_client_map_add(s8 *thread_name)
{
	QueueClientMap *pMap = _queue_client_map_add(thread_name);

	dave_strcpy(pMap->thread_name, thread_name, sizeof(pMap->thread_name));
}

void
queue_client_map_del(s8 *thread_name)
{
	_queue_client_map_del(thread_name);
}

void
queue_client_map_queue_add(QueueClientMap *pMap, s8 *queue_gid)
{
	SAFECODEv1(pMap->pv, {
		_queue_client_map_queue_add(pMap, queue_gid);
	});
}

void
queue_client_map_queue_del(QueueClientMap *pMap, s8 *queue_gid)
{
	SAFECODEv1(pMap->pv, {
		_queue_client_map_queue_del(pMap, queue_gid);
	});
}

void
queue_client_map_queue_del_all(s8 *queue_gid)
{
	ub index;
	QueueClientMap *pMap;

	for(index=0; index<1024000; index++)
	{
		pMap = kv_index_key_ptr(_map_kv, index);
		if(pMap == NULL)
			break;

		queue_client_map_queue_del(pMap, queue_gid);
	}
}

QueueClientMap *
queue_client_map_inq(s8 *thread_name)
{
	return _queue_client_map_inq(thread_name);
}

#endif

