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
#include "dos_show.h"
#include "dos_cmd.h"
#include "dos_log.h"

static ErrCode
_dos_show_support_cmd_list(s8 *param_ptr, ub param_len)
{
	dos_cmd_list_show();
	return ERRCODE_OK;
}

// =====================================================================

void
dos_ls_reset(void)
{
	dos_cmd_register("ls", _dos_show_support_cmd_list, NULL);
}

#endif

