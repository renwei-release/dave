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
#include "dave_os.h"
#include "dos_show.h"
#include "dos_cmd.h"
#include "dos_tools.h"
#include "dos_log.h"

static dave_bool
_dos_trace_on_off(s8 *thread_name, dave_bool on)
{
	TraceSwitchMsg *pSwitchmsg = thread_msg(pSwitchmsg);

	pSwitchmsg->thread_id = thread_id(thread_name);
	if(pSwitchmsg->thread_id != INVALID_THREAD_ID)
	{
		pSwitchmsg->trace_on = on;

		name_msg(GUARDIAN_THREAD_NAME, MSGID_TRACE_SWITCH, pSwitchmsg);
	}

	return dave_true;
}

static RetCode
_dos_show_log(s8 *param_ptr, ub param_len)
{
	u8 mac[DAVE_MAC_ADDR_LEN];
	RetCode ret = RetCode_OK;

	if(dave_strcmp(param_ptr, (s8 *)"mac"))
	{
		dave_os_load_mac(mac);
		dos_print("%s", macstr(mac));
	}

	return ret;
}

static RetCode
_dos_trace_log(s8 *cmd_ptr, ub cmd_len)
{
	ub cmd_index;
	s8 user_cmd[8];
	s8 param[64];
	dave_bool ret;

	cmd_index = 0;

	cmd_index += dos_get_one_parameters(&cmd_ptr[cmd_index], cmd_len-cmd_index, user_cmd, sizeof(user_cmd));
	dos_get_one_parameters(&cmd_ptr[cmd_index], cmd_len-cmd_index, param, sizeof(param));

	if(dave_strcmp(user_cmd, "on") == dave_true)
		ret = _dos_trace_on_off(param, dave_true);
	else if(dave_strcmp(user_cmd, "off") == dave_true)
		ret = _dos_trace_on_off(param, dave_false);
	else
		ret = dave_false;

	if(ret == dave_false)
	{
		dos_print("cmd:%s failed!", cmd_ptr);
	}

	return RetCode_OK;
}

// =====================================================================

void
dos_log_reset(void)
{
	dos_cmd_reg("log", _dos_show_log, NULL);
	dos_cmd_reg("trace", _dos_trace_log, NULL);
}

#endif


