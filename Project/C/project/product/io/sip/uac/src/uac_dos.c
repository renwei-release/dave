/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_base.h"
#include "dave_tools.h"
#include "uac_cfg.h"
#include "uac_call.h"
#include "uac_reg.h"
#include "uac_log.h"

static RetCode
_uac_dos_call(s8 *param_ptr, ub param_len)
{
	s8 phone_number[128];

	dos_get_one_parameters(param_ptr, param_len, phone_number, sizeof(phone_number));

	if(phone_number[0] == '\0')
	{
		dave_strcpy(phone_number, "+8613510603952", sizeof(phone_number));
	}

	uac_call(phone_number);

	return RetCode_OK;
}

static RetCode
_uac_dos_bye(s8 *param_ptr, ub param_len)
{
	uac_bye();

	return RetCode_OK;
}

// =====================================================================

void
uac_dos_init(void)
{
	dos_cmd_reg("call", _uac_dos_call, NULL);
	dos_cmd_reg("bye", _uac_dos_bye, NULL);
}

void
uac_dos_exit(void)
{

}

