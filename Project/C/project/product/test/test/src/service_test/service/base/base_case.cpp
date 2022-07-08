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
#include "test_log.h"

class BaseCase:public testing::Test {
	protected:
		virtual void SetUp() {
		}
		virtual void TearDown() {
		}
};

TEST_F(BaseCase, base_case)
{
	TESTLOG("");

    EXPECT_EQ(4, 3);
}

// =====================================================================

dave_bool
base_case(s8 *input_dir, s8 *output_dir, s8 *gid, s8 *service, ThreadId id)
{
	return dave_true;
}

#endif

