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
	s8 *key_ptr = NULL, *value_ptr = NULL;
	ub key_len, value_len;
	s8 content[256] = { "delete sd/smpp/smpp-asia/smpp-21-182 : aaaa\n" };

	log_save_load_key_value(&key_ptr, &key_len, &value_ptr, &value_len, content, dave_strlen(content));

	if((key_ptr == NULL) || (value_ptr == NULL))
	{
		GTEST_FAIL();
		return;
	}

	key_ptr[key_len] = '\0';
	value_ptr[value_len] = '\0';

	EXPECT_STREQ(key_ptr, "sd/smpp/smpp-asia/smpp-21-182");
	EXPECT_STREQ(value_ptr, "aaaa");
}

static void
_log_save_test_cases_2(void)
{
	s8 *key_ptr = NULL, *value_ptr = NULL;
	ub key_len, value_len;
	s8 content[256] = { "aaaa : aaaa jdhdhhd:2334\n" };

	log_save_load_key_value(&key_ptr, &key_len, &value_ptr, &value_len, content, dave_strlen(content));

	if((key_ptr == NULL) || (value_ptr == NULL))
	{
		GTEST_FAIL();
		return;
	}

	key_ptr[key_len] = '\0';
	value_ptr[value_len] = '\0';

	EXPECT_STREQ(key_ptr, "aaaa");
	EXPECT_STREQ(value_ptr, "aaaa");
}

static void
_log_save_test_cases_3(void)
{
	s8 *key_ptr = NULL, *value_ptr = NULL;
	ub key_len, value_len;
	s8 content_ptr[256] = { "5^^7:2334 fun:__fun_call__ line:1123\n" };
	ub content_index;
	s8 backup_value;

	content_index = 0;

	content_index += log_save_load_key_value(&key_ptr, &key_len, &value_ptr, &value_len, &content_ptr[content_index], dave_strlen(content_ptr)-content_index);
	if((key_ptr == NULL) || (value_ptr == NULL))
	{
		GTEST_FAIL();
		return;
	}
	key_ptr[key_len] = '\0';
	backup_value = value_ptr[value_len]; value_ptr[value_len] = '\0';
	EXPECT_STREQ(key_ptr, "5^^7");
	EXPECT_STREQ(value_ptr, "2334");
	value_ptr[value_len] = backup_value;

	content_index += log_save_load_key_value(&key_ptr, &key_len, &value_ptr, &value_len, &content_ptr[content_index], dave_strlen(content_ptr)-content_index);
	if((key_ptr == NULL) || (value_ptr == NULL))
	{
		GTEST_FAIL();
		return;
	}
	key_ptr[key_len] = '\0';
	backup_value = value_ptr[value_len]; value_ptr[value_len] = '\0';
	EXPECT_STREQ(key_ptr, "fun");
	EXPECT_STREQ(value_ptr, "__fun_call__");
	value_ptr[value_len] = backup_value;

	content_index += log_save_load_key_value(&key_ptr, &key_len, &value_ptr, &value_len, &content_ptr[content_index], dave_strlen(content_ptr)-content_index);
	if((key_ptr == NULL) || (value_ptr == NULL))
	{
		GTEST_FAIL();
		return;
	}
	key_ptr[key_len] = '\0';
	backup_value = value_ptr[value_len]; value_ptr[value_len] = '\0';
	EXPECT_STREQ(key_ptr, "line");
	EXPECT_STREQ(value_ptr, "1123");
	value_ptr[value_len] = backup_value;
}

static void
_log_save_test_cases_4(void)
{
	s8 *key_ptr = NULL, *value_ptr = NULL;
	ub key_len, value_len;
	s8 content_ptr[256] = { "5^^7:2334 fun:__fun_c:all__ line:1123\n" };
	ub content_index;
	s8 backup_value;

	content_index = 0;

	content_index += log_save_load_key_value(&key_ptr, &key_len, &value_ptr, &value_len, &content_ptr[content_index], dave_strlen(content_ptr)-content_index);
	if((key_ptr == NULL) || (value_ptr == NULL))
	{
		GTEST_FAIL();
		return;
	}
	key_ptr[key_len] = '\0';
	backup_value = value_ptr[value_len]; value_ptr[value_len] = '\0';
	EXPECT_STREQ(key_ptr, "5^^7");
	EXPECT_STREQ(value_ptr, "2334");
	value_ptr[value_len] = backup_value;

	content_index += log_save_load_key_value(&key_ptr, &key_len, &value_ptr, &value_len, &content_ptr[content_index], dave_strlen(content_ptr)-content_index);
	if((key_ptr == NULL) || (value_ptr == NULL))
	{
		GTEST_FAIL();
		return;
	}
	key_ptr[key_len] = '\0';
	backup_value = value_ptr[value_len]; value_ptr[value_len] = '\0';
	EXPECT_STREQ(key_ptr, "fun");
	EXPECT_STREQ(value_ptr, "__fun_c:all__");
	value_ptr[value_len] = backup_value;

	content_index += log_save_load_key_value(&key_ptr, &key_len, &value_ptr, &value_len, &content_ptr[content_index], dave_strlen(content_ptr)-content_index);
	if((key_ptr == NULL) || (value_ptr == NULL))
	{
		GTEST_FAIL();
		return;
	}
	key_ptr[key_len] = '\0';
	backup_value = value_ptr[value_len]; value_ptr[value_len] = '\0';
	EXPECT_STREQ(key_ptr, "line");
	EXPECT_STREQ(value_ptr, "1123");
	value_ptr[value_len] = backup_value;
}

static void
_log_save_test_cases_5(void)
{
	s8 *key_ptr = NULL, *value_ptr = NULL;
	ub key_len, value_len;
	s8 content_ptr[256] = { "5^^7:2334 fun:__fun_c: all__ line:1123\n" };
	ub content_index;
	s8 backup_value;

	content_index = 0;

	content_index += log_save_load_key_value(&key_ptr, &key_len, &value_ptr, &value_len, &content_ptr[content_index], dave_strlen(content_ptr)-content_index);
	if((key_ptr == NULL) || (value_ptr == NULL))
	{
		GTEST_FAIL();
		return;
	}
	key_ptr[key_len] = '\0';
	backup_value = value_ptr[value_len]; value_ptr[value_len] = '\0';
	EXPECT_STREQ(key_ptr, "5^^7");
	EXPECT_STREQ(value_ptr, "2334");
	value_ptr[value_len] = backup_value;

	content_index += log_save_load_key_value(&key_ptr, &key_len, &value_ptr, &value_len, &content_ptr[content_index], dave_strlen(content_ptr)-content_index);
	if((key_ptr == NULL) || (value_ptr == NULL))
	{
		GTEST_FAIL();
		return;
	}
	key_ptr[key_len] = '\0';
	backup_value = value_ptr[value_len]; value_ptr[value_len] = '\0';
	EXPECT_STREQ(key_ptr, "fun");
	EXPECT_STREQ(value_ptr, "__fun_c:");
	value_ptr[value_len] = backup_value;

	content_index += log_save_load_key_value(&key_ptr, &key_len, &value_ptr, &value_len, &content_ptr[content_index], dave_strlen(content_ptr)-content_index);
	if((key_ptr == NULL) || (value_ptr == NULL))
	{
		GTEST_FAIL();
		return;
	}
	key_ptr[key_len] = '\0';
	backup_value = value_ptr[value_len]; value_ptr[value_len] = '\0';
	EXPECT_STREQ(key_ptr, "line");
	EXPECT_STREQ(value_ptr, "1123");
	value_ptr[value_len] = backup_value;
}

// =====================================================================

TEST(log_save_case, log_save_case_1) { _log_save_test_cases_1(); }
TEST(log_save_case, log_save_case_2) { _log_save_test_cases_2(); }
TEST(log_save_case, log_save_case_3) { _log_save_test_cases_3(); }
TEST(log_save_case, log_save_case_4) { _log_save_test_cases_4(); }
TEST(log_save_case, log_save_case_5) { _log_save_test_cases_5(); }

#endif

