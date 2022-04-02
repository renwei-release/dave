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
#include "kv_param.h"
#include "kv_struct.h"
#include "kv_api.h"
#include "kv_log.h"

// ====================================================================

void
base_kv_init(void)
{
	kv_struct_init();
}

void
base_kv_exit(void)
{
	kv_struct_exit();
}

void *
__base_kv_malloc__(dave_bool external_call, s8 *name, KVAttrib attrib, ub out_second, kv_out_callback callback_fun, s8 *fun, ub line)
{
	return (void *)__kv_malloc__(external_call, name, attrib, out_second, callback_fun, fun, line);
}

void
__base_kv_free__(dave_bool external_call, void *kv, kv_free_recycle_callback callback_fun, s8 *fun, ub line)
{
	ub safe_counter;
	u8 key_ptr[KV_KEY_MAX];
	ErrCode ret;

	if(callback_fun != NULL)
	{
		safe_counter = 0;

		while((++ safe_counter) < 999999999)
		{
			if(kv_top(kv, key_ptr, sizeof(key_ptr)) == dave_true)
				ret = callback_fun(kv, (s8 *)key_ptr);
			else
				ret = callback_fun(kv, NULL);
			if(ret != ERRCODE_OK)
			{
				KVDEBUG("key:%s get error code:%s <%s:%d>", key_ptr, errorstr(ret), fun, line);
				break;
			}
		}

		if(safe_counter >= 999999999)
		{
			KVABNOR("while out times:%d <%s:%d>", safe_counter, fun, line);
		}
	}

	KVDEBUG("name:%s", ((KV *)(kv))->name);

	kv_free(external_call, (KV *)(kv));
}

dave_bool
base_kv_check(void *kv)
{
	return kv_check((KV *)(kv));
}

dave_bool
__base_kv_add_key_ptr__(void *kv, s8 *key, void *ptr, s8 *fun, ub line)
{
	ub ptr_value;

	ptr_value = (ub)ptr;

	return kv_add((KV *)kv, (u8 *)key, dave_strlen(key), (void *)(&ptr_value), sizeof(ptr_value), fun, line);
}

void *
__base_kv_inq_key_ptr__(void *kv, s8 *key, s8 *fun, ub line)
{
	ub ptr_value;

	if(kv_inq((KV *)kv, -1, (u8 *)key, dave_strlen(key), (void *)(&ptr_value), sizeof(ptr_value), fun, line) != sizeof(ptr_value))
	{
		return NULL;
	}
	else
	{
		return (void *)ptr_value;
	}
}

void *
__base_kv_inq_index_ptr__(void *kv, ub index, s8 *fun, ub line)
{
	ub ptr_value;

	if(kv_inq((KV *)kv, (sb)index, NULL, 0, (void *)(&ptr_value), sizeof(ptr_value), fun, line) != sizeof(ptr_value))
	{
		return NULL;
	}
	else
	{
		return (void *)ptr_value;
	}
}

void *
__base_kv_del_key_ptr__(void *kv, s8 *key, s8 *fun, ub line)
{
	ub ptr_value;

	if(kv_del((KV *)kv, (u8 *)key, dave_strlen(key), (void *)(&ptr_value), sizeof(ptr_value), fun, line) != sizeof(ptr_value))
	{
		return NULL;
	}
	else
	{
		return (void *)ptr_value;
	}
}

dave_bool
__base_kv_add_ub_ptr__(void *kv, ub ub_key, void *ptr, s8 *fun, ub line)
{
	s8 key_value[32];
	ub key_len;
	ub ptr_value;

	key_len = dave_snprintf(key_value, sizeof(key_value), "%lx", ub_key);
	ptr_value = (ub)ptr;

	return kv_add((KV *)kv, (u8 *)key_value, key_len, (void *)(&ptr_value), sizeof(ptr_value), fun, line);
}

void *
__base_kv_inq_ub_ptr__(void *kv, ub ub_key, s8 *fun, ub line)
{
	s8 key_value[32];
	ub key_len;
	ub ptr_value;

	key_len = dave_snprintf(key_value, sizeof(key_value), "%lx", ub_key);

	if(kv_inq((KV *)kv, -1, (u8 *)key_value, key_len, (void *)(&ptr_value), sizeof(ptr_value), fun, line) != sizeof(ptr_value))
	{
		return NULL;
	}
	else
	{
		return (void *)ptr_value;
	}
}

void *
__base_kv_del_ub_ptr__(void *kv, ub ub_key, s8 *fun, ub line)
{
	s8 key_value[32];
	ub key_len;
	ub ptr_value;

	key_len = dave_snprintf(key_value, sizeof(key_value), "%lx", ub_key);

	if(kv_del((KV *)kv, (u8 *)key_value, key_len, (void *)(&ptr_value), sizeof(ptr_value), fun, line) != sizeof(ptr_value))
	{
		return NULL;
	}
	else
	{
		return (void *)ptr_value;
	}
}

dave_bool
__base_kv_add_bin_ptr__(void *kv, u8 *bin_data, ub bin_len, void *ptr, s8 *fun, ub line)
{
	ub ptr_value;

	ptr_value = (ub)ptr;

	return kv_add((KV *)kv, bin_data, bin_len, (void *)(&ptr_value), sizeof(ptr_value), fun, line);
}

void *
__base_kv_inq_bin_ptr__(void *kv, u8 *bin_data, ub bin_len, s8 *fun, ub line)
{
	ub ptr_value;

	if(kv_inq((KV *)kv, -1, bin_data, bin_len, (void *)(&ptr_value), sizeof(ptr_value), fun, line) != sizeof(ptr_value))
	{
		return NULL;
	}
	else
	{
		return (void *)ptr_value;
	}
}

void *
__base_kv_del_bin_ptr__(void *kv, u8 *bin_data, ub bin_len, s8 *fun, ub line)
{
	ub ptr_value;

	if(kv_del((KV *)kv, bin_data, bin_len, (void *)(&ptr_value), sizeof(ptr_value), fun, line) != sizeof(ptr_value))
	{
		return NULL;
	}
	else
	{
		return (void *)ptr_value;
	}
}

void *
__base_kv_inq_top_ptr__(void *kv, s8 *fun, ub line)
{
	u8 key[KV_KEY_MAX];

	if(kv_top((KV *)kv, key, sizeof(key)) == dave_false)
	{
		return NULL;
	}
	else
	{
		return __base_kv_inq_key_ptr__(kv, (s8 *)key, fun, line);
	}
}

ub
base_kv_info(void *kv, s8 *info_ptr, ub info_len)
{
	return kv_info((KV *)kv, info_ptr, info_len);
}

#endif

