/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_3rdparty.h"
#if defined(GTEST_3RDPARTY)
#include <iostream>
using namespace std;
#include "gtest.h"
#include "dave_3rdparty.h"
#include "base_log.h"
#include "log_buffer.h"

static void
_log_test_cases_1(void)
{
	DAVELOG("_log_test_cases_1\n");
	DAVELOG("_log_test_cases_1:%s\n", "_log_test_cases_1");
}

static void
_log_test_cases_2(void)
{
	ub loop;

	for(loop=0; loop<LOG_BUFFER_MAX/2; loop++)
	{
		DAVELOG("_log_test_cases_2:%d\n", loop);
	}
}

static void
_log_test_cases_3(void)
{
	const char *printf_data = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

	DAVELOG("_log_test_cases_2:%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s\n",
		printf_data, printf_data, printf_data,
		printf_data, printf_data, printf_data,
		printf_data, printf_data, printf_data,
		printf_data, printf_data, printf_data,
		printf_data, printf_data, printf_data,
		printf_data, printf_data, printf_data);
}

static void
_log_test_cases_4(void)
{
	ub loop;

	for(loop=0; loop<LOG_BUFFER_MAX + 10; loop++)
	{
		DAVELOG("_log_test_cases_4:%d\n", loop);
	}
}

// =====================================================================

TEST(log_case, log_case_1) { _log_test_cases_1(); }
TEST(log_case, log_case_2) { _log_test_cases_2(); }
TEST(log_case, log_case_3) { _log_test_cases_3(); }
TEST(log_case, log_case_4) { _log_test_cases_4(); }

#endif

