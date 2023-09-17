/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_base.h"
#include "dave_os.h"
#include "dave_tools.h"
#include "base_tools.h"
#include "dos_tools.h"
#include "dos_show.h"
#include "dos_cmd.h"
#include "dos_log.h"

static RetCode
_dos_echo(s8 *command)
{
	MsgIdEchoReq *pEcho = thread_reset_msg(pEcho);

	if((dave_strcmp(command, "concurrent") == dave_true)
		|| (dave_strcmp(command, "c") == dave_true))
	{
		pEcho->echo.type = EchoType_start;
		pEcho->echo.concurrent_flag = dave_true;
	}
	else if((dave_strcmp(command, "single") == dave_true)
		|| (dave_strcmp(command, "s") == dave_true))
	{
		pEcho->echo.type = EchoType_start;
		pEcho->echo.concurrent_flag = dave_false;
	}
	else if(dave_strcmp(command, "stop") == dave_true)
	{
		pEcho->echo.type = EchoType_stop;
	}
	else
	{
		return RetCode_Invalid_parameter;
	}

	dos_print("%s %s echo",
		thread_name(main_thread_id_get()),
		pEcho->echo.type == EchoType_start ? "start" : "stop");

	id_msg(main_thread_id_get(), MSGID_ECHO_REQ, pEcho);

	return RetCode_OK;
}

static RetCode
_dos_echo_help(void)
{
	dos_print("Usage: echo [concurrent]|[single]|[stop]\nStart the echo test to test the link connection performance.");
	return RetCode_OK;
}

static RetCode
_dos_echo_cmd(s8 *cmd_ptr, ub cmd_len)
{
	ub cmd_index = 0;
	s8 command[128];

	cmd_index += dos_load_string(&cmd_ptr[cmd_index], cmd_len-cmd_index, command, sizeof(command));

	return _dos_echo(command);
}

// =====================================================================

void
dos_echo_reset(void)
{
	dos_cmd_reg("echo", _dos_echo_cmd, _dos_echo_help);
}

#endif

