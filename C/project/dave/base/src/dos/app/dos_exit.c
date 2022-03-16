/*
 * ================================================================================
 * (c) Copyright 2016 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_base.h"
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
_dos_restart_user(s8 *param, ub param_len)
{
	const char *msg = "User restart the system!";

	dos_print(msg);
	dave_restart(msg);

	return ERRCODE_OK;
}

static ErrCode
_dos_exit_help(void)
{
	dos_print("Usage: exit\nExit the system!");
	return ERRCODE_OK;
}

static ErrCode
_dos_exit_user(s8 *param, ub param_len)
{
	const char *msg = "User reboot the system!";

	if(dave_os_on_docker() == dave_true)
	{
		dos_print("In the docker environment, please do not use the exit command!");
	}
	else
	{
		dos_print(msg);

		dave_restart(msg);
	}

	return ERRCODE_OK;
}

// =====================================================================

void
dos_exit_reset(void)
{
	dos_cmd_register("restart", _dos_restart_user, (help_process_fun)_dos_restart_help);
	dos_cmd_register("exit", _dos_exit_user, (help_process_fun)_dos_exit_help);
}

#endif

