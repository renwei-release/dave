/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_base.h"
#include "thread_struct.h"
#include "thread_log.h"

extern ub thread_cfg_system_memory_max_use_percentage(void);
extern ub thread_cfg_multiple_coroutine_on_thread(void);

static inline dave_bool
_thread_flow_control(
	ThreadStruct *pThread,
	ub *memory_use_percentage, ub *coroutines_site_creat_counter, ub *coroutines_site_release_counter)
{
	ub site_counter;

	*memory_use_percentage = 0;
	*coroutines_site_creat_counter = pThread->coroutines_site_creat_counter;
	*coroutines_site_release_counter = pThread->coroutines_site_release_counter;

	/*
	 * if the memory is not enough, we will not read the message.
	 * This is worried that the application layer may consume a lot of memory
	 * when handling these news, and the operating system may kill this process.
	 * This is the behavior of throttling based on memory usage.
	 */

	if(((pThread->thread_flag & THREAD_THREAD_FLAG) == 0x00)
		|| (pThread->thread_flag & THREAD_PRIVATE_FLAG)
		|| ((pThread->thread_flag & THREAD_COROUTINE_FLAG) == 0x00)
		|| (pThread->thread_flag & THREAD_CORE_FLAG)
		|| (pThread->attrib == REMOTE_TASK_ATTRIB))
	{
		return dave_true;
	}

	if((pThread->thread_flag & THREAD_THREAD_FLAG)
		&& (pThread->thread_flag & THREAD_COROUTINE_FLAG))
	{
		site_counter = *coroutines_site_creat_counter - *coroutines_site_release_counter;
	
		if(site_counter >= (pThread->level_number * thread_cfg_multiple_coroutine_on_thread()))
		{
			*memory_use_percentage = dave_os_memory_use_percentage();

			return dave_false;
		}
	}

	*memory_use_percentage = dave_os_memory_use_percentage();

	if(*memory_use_percentage > thread_cfg_system_memory_max_use_percentage())
	{
		return dave_false;
	}

	return dave_true;
}

// =====================================================================

dave_bool
thread_flow_control(ThreadStruct *pThread)
{
	ub memory_use_percentage, coroutines_site_creat_counter, coroutines_site_release_counter;
	dave_bool ret = _thread_flow_control(
		pThread,
		&memory_use_percentage, &coroutines_site_creat_counter, &coroutines_site_release_counter);

	if(ret == dave_false)
	{
		THREADLTRACE(3, 1,
			"Service %s at flow control! memory(sys:%ld cfg:%ld) coroutines(creat:%ld release:%ld max number of coroutine:%ld)",
			pThread->thread_name,
			memory_use_percentage, thread_cfg_system_memory_max_use_percentage(),
			coroutines_site_creat_counter, coroutines_site_release_counter,
			pThread->level_number * thread_cfg_multiple_coroutine_on_thread());
	}

	return ret;
}

#endif

