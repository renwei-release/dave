/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_3rdparty.h"
#if defined(GTEST_3RDPARTY) && defined(JSON_3RDPARTY)
#include <iostream>
using namespace std;
#include "gtest.h"
#include "dave_3rdparty.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "json-c/json.h"
#include "tools_log.h"

static void
_json_test_cases_1(void)
{
	void *pJson, *pArray;
	ub ub_value;
	double double_value;

	pJson = dave_json_malloc();

	dave_json_add_boolean(pJson, (char *)"boolean", dave_true);
	dave_json_add_str(pJson, (char *)"str", (s8 *)"bbbbbbb");
	dave_json_add_ub(pJson, (char *)"ub", 1234567);
	dave_json_add_double(pJson, (char *)"double", 1234567.909);

	pArray = dave_json_array_malloc();
	dave_json_array_add_int(pArray, 123456);
	dave_json_add_object(pJson, (char *)"array", pArray);

	dave_json_get_ub(pJson, (char *)"ub", &ub_value);
	dave_json_get_double(pJson, (char *)"double", &double_value);

	EXPECT_EQ(ub_value, (ub)1234567);
	EXPECT_EQ(double_value, 1234567.909);

	dave_json_free(pJson);
}

static void
_json_test_cases_2(void)
{
	ub loop;

	for(loop=0; loop<10240; loop++)
	{
		_json_test_cases_1();
	}
}

// =====================================================================

TEST(json_case, json_case_1) { _json_test_cases_1(); }
TEST(json_case, json_case_2) { _json_test_cases_2(); }

#endif

