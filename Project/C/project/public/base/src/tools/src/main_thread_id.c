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
#include "dave_base.h"
#include "dave_verno.h"
#include "base_tools.h"
#include "base_lock.h"
#include "base_dll_main.h"

static ThreadId _main_thread = INVALID_THREAD_ID;

static ThreadId
_product_main_id(void)
{
	s8 *product_name = dave_verno_my_product();
	ThreadId main_id;

	main_id = thread_id(product_name);
	if(main_id == INVALID_THREAD_ID)
	{
		main_id = thread_id(lower(product_name));
	}
	if(main_id == INVALID_THREAD_ID)
	{
		main_id = thread_id(upper(product_name));
	}

	return main_id;
}

// =====================================================================

ThreadId
main_thread_id_get(void)
{
	if(_main_thread != INVALID_THREAD_ID)
	{
		return _main_thread;
	}

	_main_thread = dave_dll_main_thread_id();
	if(_main_thread == INVALID_THREAD_ID)
	{
		_main_thread = _product_main_id();	
	}

	return _main_thread;
}

void
main_thread_id_set(ThreadId main_thread)
{
	_main_thread = main_thread;
}

#endif

