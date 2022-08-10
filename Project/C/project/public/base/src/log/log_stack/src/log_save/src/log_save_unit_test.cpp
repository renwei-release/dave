/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "log_stack.h"
#ifdef LOG_STACK_SERVER
#include <iostream>
using namespace std;
#include "gtest.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "log_save_tools.h"
#include "log_log.h"

static void
_log_save_test_cases_1(void)
{
	s8 *key_start = NULL, *key_end = NULL, *value_start = NULL, *value_end = NULL;
	s8 *content = (s8 *)"delete sd/smpp/smpp-asia/smpp-21-182 : aaaa\n";

	log_save_load_key_value(&key_start, &key_end, &value_start, &value_end, content, dave_strlen(content));

	LOGLOG("%lu->%lu %lu->%lu", key_start, key_end, value_start, value_end);

	EXPECT_STREQ(key_start, "delete sd/smpp/smpp-asia/smpp-21-182");
	EXPECT_STREQ(value_start, "aaaa");
}

// =====================================================================

TEST(log_save_case, log_save_case_1) { _log_save_test_cases_1(); }

#endif

