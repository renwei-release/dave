/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_base.h"
#include "dave_tools.h"
#include "dll_log.h"

extern void (*__free_hook)(void *ptr);
extern void *(*__malloc_hook)(size_t size);

// =====================================================================

int
dave_dll_self_check(char *string_data, int int_data, float float_data, dll_checkback_fun checkback)
{
	int fun_ret;

	if(dave_strcmp(string_data, "123456") == dave_false)
	{
		DLLLOG("invalid string_data:%s\n", string_data);
		return -1;
	}

	if(int_data != 123456)
	{
		DLLLOG("invalid int_data:%d\n", int_data);
		return -1;
	}

	if(abs(float_data-123456.123456) > 0.1)
	{
		DLLLOG("invalid float_data:%f\n", float_data);
		return -1;
	}

	if(checkback != NULL)
	{
		fun_ret = checkback(123456);
		if(fun_ret != 123456)
		{
			DLLLOG("invalid fun_ret:%d\n", fun_ret);
			return -1;
		}
	}

	DLLLOG("malloc:%lx __malloc_hook:%lx free:%lx __free_hook:%lx",
		malloc, __malloc_hook, free, __free_hook);

	return 0;
}

#endif

