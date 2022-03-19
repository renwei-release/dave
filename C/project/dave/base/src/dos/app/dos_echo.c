/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_base.h"
#include "dave_tools.h"
#include "dos_tools.h"
#include "dos_show.h"
#include "dos_cmd.h"
#include "dos_log.h"

static void
_dos_echo(dave_bool echo_multiple, s8 *thread_name)
{
	ThreadId echo_id = thread_id(thread_name);
	MsgIdEcho *pEcho;

	if(echo_id == INVALID_THREAD_ID)
	{
		dos_print("can't find %s", thread_name);
		return;
	}

	pEcho = thread_reset_msg(pEcho);
	pEcho->echo_counter = 0;
	pEcho->echo_time = dave_os_time_us();
	pEcho->echo_multiple = echo_multiple;
	pEcho->concurrency_flag = dave_false;
	dave_snprintf(pEcho->msg, sizeof(pEcho->msg), "echo counter:%ld time:%ld", pEcho->echo_counter, pEcho->echo_time);

	dos_print("%s start echo (%s) ......",
		thread_name,
		pEcho->echo_multiple == dave_true ? "multiple" : "single");

	write_msg(echo_id, MSGID_ECHO, pEcho);
}

static ErrCode
_dos_echo_help(void)
{
	dos_print("Usage: echo [true]|[false] [thread name]\nStart the echo test to test the link connection performance.!");
	return ERRCODE_OK;
}

static ErrCode
_dos_echo_cmd(s8 *cmd_ptr, ub cmd_len)
{
	ub cmd_index = 0;
	dave_bool echo_multiple;
	s8 thread_name[128];

	cmd_index += dos_get_bool(&cmd_ptr[cmd_index], cmd_len-cmd_index, &echo_multiple);
	dos_get_str(&cmd_ptr[cmd_index], cmd_len-cmd_index, thread_name, sizeof(thread_name));

	if(thread_name[0] == '\0')
		return ERRCODE_Invalid_parameter;

	_dos_echo(echo_multiple, thread_name);

	return ERRCODE_OK;
}

// =====================================================================

void
dos_echo_reset(void)
{
	dos_cmd_register("echo", _dos_echo_cmd, _dos_echo_help);
}

#endif

