/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "dave_3rdparty.h"
#if defined(__DAVE_BASE__) && defined(GTEST_3RDPARTY)
#include "dave_base.h"
#include "dave_os.h"
#include "dave_tools.h"
#include "dos_tty.h"
#include "dos_pop.h"
#include "dos_show.h"
#include "dos_cmd.h"
#include "dos_tools.h"
#include "dos_log.h"

static RetCode
_dos_test_help(void)
{
	dos_print("Usage: test [unit]\n");
	return RetCode_OK;
}

static RetCode
_dos_test(s8 *param_ptr, ub param_len)
{
	s8 test_unit[256];

	dave_memset(test_unit, 0x00, sizeof(test_unit));

	dos_get_one_parameters(param_ptr, param_len, test_unit, sizeof(test_unit));

	return dave_gtest_run(test_unit);
}

// =====================================================================

void
dos_test_reset(void)
{
	dos_cmd_reg("test", _dos_test, _dos_test_help);
}

#endif

