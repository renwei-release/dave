/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "dave_tools.h"
#include "thread_struct.h"
#include "thread_quit.h"
#include "thread_parameter.h"
#include "thread_map.h"
#include "thread_tools.h"
#include "thread_log.h"

static dave_bool
_thread_must_in_local(s8 *thread_name)
{
	ThreadStruct *pThread;

	pThread = thread_find_busy_thread(thread_id(thread_name));
	if(pThread == NULL)
	{
		THREADDEBUG("%s in local!", thread_name);
		return dave_true;
	}

	if((pThread->thread_flag & THREAD_PRIVATE_FLAG) == THREAD_PRIVATE_FLAG)
	{
		return dave_true;
	}

	return dave_false;
}

// =====================================================================

/*
 * 最好的方式是通过设置thread_flag标记来确定这个线程是否要公开出去。
 */

dave_bool
thread_must_in_local(s8 *thread_name)
{
	dave_bool ret;

	ret = _thread_must_in_local(thread_name);

	THREADDEBUG("%s %s!", thread_name, ret==dave_true?"in local":"in remote");

	return ret;
}

#endif

