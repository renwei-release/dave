/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "dave_tools.h"
#include "thread_log.h"

#define CFG_SYSTEM_MEMORY_MAX_USE_PERCENTAGE "SysMemMaxUsePercentage"
#define default_CFG_SYSTEM_MEMORY_MAX_USE_PERCENTAGE 85

static ub _sys_mem_max_use_percentage = 0;

static void
_thread_cfg_update_sys_mem_max_use_percentage(void)
{
	ub use_percentage = cfg_get_ub(CFG_SYSTEM_MEMORY_MAX_USE_PERCENTAGE, default_CFG_SYSTEM_MEMORY_MAX_USE_PERCENTAGE);

	if((use_percentage >= 100) || (use_percentage <= 10))
	{
		THREADLOG("find invalid %s:%d, reset by default:%d",
			CFG_SYSTEM_MEMORY_MAX_USE_PERCENTAGE,
			use_percentage, default_CFG_SYSTEM_MEMORY_MAX_USE_PERCENTAGE);
		use_percentage = default_CFG_SYSTEM_MEMORY_MAX_USE_PERCENTAGE;
		cfg_set_ub(CFG_SYSTEM_MEMORY_MAX_USE_PERCENTAGE, default_CFG_SYSTEM_MEMORY_MAX_USE_PERCENTAGE);
	}

	if(use_percentage != _sys_mem_max_use_percentage)
	{
		if(_sys_mem_max_use_percentage != 0)
		{
			THREADLOG("%s change:%d->%d",
				CFG_SYSTEM_MEMORY_MAX_USE_PERCENTAGE,
				_sys_mem_max_use_percentage, use_percentage);
		}
		_sys_mem_max_use_percentage = use_percentage;
	}
}

static void
_thread_cfg_update(s8 *name_ptr, ub name_len, s8 *value_ptr, ub value_len)
{
	if(dave_strcmp(name_ptr, CFG_SYSTEM_MEMORY_MAX_USE_PERCENTAGE) == dave_true)
	{
		_thread_cfg_update_sys_mem_max_use_percentage();
	}
}

// =====================================================================

void
thread_cfg_init(void)
{
	_thread_cfg_update_sys_mem_max_use_percentage();

	cfg_reg(CFG_SYSTEM_MEMORY_MAX_USE_PERCENTAGE, _thread_cfg_update);
}

void
thread_cfg_exit(void)
{

}

ub
thread_cfg_system_memory_max_use_percentage(void)
{
	return _sys_mem_max_use_percentage;
}

#endif

