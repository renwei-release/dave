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
#include "ramkv_struct.h"
#include "ramkv_api.h"
#include "ramkv_log.h"

#define RECYCLE_LOOP_MAX 999999999

static RetCode
_base_ramkv_system_recycle(void *ramkv, s8 *key)
{
	if(key == NULL)
	{
		KVDEBUG("key is NULL!");
		return RetCode_empty_data;
	}

	if(kv_del_key_ptr(ramkv, key) != NULL)
		return RetCode_OK;
	else
		return RetCode_empty_data;
}

static void
_base_ramkv_recycle_run(void *ramkv, ramkv_recycle_callback recycle_fun, s8 *fun, ub line)
{
	ub safe_counter;
	u8 key_ptr[RAMKV_KEY_MAX];
	RetCode ret;

	safe_counter = 0;

	while((++ safe_counter) < RECYCLE_LOOP_MAX)
	{
		if(ramkv_top(ramkv, key_ptr, sizeof(key_ptr)) == dave_true)
			ret = recycle_fun(ramkv, (s8 *)key_ptr);
		else
			ret = recycle_fun(ramkv, NULL);

		if(ret != RetCode_OK)
		{
			KVDEBUG("key:%s get error code:%s <%s:%d>", key_ptr, retstr(ret), fun, line);
			break;
		}
	}

	if(safe_counter >= RECYCLE_LOOP_MAX)
	{
		KVABNOR("while out times:%d <%s:%d>", safe_counter, fun, line);
	}
}

// ====================================================================

void
base_ramkv_init(void)
{
	ramkv_struct_init();
}

void
base_ramkv_exit(void)
{
	ramkv_struct_exit();
}

dave_bool
base_ramkv_check(void *ramkv)
{
	return ramkv_check((KV *)(ramkv));
}

ub
base_ramkv_info(void *ramkv, s8 *info_ptr, ub info_len)
{
	return ramkv_info((KV *)ramkv, info_ptr, info_len);
}

void *
__base_ramkv_malloc__(dave_bool external_call, s8 *name, KvAttrib attrib, ub out_second, ramkv_time_callback outback_fun, s8 *fun, ub line)
{
	return (void *)__ramkv_malloc__(external_call, name, attrib, out_second, outback_fun, fun, line);
}

void
__base_ramkv_free__(dave_bool external_call, void *ramkv, ramkv_recycle_callback recycle_fun, s8 *fun, ub line)
{
	if(ramkv == NULL)
	{
		KVLOG("free null on <%s:%d>", fun, line);
		return;
	}

	if(recycle_fun == NULL)
	{
		recycle_fun = _base_ramkv_system_recycle;
	}

	_base_ramkv_recycle_run(ramkv, recycle_fun, fun, line);

	KVDEBUG("name:%s", ((KV *)(ramkv))->name);

	__ramkv_free__(external_call, (KV *)(ramkv), fun, line);
}

dave_bool
__base_ramkv_add_key_ptr__(void *ramkv, s8 *key, void *ptr, s8 *fun, ub line)
{
	ub ptr_value;

	ptr_value = (ub)ptr;

	return ramkv_add((KV *)ramkv, (u8 *)key, dave_strlen(key), (void *)(&ptr_value), sizeof(ptr_value), fun, line);
}

void *
__base_ramkv_inq_key_ptr__(void *ramkv, s8 *key, s8 *fun, ub line)
{
	ub ptr_value;

	if(ramkv_inq((KV *)ramkv, -1, (u8 *)key, dave_strlen(key), (void *)(&ptr_value), sizeof(ptr_value), fun, line) != sizeof(ptr_value))
	{
		return NULL;
	}
	else
	{
		return (void *)ptr_value;
	}
}

void *
__base_ramkv_inq_index_ptr__(void *ramkv, ub index, s8 *fun, ub line)
{
	ub ptr_value;

	if(ramkv_inq((KV *)ramkv, (sb)index, NULL, 0, (void *)(&ptr_value), sizeof(ptr_value), fun, line) != sizeof(ptr_value))
	{
		return NULL;
	}
	else
	{
		return (void *)ptr_value;
	}
}

void *
__base_ramkv_del_key_ptr__(void *ramkv, s8 *key, s8 *fun, ub line)
{
	ub ptr_value;

	if(ramkv_del((KV *)ramkv, (u8 *)key, dave_strlen(key), (void *)(&ptr_value), sizeof(ptr_value), fun, line) != sizeof(ptr_value))
	{
		return NULL;
	}
	else
	{
		return (void *)ptr_value;
	}
}

dave_bool
__base_ramkv_add_ub_ptr__(void *ramkv, ub ub_key, void *ptr, s8 *fun, ub line)
{
	s8 key_value[32];
	ub key_len;
	ub ptr_value;

	key_len = dave_snprintf(key_value, sizeof(key_value), "%lx", ub_key);
	ptr_value = (ub)ptr;

	return ramkv_add((KV *)ramkv, (u8 *)key_value, key_len, (void *)(&ptr_value), sizeof(ptr_value), fun, line);
}

void *
__base_ramkv_inq_ub_ptr__(void *ramkv, ub ub_key, s8 *fun, ub line)
{
	s8 key_value[32];
	ub key_len;
	ub ptr_value;

	key_len = dave_snprintf(key_value, sizeof(key_value), "%lx", ub_key);

	if(ramkv_inq((KV *)ramkv, -1, (u8 *)key_value, key_len, (void *)(&ptr_value), sizeof(ptr_value), fun, line) != sizeof(ptr_value))
	{
		return NULL;
	}
	else
	{
		return (void *)ptr_value;
	}
}

void *
__base_ramkv_del_ub_ptr__(void *ramkv, ub ub_key, s8 *fun, ub line)
{
	s8 key_value[32];
	ub key_len;
	ub ptr_value;

	key_len = dave_snprintf(key_value, sizeof(key_value), "%lx", ub_key);

	if(ramkv_del((KV *)ramkv, (u8 *)key_value, key_len, (void *)(&ptr_value), sizeof(ptr_value), fun, line) != sizeof(ptr_value))
	{
		return NULL;
	}
	else
	{
		return (void *)ptr_value;
	}
}

dave_bool
__base_ramkv_add_bin_ptr__(void *ramkv, u8 *bin_data, ub bin_len, void *ptr, s8 *fun, ub line)
{
	ub ptr_value;

	ptr_value = (ub)ptr;

	return ramkv_add((KV *)ramkv, bin_data, bin_len, (void *)(&ptr_value), sizeof(ptr_value), fun, line);
}

void *
__base_ramkv_inq_bin_ptr__(void *ramkv, u8 *bin_data, ub bin_len, s8 *fun, ub line)
{
	ub ptr_value;

	if(ramkv_inq((KV *)ramkv, -1, bin_data, bin_len, (void *)(&ptr_value), sizeof(ptr_value), fun, line) != sizeof(ptr_value))
	{
		return NULL;
	}
	else
	{
		return (void *)ptr_value;
	}
}

void *
__base_ramkv_del_bin_ptr__(void *ramkv, u8 *bin_data, ub bin_len, s8 *fun, ub line)
{
	ub ptr_value;

	if(ramkv_del((KV *)ramkv, bin_data, bin_len, (void *)(&ptr_value), sizeof(ptr_value), fun, line) != sizeof(ptr_value))
	{
		return NULL;
	}
	else
	{
		return (void *)ptr_value;
	}
}

void *
__base_ramkv_inq_top_ptr__(void *ramkv, s8 *fun, ub line)
{
	u8 key[RAMKV_KEY_MAX];

	if(ramkv_top((KV *)ramkv, key, sizeof(key)) == dave_false)
	{
		return NULL;
	}
	else
	{
		return __base_ramkv_inq_key_ptr__(ramkv, (s8 *)key, fun, line);
	}
}

dave_bool
__base_ramkv_add_key_value__(void *ramkv, s8 *key, s8 *value, s8 *fun, ub line)
{
	return ramkv_add((KV *)ramkv, (u8 *)key, dave_strlen(key), (void *)value, dave_strlen(value), fun, line);
}

sb
__base_ramkv_inq_key_value__(void *ramkv, s8 *key, s8 *value_ptr, ub value_len, s8 *fun, ub line)
{
	return ramkv_inq((KV *)ramkv, -1, (u8 *)key, dave_strlen(key), (void *)value_ptr, value_len, fun, line);
}

sb
__base_ramkv_index_key_value__(void *ramkv, sb index, s8 *key_ptr, ub key_len, s8 *value_ptr, ub value_len, s8 *fun, ub line)
{
	if((key_ptr != NULL) && (key_len > 0))
	{
		((s8 *)key_ptr)[0] = '\0';
	}	

	return ramkv_inq((KV *)ramkv, index, (void *)key_ptr, key_len, (void *)value_ptr, value_len, fun, line);
}

dave_bool
__base_ramkv_del_key_value__(void *ramkv, s8 *key, s8 *fun, ub line)
{
	ramkv_del((KV *)ramkv, (u8 *)key, dave_strlen(key), NULL, 0, fun, line);
	return dave_true;
}

#endif

