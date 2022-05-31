/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "uip_msg.h"
#include "uip_log.h"

#define REGISTER_TABLE_NAME (s8 *)"uipreg"

typedef struct {
	s8 method[DAVE_UIP_METHOD_MAX_LEN];
	ThreadId thread_id;
	s8 thread_name[DAVE_THREAD_NAME_LEN];
} UIPRegister;

static void *pKV = NULL;

static dave_bool
_uip_server_register_malloc(ThreadId src, s8 *method)
{
	UIPRegister *pRegister;

	pRegister = base_ramkv_inq_key_ptr(pKV, method);
	if(pRegister == NULL)
	{
		pRegister = dave_malloc(sizeof(UIPRegister));
	}

	dave_strcpy(pRegister->method, method, DAVE_UIP_METHOD_MAX_LEN);
	pRegister->thread_id = src;
	dave_strcpy(pRegister->thread_name, thread_name(src), DAVE_THREAD_NAME_LEN);

	return base_ramkv_add_key_ptr(pKV, pRegister->method, pRegister);
}

static RetCode
_uip_server_register_free(void *kv, s8 *method)
{
	UIPRegister *pRegister = base_ramkv_del_key_ptr(pKV, method);

	if(pRegister == NULL)
	{
		return RetCode_empty_data;
	}

	dave_free(pRegister);

	return RetCode_OK;
}

static s8 *
_uip_server_register_method_to_thread(s8 *method)
{
	UIPRegister *pRegister;

	pRegister = base_ramkv_inq_key_ptr(pKV, method);
	if(pRegister == NULL)
	{
		return NULL;
	}

	return pRegister->thread_name;
}

// =====================================================================

void
uip_server_register_init(void)
{
	pKV = base_ramkv_malloc(REGISTER_TABLE_NAME, KvAttrib_list, 0, NULL);
}

void
uip_server_register_exit(void)
{
	base_ramkv_free(pKV, _uip_server_register_free);
}

RetCode
uip_server_register(ThreadId src, s8 *method)
{
	UIPTRACE("register method:%s", method);

	if(_uip_server_register_malloc(src, method) == dave_true)
	{
		return RetCode_OK;
	}

	UIPLOG("register method:%s failed!", method);

	return RetCode_invalid_option;
}

RetCode
uip_server_unregister(s8 *method)
{
	UIPTRACE("unregister method:%s", method);

	return _uip_server_register_free(pKV, method);
}

RetCode
uip_server_register_data(s8 *thread, s8 *method)
{
	s8 *local_thread;

	if((method == NULL) || (method[0] == '\0'))
	{
		return RetCode_Invalid_data;
	}

	local_thread = _uip_server_register_method_to_thread(method);
	if(local_thread == NULL)
	{
		UIPLOG("method:%s not find!", method);
		return RetCode_can_not_find_thread;
	}

	dave_strcpy(thread, local_thread, DAVE_THREAD_NAME_LEN);

	return RetCode_OK;
}

ub
uip_server_register_info(s8 *info_ptr, ub info_len)
{
	ub safe_index, info_index;
	UIPRegister *pRegister;

	safe_index = 0;
	info_index = 0;

	info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "UIP REGISTER INFO:\r\n");

	while(safe_index < 1024000)
	{
		pRegister = base_ramkv_inq_index_ptr(pKV, safe_index ++);
		if(pRegister == NULL)
		{
			break;
		}
		
		info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "method:%s thread:%s\r\n", pRegister->method, pRegister->thread_name);
	}

	return info_index;
}

