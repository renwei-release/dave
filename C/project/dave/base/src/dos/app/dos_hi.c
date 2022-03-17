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
#include "dos_welcome.h"
#include "dos_show.h"
#include "dos_cmd.h"
#include "dos_log.h"

static ErrCode
_hi_help(void)
{
	dos_print("Usage: hi\nGet the version information!");
	return ERRCODE_OK;
}

static ErrCode
_hi_show(s8 *param, ub param_len)
{
	dos_welcome_screen();
	return ERRCODE_OK;
}

// =====================================================================

void
dos_hi_reset(void)
{
	dos_cmd_register("hi", _hi_show, (help_process_fun)_hi_help);
}

#endif

