/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__) && defined(FORM_PRODUCT_BIN)
#include <iostream>
using namespace std;
#include "gtest.h"
#include "dave_base.h"
#include "dave_os.h"
#include "dave_tools.h"
#include "dos_tty.h"
#include "dos_pop.h"
#include "dos_show.h"
#include "dos_cmd.h"
#include "dos_tools.h"
#include "dos_log.h"

static const s8 * _input_dir = "/project/test/test_uint/input";
static const s8 * _output_dir = "/project/test/test_uint/output";

static RetCode
_dos_test_help(void)
{
	dos_print("Usage: test [unit]\n");
	return RetCode_OK;
}

static RetCode
_dos_test_run(s8 *test_unit)
{
	s8 output_file[1024];
	s8 test_case[1024];
	int ret;

	testing::InitGoogleTest();

	dave_snprintf(output_file, sizeof(output_file),
		"xml:%s/service_%s.xml",
		_output_dir,
		test_unit);
	testing::GTEST_FLAG(output) = output_file;

	if(test_unit[0] != '\0')
	{
		dave_snprintf(test_case, sizeof(test_case),
			"%s_case.*", test_unit);
		testing::GTEST_FLAG(filter) = test_case;
	}

	ret = RUN_ALL_TESTS();
	if(ret != 0)
	{
		dos_print("input_dir:%s output_dir:%s run test case %s failed! ret:%d",
			_input_dir, _output_dir,
			test_unit[0] != '\0' ? test_unit : "all",
			ret);
	}

	return RetCode_OK;
}

static RetCode
_dos_test(s8 *param_ptr, ub param_len)
{
	s8 test_unit[256];

	dave_memset(test_unit, 0x00, sizeof(test_unit));

	dos_get_one_parameters(param_ptr, param_len, test_unit, sizeof(test_unit));

	return _dos_test_run(test_unit);
}

// =====================================================================

extern "C" void
dos_test_reset(void)
{
	dos_cmd_reg("test", _dos_test, _dos_test_help);
}

#endif

