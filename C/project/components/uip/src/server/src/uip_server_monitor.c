/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "uip_tools.h"
#include "uip_server_monitor.h"
#include "uip_parsing.h"
#include "uip_log.h"

#define THE_MY_MAGIC_DATA 0x9988aaeee
#define CFG_UIP_SERVER_MONITOR_TIMER (s8 *)"UIPServerMonitorTimer"
#define MONITOR_KV_DEFAULT_TIME (360)
#define MONITOR_KV_MIN_TIME (30)
#define MONITOR_CHECK_DEFAULT_CONSUMING_TIME (9 * 1000 * 1000)

typedef struct {
	ub magic_data;

	UIPStack *pStack;

	ub malloc_time;
} UIPMonitor;

static void *_uip_monitor_kv = NULL;
static ub _uip_monitor_check_time_consuming = MONITOR_CHECK_DEFAULT_CONSUMING_TIME;
static uip_monitor_error _uip_monitor_error_fun = NULL;

static inline UIPMonitor *
_uip_server_monitor_malloc(UIPStack *pStack)
{
	UIPMonitor *pMonitor = dave_malloc(sizeof(UIPMonitor));

	pMonitor->magic_data = THE_MY_MAGIC_DATA;

	pMonitor->pStack = pStack;

	pMonitor->malloc_time = dave_os_time_us();

	base_ramkv_add_ub_ptr(_uip_monitor_kv, (ub)(pMonitor), pMonitor);

	return pMonitor;
}

static inline UIPStack *
_uip_server_monitor_free(UIPMonitor *pMonitor)
{
	UIPStack *pStack;
	UIPMonitor *pKVMonitor;
	ub time_consuming;

	if(pMonitor->magic_data != THE_MY_MAGIC_DATA)
	{
		UIPABNOR("invalid magic_data:%x", pMonitor->magic_data);
		return NULL;
	}

	pKVMonitor = base_ramkv_del_ub_ptr(_uip_monitor_kv, (ub)(pMonitor));
	if(pKVMonitor != pMonitor)
	{
		UIPABNOR("find kv store ptr mismatch:%x/%x maybe timer out!", pKVMonitor, pMonitor);
		if(pKVMonitor == NULL)
			return NULL;
		dave_free(pKVMonitor);
	}

	time_consuming = dave_os_time_us() - pMonitor->malloc_time;

	if(time_consuming > _uip_monitor_check_time_consuming)
	{
		UIPLOG("It takes too long to get a response, %x/%x %s method:%s channel:%s time:%lds!",
			pMonitor, pMonitor->pStack,
			thread_name(pMonitor->pStack->src),
			pMonitor->pStack->head.method,
			pMonitor->pStack->head.channel,
			time_consuming/1000000);
	}

	pStack = pMonitor->pStack;

	dave_free(pMonitor);

	return pStack;
}

static void
_uip_server_monitor_timer_out(void *kv, s8 *key)
{
	UIPMonitor *pMonitor = (UIPMonitor *)base_ramkv_del_key_ptr(_uip_monitor_kv, key);
	UIPStack *pStack;
	RetCode ret;

	if(pMonitor != NULL)
	{
		pStack = pMonitor->pStack;

		UIPLOG("%x/%x %s method:%s channel:%s timer out:%lds!",
			pMonitor, pMonitor->pStack,
			thread_name(pMonitor->pStack->src),
			pMonitor->pStack->head.method,
			pMonitor->pStack->head.channel,
			(dave_os_time_us() - pMonitor->malloc_time)/1000000);

		if(pStack != NULL)
		{
			ret = RetCode_timer_out;

			_uip_monitor_error_fun(pStack->src, ret, pStack->ptr, NULL);

			uip_free(pStack);
		}

		dave_free(pMonitor);
	}
}

static RetCode
_uip_server_monitor_recycle(void *kv, s8 *key)
{
	UIPMonitor *pMonitor = (UIPMonitor *)base_ramkv_del_key_ptr(kv, key);

	if(pMonitor == NULL)
		return RetCode_empty_data;
		
	dave_free(pMonitor);

	return RetCode_OK;
}

static ub
_uip_server_monitor_kv_timer(void)
{
	u8 kv_timer_str[128];
	ub kv_timer;

	if(database_get(CFG_UIP_SERVER_MONITOR_TIMER, kv_timer_str, sizeof(kv_timer_str)) == dave_false)
	{
		digitalstring((s8 *)kv_timer_str, sizeof(kv_timer_str), MONITOR_KV_DEFAULT_TIME);
		database_set(CFG_UIP_SERVER_MONITOR_TIMER, kv_timer_str, dave_strlen(kv_timer_str));
		return MONITOR_KV_DEFAULT_TIME;
	}

	kv_timer = stringdigital((s8 *)kv_timer_str);
	if(kv_timer < MONITOR_KV_MIN_TIME)
	{
		UIPLOG("invalid kv_timer:%d recovery time to %ds", kv_timer, MONITOR_KV_MIN_TIME);
		kv_timer = MONITOR_KV_MIN_TIME;
	}

	return kv_timer;
}

// =====================================================================

void
uip_server_monitor_init(uip_monitor_error error_fun)
{
	ub kv_timer = _uip_server_monitor_kv_timer();

	_uip_monitor_kv = base_ramkv_malloc((s8 *)"uipmonitor", KvAttrib_list, kv_timer, _uip_server_monitor_timer_out);

	_uip_monitor_check_time_consuming = MONITOR_CHECK_DEFAULT_CONSUMING_TIME;

	_uip_monitor_error_fun = error_fun;
}

void
uip_server_monitor_exit(void)
{
	base_ramkv_free(_uip_monitor_kv, _uip_server_monitor_recycle);

	_uip_monitor_kv = NULL;
}

void *
uip_server_monitor_malloc(UIPStack *pStack)
{
	void *monitor_ptr;

	monitor_ptr = (void *)_uip_server_monitor_malloc(pStack);

	UIPDEBUG("monitor:%x", monitor_ptr);

	return monitor_ptr;
}

UIPStack *
uip_server_monitor_free(void *monitor_ptr)
{
	if(monitor_ptr == NULL)
	{
		UIPABNOR("empty monitor_ptr!");
		return NULL;
	}

	UIPDEBUG("monitor:%x", monitor_ptr);

	return  _uip_server_monitor_free((UIPMonitor *)monitor_ptr);
}

void
uip_server_monitor_time_consuming(ub time_consuming)
{
	UIPLOG("Response time detection is set from %d to %d", _uip_monitor_check_time_consuming, time_consuming);

	_uip_monitor_check_time_consuming = time_consuming;
}

