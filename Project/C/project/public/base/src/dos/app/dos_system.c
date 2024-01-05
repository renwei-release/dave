/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_verno.h"
#include "dos_show.h"
#include "dos_cmd.h"
#include "dos_tools.h"
#include "dos_log.h"

static RetCode
_dos_system_online(s8 *cmd_ptr, ub cmd_len)
{
	ApplicationIdle *pIdle = thread_reset_msg(pIdle);

	pIdle->cfg_flag = dave_true;
	
	name_msg(GUARDIAN_THREAD_NAME, MSGID_APPLICATION_IDLE, pIdle);

	dos_print("application on idle");

	return RetCode_OK;
}

static RetCode
_dos_system_offline(s8 *cmd_ptr, ub cmd_len)
{
	ApplicationBusy *pBusy = thread_reset_msg(pBusy);

	pBusy->cfg_flag = dave_true;

	name_msg(GUARDIAN_THREAD_NAME, MSGID_APPLICATION_BUSY, pBusy);

	dos_print("application on busy");

	return RetCode_OK;
}

// =====================================================================

void
dos_system_reset(void)
{
	dos_cmd_reg("online", _dos_system_online, NULL);
	dos_cmd_reg("offline", _dos_system_offline, NULL);
}

#endif

