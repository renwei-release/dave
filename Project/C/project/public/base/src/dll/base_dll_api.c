/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_base.h"
#include "dave_os.h"
#include "dave_verno.h"
#include "dave_tools.h"
#include "dll_tools.h"
#include "dll_log.h"

// =====================================================================

char *
dave_dll_verno(void)
{
	return (char *)dave_verno();
}

char *
dave_dll_reset_verno(char *verno)
{
	return dave_verno_reset(verno);
}

void *
dave_dll_mmalloc(int length, char *fun, int line)
{
	if(fun == NULL)
	{
		fun = (char *)__func__;
		line = __LINE__;
	}

	return __base_mmalloc__((ub)length, (s8 *)fun, (ub)line);
}

int
dave_dll_mfree(void *m, char *fun, int line)
{
	if(fun == NULL)
	{
		fun = (char *)__func__;
		line = __LINE__;
	}

	return (int)__base_mfree__((MBUF *)m, (s8 *)fun, (ub)line);
}

int
dave_dll_cfg_set(char *cfg_name, char *cfg_value)
{
	ub name_len = dave_strlen(cfg_name);
	ub value_len = dave_strlen(cfg_value);

	if((name_len == 0) || (value_len > 8192))
	{
		DLLLOG("cfg set:%s:%s failed! invalid length:%ld/%ld",
			cfg_name, cfg_value,
			name_len, value_len);
		return -1;
	}

	if(cfg_set((s8 *)cfg_name, (u8 *)cfg_value, value_len) != RetCode_OK)
		return -1;
	else
		return 0;
}

int
dave_dll_cfg_get(char *cfg_name, char *cfg_value_ptr, int cfg_value_len)
{
	dave_memset(cfg_value_ptr, 0x00, cfg_value_len);

	if(cfg_get((s8 *)cfg_name, (u8 *)cfg_value_ptr, (u32)cfg_value_len) == dave_false)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int
dave_dll_cfg_del(char *cfg_name)
{
	cfg_del((s8 *)cfg_name);

	return 0;
}

int
dave_dll_cfg_reg(char *cfg_name, dll_cfg_reg_fun reg_fun)
{
	if(cfg_reg((s8 *)cfg_name, (cfg_reg_fun)reg_fun) == dave_false)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int
dave_dll_cfg_remote_set(char *cfg_name, char *cfg_value, int ttl)
{
	u32 value_len = dave_strlen(cfg_value);

	if((value_len == 0) || (value_len > 8192))
	{
		DLLLOG("cfg set:%s failed! the length too longer:%d", cfg_name, value_len);
		return -1;
	}

	if(rcfg_set((s8 *)cfg_name, (s8 *)cfg_value, (sb)ttl) != RetCode_OK)
		return -1;
	else
		return 0;
}

int
dave_dll_cfg_remote_get(char *cfg_name, char *cfg_value_ptr, int cfg_value_len)
{
	dave_memset(cfg_value_ptr, 0x00, cfg_value_len);

	if(rcfg_get((s8 *)cfg_name, (s8 *)cfg_value_ptr, cfg_value_len) < 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int
dave_dll_cfg_remote_del(char *cfg_name)
{
	if(rcfg_del(cfg_name) == dave_false)
		return -1;
	else
		return 0;
}

void
dave_dll_poweroff(void)
{
	base_restart("dll poweroff");
}

void *
dave_dll_kv_malloc(char *name, int out_second, dll_kv_timerout_fun outback_fun)
{
	return kv_malloc(name, out_second, (ramkv_time_callback)outback_fun);
}

void
dave_dll_kv_free(void *kv)
{
	kv_free(kv, NULL);
}

int
dave_dll_kv_add(void *kv, char *key, char *value)
{
	if(kv_add_key_value(kv, key, value) == dave_true)
		return 0;
	else
		return -1;
}

int
dave_dll_kv_inq(void *kv, char *key, char *value_ptr, int value_len)
{
	return (int)kv_inq_key_value(kv, key, value_ptr, (int)value_len);
}

int
dave_dll_kv_del(void *kv, char *key)
{
	kv_del_key_value(kv, key);
	return 0;
}

int
dave_dll_dos_cmd_reg(const char *cmd, dll_dos_cmd_fun cmd_fun)
{
	RetCode ret;

	ret = dos_cmd_reg(cmd, (dos_cmd_fun)cmd_fun, NULL);
	if(ret == RetCode_OK)
		return 0;

	return -1;
}

void
dave_dll_dos_print(char *msg)
{
	ub msg_len = 4096;
	s8 *msg_buffer = dave_malloc(msg_len);

	if(msg != NULL)
	{
		msg_len = dave_strcpy(msg_buffer, msg, msg_len);
	}
	else
	{
		msg_buffer[0] = '\0';
		msg_len = 0;
	}

	dll_remove_some_data(msg_buffer, msg_len);

	dos_print("%s", msg_buffer);

	dave_free(msg_buffer);
}

char *
dave_dll_dos_get_user_input(char *give_user_msg, int wait_second)
{
	ub msg_len = 2048;
	s8 *msg_buffer = dave_malloc(msg_len);
	char *user_input;

	if(give_user_msg != NULL)
	{
		msg_len = dave_strcpy(msg_buffer, give_user_msg, msg_len);
	}
	else
	{
		msg_buffer[0] = '\0';
		msg_len = 0;
	}

	if(wait_second <= 0)
	{
		wait_second = 15;
	}

	dll_remove_some_data(msg_buffer, msg_len);

	user_input = dos_get_user_input(msg_buffer, (ub)wait_second);

	dave_free(msg_buffer);

	return user_input;
}

#endif

