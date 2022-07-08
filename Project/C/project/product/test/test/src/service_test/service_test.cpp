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

static const s8 * _input_dir = "/project/product/test/test/service_test/input";
static const s8 * _output_dir = "/project/product/test/test/service_test/output";

static const s8 *_test_service_table[] {
	"base",
	NULL
} ;

static dave_bool
_test_service_enable(s8 *gid, s8 *service, ThreadId id)
{
	ub table_index;

	for(table_index=0; table_index<40960; table_index++)
	{
		if(_test_service_table[table_index] == NULL)
		{
			break;
		}

		if(dave_strcmp(_test_service_table[table_index], service) == dave_true)
		{
			return dave_true;
		}
	}

	return dave_false;
}

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

	if(_test_service_enable(globally_identifier, remote_thread_name, remote_thread_id) == dave_false)
	{
		return;
	}

	testing::InitGoogleTest(&argc, argv);

	dave_snprintf(output_file, sizeof(output_file),
		"xml:%s/service_%s_%s.xml",
		_output_dir,
		globally_identifier, remote_thread_name);
	testing::GTEST_FLAG(output) = output_file;

	dave_snprintf(test_case, sizeof(test_case),
		"%s_case.*", remote_thread_name);
	testing::GTEST_FLAG(filter) = test_case;

	testing::InitGoogleTest();

	ret = RUN_ALL_TESTS();
	if(ret != 0)
	{
		TESTABNOR("input_dir:%s output_dir:%s gid:%s service:%s id:%lx run test case failed! ret:%d",
			_input_dir, _output_dir,
			globally_identifier, remote_thread_name, remote_thread_id,
			ret);
	}
}

#endif

