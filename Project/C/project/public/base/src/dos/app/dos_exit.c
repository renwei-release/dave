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

static RetCode
_dos_exit_help(void)
{
	dos_print("Usage: restart\nExit the system!");
	return RetCode_OK;
}

static RetCode
_dos_exit_user(s8 *param_ptr, ub param_len)
{
	const char *msg = "User exit the system!";

	dos_print(msg);
	base_restart(msg);

	return RetCode_OK;
}

// =====================================================================

void
dos_exit_reset(void)
{
	if(dave_os_on_docker() == dave_false)
	{
		dos_cmd_reg("exit", _dos_exit_user, _dos_exit_help);
	}
}

#endif

