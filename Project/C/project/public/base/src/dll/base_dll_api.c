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

void
dave_dll_wait_dll_exit(void)
{
	while(base_power_state() == dave_true) dave_os_sleep(1000); ;
}

int
dave_dll_cfg_set(char *cfg_name, char *cfg_ptr)
{
	u32 cfg_len = dave_strlen(cfg_ptr);

	if((cfg_len == 0) || (cfg_len > 8192))
	{
		DLLLOG("cfg set:%s failed! the length too longer:%d", cfg_name, cfg_len);
		return -1;
	}

	if(cfg_set((s8 *)cfg_name, (u8 *)cfg_ptr, cfg_len) != RetCode_OK)
		return -1;
	else
		return 0;
}

int
dave_dll_cfg_get(char *cfg_name, char *cfg_ptr, int cfg_len)
{
	dave_memset(cfg_ptr, 0x00, cfg_len);

	if(cfg_get((s8 *)cfg_name, (u8 *)cfg_ptr, (u32)cfg_len) == dave_false)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int
dave_dll_cfg_remote_set(char *cfg_name, char *cfg_ptr)
{
	u32 cfg_len = dave_strlen(cfg_ptr);

	if((cfg_len == 0) || (cfg_len > 8192))
	{
		DLLLOG("cfg set:%s failed! the length too longer:%d", cfg_name, cfg_len);
		return -1;
	}

	if(rcfg_set((s8 *)cfg_name, (s8 *)cfg_ptr) != RetCode_OK)
		return -1;
	else
		return 0;
}

int
dave_dll_cfg_remote_get(char *cfg_name, char *cfg_ptr, int cfg_len)
{
	dave_memset(cfg_ptr, 0x00, cfg_len);

	if(rcfg_get((s8 *)cfg_name, (s8 *)cfg_ptr, cfg_len) == dave_false)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

void
dave_dll_poweroff(void)
{
	base_restart("dll poweroff");
}

#endif

