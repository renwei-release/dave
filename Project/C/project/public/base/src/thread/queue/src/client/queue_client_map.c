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

static void *_client_map_kv = NULL;
static void *_gid_map_kv = NULL;

static QueueClientMap *
_queue_client_map_malloc(s8 *thread_name)
{
	QueueClientMap *pMap = dave_ralloc(sizeof(QueueClientMap));

	dave_strcpy(pMap->thread_name, thread_name, sizeof(pMap->thread_name));
	pMap->queue_index = 0;
	pMap->queue_number = 0;

	pMap->run_req_counter = pMap->run_rsp_counter = 0;

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
	QueueClientMap *pMap = kv_del_key_ptr(_client_map_kv, key);

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
	QueueClientMap *pMap = kv_inq_key_ptr(_client_map_kv, thread_name);

	return pMap;
}

static QueueClientMap *
_queue_client_map_add(s8 *thread_name)
{
	QueueClientMap *pMap = _queue_client_map_inq(thread_name);

	if(pMap == NULL)
	{
		pMap = _queue_client_map_malloc(thread_name);

		kv_add_key_ptr(_client_map_kv, thread_name, pMap);
	}

	return pMap;
}

static void
_queue_client_map_del(s8 *thread_name)
{
	_queue_client_map_recycle(_client_map_kv, thread_name);
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
	_client_map_kv = kv_malloc("queue-client-map", 0, NULL);
	_gid_map_kv = kv_malloc("queue-gid-map", 0, NULL);
}

void
queue_client_map_exit(void)
{
	kv_free(_client_map_kv, _queue_client_map_recycle);
	kv_free(_gid_map_kv, NULL);
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
		pMap = kv_index_key_ptr(_client_map_kv, index);
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

void
queue_client_gid_map_add(s8 *thread_name, s8 *gid)
{
	QueueClientMap *pMap = _queue_client_map_inq(thread_name);

	if(pMap != NULL)
	{
		kv_add_key_ptr(_gid_map_kv, gid, pMap);
	}
}

void
queue_client_gid_map_del(s8 *gid)
{
	kv_del_key_ptr(_gid_map_kv, gid);
}

QueueClientMap *
queue_client_gid_map_inq(s8 *gid)
{
	return kv_inq_key_ptr(_gid_map_kv, gid);
}

ub
queue_client_map_info(s8 *info_ptr, ub info_len)
{
	ub info_index, index, map_index;
	QueueClientMap *pMap;

	info_index = 0;

	info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "QUEUE MAP INFO:\n");

	for(index=0; index<1024000; index++)
	{
		pMap = kv_index_key_ptr(_client_map_kv, index);
		if(pMap == NULL)
			break;

		info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
			" %s->",
			pMap->thread_name);
		for(map_index=0; map_index<QUEUE_CLIENT_MAP_MAX; map_index++)
		{
			if(pMap->queue_gid[map_index][0] == '\0')
				break;

			info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
				"%s ",
				pMap->queue_gid[map_index]);
		}
		info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "(%lu/%lu)\n",
			pMap->run_req_counter, pMap->run_rsp_counter);
	}

	return info_index;
}

#endif

