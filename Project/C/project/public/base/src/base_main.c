/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "dave_os.h"
#include "dave_tools.h"
#include "base_lock.h"
#include "base_log.h"
#include "base_timer.h"
#include "base_rxtx.h"

static volatile dave_bool _base_running = dave_true;
static volatile dave_bool _base_power_state = dave_true;
static void *_main_thread_id = NULL;

// =====================================================================

void
base_init(void *main_thread_id, s8 *sync_domain)
{
	_main_thread_id = main_thread_id;

	booting_lock();
	base_log_init();
	base_mem_init();
	base_ramkv_init();
	base_cfg_init();
	base_thread_init(main_thread_id, sync_domain);
	base_timer_init();
	base_socket_init();
	base_rxtx_init();
	base_log_stack_init();
	base_dos_init();
}

dave_bool
base_running(dave_bool platform_schedule)
{
	do {

		base_thread_schedule();

	} while((platform_schedule == dave_false) && (_base_running == dave_true));

	return _base_running;
}

void
base_exit(void)
{
	base_dos_exit();
	base_log_stack_exit();
	base_rxtx_exit();
	base_socket_exit();
	base_timer_exit();
	base_thread_exit();
	base_cfg_exit();
	base_ramkv_exit();
	base_mem_exit();
	base_log_exit();
}

void
base_restart(const char *args, ...)
{
	va_list list_args;
	POWEROFFMSG *pRoweroff = thread_msg(pRoweroff);

	_base_power_state = dave_false;

	va_start(list_args, args);
	vsnprintf((char *)pRoweroff->reason, sizeof(pRoweroff->reason), args, list_args);
	va_end(list_args);

	id_msg(thread_id(GUARDIAN_THREAD_NAME), MSGID_POWER_OFF, pRoweroff);
}

void
base_power_off(s8 *reason)
{
	DateStruct date;
	s8 file_name[64];

	_base_running = dave_false;

	dave_os_thread_wakeup(_main_thread_id);

	if(reason != NULL)
	{
		date = t_time_get_date(NULL);

		dave_snprintf(file_name, sizeof(file_name), "CORE-DUMP-%04d-%02d-%02d_%02d:%02d:%02d",
			date.year, date.month, date.day,
			date.hour, date.minute, date.second);

		dave_os_file_write(CREAT_WRITE_FLAG, file_name, 0, dave_strlen(reason), (u8 *)reason);
	}
	else
	{
		reason = "Bye!";
	}

	dave_os_power_off(reason);
}

dave_bool
base_power_state(void)
{
	return _base_power_state;
}

#endif

