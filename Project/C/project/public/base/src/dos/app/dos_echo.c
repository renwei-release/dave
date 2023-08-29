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
#include "base_tools.h"
#include "dos_tools.h"
#include "dos_show.h"
#include "dos_cmd.h"
#include "dos_log.h"

static void
_dos_echo(dave_bool echo_multiple)
{
	MsgIdEcho *pEcho = thread_reset_msg(pEcho);

	pEcho->echo_counter = 0;
	pEcho->echo_time = dave_os_time_us();
	pEcho->echo_multiple = echo_multiple;
	pEcho->concurrency_flag = dave_false;
	dave_snprintf(pEcho->msg, sizeof(pEcho->msg), "echo counter:%ld time:%ld", pEcho->echo_counter, pEcho->echo_time);

	dos_print("%s start echo (%s) ......",
		thread_name(main_thread_id_get()),
		pEcho->echo_multiple == dave_true ? "multiple" : "single");

	id_msg(main_thread_id_get(), MSGID_ECHO, pEcho);
}

static RetCode
_dos_echo_help(void)
{
	dos_print("Usage: echo [true]|[false]\nStart the echo test to test the link connection performance.!");
	return RetCode_OK;
}

static RetCode
_dos_echo_cmd(s8 *cmd_ptr, ub cmd_len)
{
	ub cmd_index = 0;
	dave_bool echo_multiple;

	cmd_index += dos_load_bool(&cmd_ptr[cmd_index], cmd_len-cmd_index, &echo_multiple);

	_dos_echo(echo_multiple);

	return RetCode_OK;
}

// =====================================================================

void
dos_echo_reset(void)
{
	dos_cmd_reg("echo", _dos_echo_cmd, _dos_echo_help);
}

#endif

