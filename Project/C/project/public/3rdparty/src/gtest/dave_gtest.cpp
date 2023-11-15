/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#if defined(GTEST_3RDPARTY)
#include <iostream>
using namespace std;
#include "gtest.h"
#include "dave_base.h"
#include "dave_tools.h"

static const s8 * _input_dir = "/project/test/test_uint/input";
static const s8 * _output_dir = "/project/test/test_uint/output";

static RetCode
_gtest_run(s8 *test_unit)
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

// =====================================================================

extern "C" RetCode
dave_gtest_run(s8 *test_unit)
{
	return _gtest_run(test_unit);
}

#endif

