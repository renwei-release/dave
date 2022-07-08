/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#ifdef __DAVE_PRODUCT_TEST__
#include <iostream>
using namespace std;
#include "gtest.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "test_log.h"

static const s8 * input_dir = "/project/product/test/test/service_test/input";
static const s8 * output_dir = "/project/product/test/test/service_test/output";

// =====================================================================

void
service_test(s8 *globally_identifier, s8 *remote_thread_name, ThreadId remote_thread_id)
{
	int argc = 0;
	char *argv[1] = {
		(char *)"null",
	};
	s8 output_file[1024];
	s8 test_case[1024];
	int ret;

	lower(remote_thread_name);

	testing::InitGoogleTest(&argc, argv);

	dave_snprintf(output_file, sizeof(output_file),
		"xml:%s/service_%s_%s.xml",
		output_dir,
		globally_identifier, remote_thread_name);
	testing::GTEST_FLAG(output) = output_file;

	dave_snprintf(test_case, sizeof(test_case),
		"BaseCase", remote_thread_name);
	testing::GTEST_FLAG(filter) = test_case;

	testing::InitGoogleTest();

	ret = RUN_ALL_TESTS();
	if(ret != 0)
	{
		TESTABNOR("gid:%s thread:%s id:%lx run test case failed! ret:%d",
			globally_identifier, remote_thread_name, remote_thread_id,
			ret);
	}
}

#endif

