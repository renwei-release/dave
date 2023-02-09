/*
 * Copyright (c) 2022 Renwei
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
dave_dll_mmalloc(int length, char *func, int line)
{
	return __base_mmalloc__((ub)length, (s8 *)__func__, (ub)__LINE__);
}

int
dave_dll_mfree(void *m, char *func, int line)
{
	return (int)__base_mfree__((MBUF *)m, (s8 *)__func__, (ub)__LINE__);
}

int
dave_dll_cfg_set(char *cfg_name, char *cfg_value)
{
	u32 value_len = dave_strlen(cfg_value);

	if((value_len == 0) || (value_len > 8192))
	{
		DLLLOG("cfg set:%s failed! the length too longer:%d", cfg_name, value_len);
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

#endif

