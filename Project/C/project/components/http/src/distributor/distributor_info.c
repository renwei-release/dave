/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "distributor_info.h"
#include "http_log.h"

#define AUTO_CLOSE_BASE_TIME 10

static void *_listen_kv = NULL;
static void *_listen_auto_close_kv = NULL;

static HttpDistributorInfo *
_distributor_info_malloc(ThreadId src, s8 *path, ub listening_seconds_time)
{
	HttpDistributorInfo *pInfo = dave_malloc(sizeof(HttpDistributorInfo));

	dave_strcpy(pInfo->thread_name, thread_name(src), sizeof(pInfo->thread_name));
	dave_strcpy(pInfo->path, path, sizeof(pInfo->path));
	pInfo->listening_seconds_life = 0;
	pInfo->listening_seconds_time = listening_seconds_time;

	t_lock_reset(&(pInfo->pv));
	pInfo->receive_counter = 0;

	return pInfo;
}

static void
_distributor_info_free(HttpDistributorInfo *pInfo)
{
	t_lock_destroy(&(pInfo->pv));

	dave_free(pInfo);
}

static RetCode
_distributor_info_clean(void *ramkv, s8 *key)
{
	HttpDistributorInfo *pInfo;

	pInfo = kv_del_key_ptr(ramkv, key);
	if(pInfo == NULL)
	{
		return RetCode_empty_data;
	}

	_distributor_info_free(pInfo);

	return RetCode_OK;
}

static void
_distributor_info_auto_close_timer(void *ramkv, s8 *key)
{
	HttpDistributorInfo *pInfo;

	pInfo = kv_inq_key_ptr(ramkv, key);
	if(pInfo == NULL)
	{
		return;
	}

	if((pInfo->listening_seconds_life + AUTO_CLOSE_BASE_TIME) >= pInfo->listening_seconds_time)
	{
		HTTPLOG("auto close thread:%s path:%s time:%d/%d receive:%d",
			pInfo->thread_name, pInfo->path,
			pInfo->listening_seconds_life, pInfo->listening_seconds_time,
			pInfo->receive_counter);

		_distributor_info_clean(ramkv, key);
	}
}

static ub
_distributor_info_info(void *ramkv, s8 *info_ptr, ub info_len)
{
	ub info_index = 0, index = 0;
	HttpDistributorInfo *pInfo;

	for(index=0; index<102400; index++)
	{
		pInfo = kv_index_key_ptr(ramkv, index);
		if(pInfo == NULL)
			break;

		info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
			" thread:%s path:%s time:%d/%d receive_counter:%d\n",
			pInfo->thread_name, pInfo->path,
			pInfo->listening_seconds_life, pInfo->listening_seconds_time,
			pInfo->receive_counter);
	}

	return info_index;
}

// =====================================================================

void
distributor_info_init(void)
{
	_listen_kv = kv_malloc("http-distributor", KvAttrib_list, 0, NULL);
	_listen_auto_close_kv = kv_malloc("http-distributor-auto", KvAttrib_list, AUTO_CLOSE_BASE_TIME, _distributor_info_auto_close_timer);
}

void
distributor_info_exit(void)
{
	kv_free(_listen_kv, _distributor_info_clean); _listen_kv = NULL;
	kv_free(_listen_auto_close_kv, _distributor_info_clean); _listen_auto_close_kv = NULL;
}

dave_bool
distributor_info_malloc(ThreadId src, s8 *path, ub listening_seconds_time)
{
	HttpDistributorInfo *pInfo;
	dave_bool ret;

	pInfo = _distributor_info_malloc(src, path, listening_seconds_time);
	if(pInfo == NULL)
	{
		HTTPABNOR("%s listen:%s time:%d malloc failed!",
			thread_name(src), path, listening_seconds_time);
		return dave_false;
	}

	if(listening_seconds_time == 0)
		ret = kv_add_key_ptr(_listen_kv, pInfo->path, pInfo);
	else
		ret = kv_add_key_ptr(_listen_auto_close_kv, pInfo->path, pInfo);

	if(ret == dave_false)
	{
		HTTPABNOR("%s listen:%s time:%d add kv failed!",
			thread_name(src), path, listening_seconds_time);
	}

	return ret;
}

void
distributor_info_free(s8 *path)
{
	HttpDistributorInfo *pInfo;

	pInfo = kv_del_key_ptr(_listen_kv, path);
	if(pInfo == NULL)
	{
		pInfo = kv_del_key_ptr(_listen_auto_close_kv, path);
	}
	if(pInfo == NULL)
	{
		return;
	}

	_distributor_info_free(pInfo);
}

HttpDistributorInfo *
distributor_info_inq(s8 *path)
{
	HttpDistributorInfo *pInfo;

	pInfo = (HttpDistributorInfo *)kv_inq_key_ptr(_listen_kv, path);
	if(pInfo == NULL)
	{
		pInfo = (HttpDistributorInfo *)kv_inq_key_ptr(_listen_auto_close_kv, path);
	}

	return pInfo;
}

ub
distributor_info_info(s8 *info_ptr, ub info_len)
{
	ub info_index = 0;

	info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "listen path:\n");
	info_index += _distributor_info_info(_listen_kv, &info_ptr[info_index], info_len-info_index);
	info_index += _distributor_info_info(_listen_auto_close_kv, &info_ptr[info_index], info_len-info_index);

	return info_index;
}

