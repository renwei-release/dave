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
#include "dave_tools.h"
#include "dos_tty.h"
#include "dos_pop.h"
#include "dos_show.h"
#include "dos_cmd.h"
#include "dos_log.h"

static ErrCode
_dos_restart_help(void)
{
	dos_print("Usage: restart\nRestart the system!");
	return ERRCODE_OK;
}

static ErrCode
_dos_restart_user(s8 *param_ptr, ub param_len)
{
	const char *msg = "User exit the system!";

	dos_print(msg);
	dave_restart(msg);

	return ERRCODE_OK;
}

// =====================================================================

void
dos_exit_reset(void)
{
	if(dave_os_on_docker() == dave_false)
	{
		dos_cmd_register("exit", _dos_restart_user, (help_process_fun)_dos_restart_help);
	}
	else
	{
		dos_cmd_register("restart", _dos_restart_user, (help_process_fun)_dos_restart_help);
	}
}

#endif

