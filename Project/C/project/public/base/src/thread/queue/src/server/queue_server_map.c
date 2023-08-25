/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_queue.h"
#if defined(QUEUE_STACK_SERVER)
#include "dave_base.h"
#include "thread_struct.h"
#include "thread_tools.h"
#include "queue_server_map.h"
#include "queue_log.h"

static void *_map_kv = NULL;
static TLock _map_pv;

static QueueServerMap *
_queue_server_map_malloc(s8 *thread_name)
{
	QueueServerMap *pMap = dave_ralloc(sizeof(QueueServerMap));

	dave_strcpy(pMap->thread_name, thread_name, sizeof(pMap->thread_name));

	pMap->client_index = pMap->client_number = 0;

	return pMap;
}

static void
_queue_server_map_free(QueueServerMap *pMap)
{
	dave_free(pMap);
}

static QueueServerMap *
_queue_server_map_inq(s8 *thread_name)
{
	QueueServerMap *pMap = kv_inq_key_ptr(_map_kv, thread_name);

	if(pMap == NULL)
	{
		pMap = _queue_server_map_malloc(thread_name);

		kv_add_key_ptr(_map_kv, thread_name, pMap);
	}

	return pMap;
}

static void
_queue_server_map_add(QueueServerMap *pMap, s8 *gid)
{
	ub index;

	for(index=0; index<QUEUE_SERVER_MAP_MAX; index++)
	{
		if(pMap->client_gid[index][0] == '\0')
		{
			dave_strcpy(pMap->client_gid[index], gid, DAVE_GLOBALLY_IDENTIFIER_LEN);
			if((++ pMap->client_number) > QUEUE_SERVER_MAP_MAX)
			{
				QUEUELOG("invalid client_number:%d", pMap->client_number);
				pMap->client_number = QUEUE_SERVER_MAP_MAX;
			}
			return;
		}

		if(dave_strcmp(pMap->client_gid[index], gid) == dave_true)
		{
			return;
		}
	}

	QUEUEABNOR("add %s to %s failed! limited resources of the map.",
		gid, pMap->thread_name);
}

static void
_queue_server_map_del(QueueServerMap *pMap, s8 *gid)
{
	ub index, copy_index;

	for(index=0; index<QUEUE_SERVER_MAP_MAX; index++)
	{
		if(pMap->client_gid[index][0] == '\0')
			break;

		if(dave_strcmp(pMap->client_gid[index], gid) == dave_true)
		{
			pMap->client_gid[index][0] = '\0';
		}
	}

	copy_index = 0;

	for(index=0; index<QUEUE_SERVER_MAP_MAX; index++)
	{
		if(pMap->client_gid[index][0] != '\0')
		{
			dave_strcpy(pMap->client_gid[copy_index ++], pMap->client_gid[index], DAVE_GLOBALLY_IDENTIFIER_LEN);
		}
	}
	pMap->client_number = copy_index;
	if(pMap->client_number > QUEUE_SERVER_MAP_MAX)
	{
		QUEUELOG("invalid client_number:%d", pMap->client_number);
		pMap->client_number = QUEUE_SERVER_MAP_MAX;
	}
	for(copy_index=0; copy_index<QUEUE_SERVER_MAP_MAX; copy_index++)
	{
		pMap->client_gid[copy_index][0] = '\0';
	}
}

static RetCode
_queue_server_map_recycle(void *ramkv, s8 *key)
{
	QueueServerMap *pMap = kv_del_key_ptr(_map_kv, key);

	if(pMap == NULL)
	{
		return RetCode_empty_data;
	}

	_queue_server_map_free(pMap);

	return RetCode_OK;
}

// =====================================================================

void
queue_server_map_init(void)
{
	_map_kv = kv_malloc("queue-server-map", 0, NULL);
	t_lock_reset(&_map_pv);
}

void
queue_server_map_exit(void)
{
	kv_free(_map_kv, _queue_server_map_recycle);
}

void
queue_server_map_add(s8 *thread_name, s8 *gid)
{
	QueueServerMap *pMap = _queue_server_map_inq(thread_name);

	SAFECODEv1(_map_pv, {
		_queue_server_map_add(pMap, gid);
	});

	QUEUETRACE("thread:%s gid:%s", thread_name, gid);
}

void
queue_server_map_del(s8 *thread_name, s8 *gid)
{
	QueueServerMap *pMap = _queue_server_map_inq(thread_name);

	QUEUETRACE("thread:%s gid:%s", thread_name, gid);

	SAFECODEv1(_map_pv, {
		_queue_server_map_del(pMap, gid);
	});
}

QueueServerMap *
queue_server_map_inq(s8 *thread_name)
{
	return _queue_server_map_inq(thread_name);
}

ub
queue_server_map_info(s8 *info_ptr, ub info_len)
{
	ub info_index, index, map_index;
	QueueServerMap *pMap;

	info_index = 0;

	info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "QUEUE MAP INFO:\n");

	for(index=0; index<1024000; index++)
	{
		pMap = kv_index_key_ptr(_map_kv, index);
		if(pMap == NULL)
			break;

		info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
			" %s->",
			pMap->thread_name);
		for(map_index=0; map_index<QUEUE_SERVER_MAP_MAX; map_index++)
		{
			if(pMap->client_gid[map_index][0] == '\0')
				break;

			info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
				"%s ",
				pMap->client_gid[map_index]);
		}
		info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "\n");
	}

	return info_index;
}

#endif

