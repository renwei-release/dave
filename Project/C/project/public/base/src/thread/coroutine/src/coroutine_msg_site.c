/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_parameter.h"
#if defined(__DAVE_BASE__) && defined(ENABLE_THREAD_COROUTINE)
#include "dave_os.h"
#include "dave_tools.h"
#include "thread_log.h"

// =====================================================================

ub
coroutine_msg_site_malloc(void *pSite)
{
	ub current_time = dave_os_time_ms();
	ub msg_site = (current_time << 48) + ((ub)pSite & 0xffffffffffff);

	THREADDEBUG("msg_site:%lx time:%lx pSite:%lx", msg_site, current_time, pSite);

	return msg_site;
}

ub
coroutine_msg_site_free(ub msg_site, void *pSite)
{
	if((msg_site & 0xffffffffffff) != ((ub)pSite & 0xffffffffffff))
	{
		THREADABNOR("Arithmetic error:%lx/%lx", msg_site, pSite);
	}

	return 0;
}

#endif

