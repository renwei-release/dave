/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef FORM_PRODUCT_BIN
#include <iostream>
using namespace std;
#include "gtest.h"
#include "dave_3rdparty.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "json.h"
#include "tools_log.h"

static void
_t_bson_test_cases_1(void)
{
	void *pBson;

	pBson = t_bson_malloc_object();

	t_bson_add_boolean(pBson, (char *)"bool_false", false);
	t_bson_add_boolean(pBson, (char *)"bool_true", true);
	t_bson_add_int(pBson, (char *)"int", 123);
	t_bson_add_double(pBson, (char *)"double", 123.7899);
	t_bson_add_string(pBson, (char *)"string", (char *)"test_cases_1");
	t_bson_add_bin(pBson, (char *)"bin", (char *)pBson, 12);

	t_bson_free_object(pBson);
}

static void
_t_bson_test_cases_2(void)
{
	void *pBson;
	bool bool_value;
	int int_value;
	double double_value;
	size_t string_len;
	char *string_value;
	size_t bin_len;
	char *bin_value;

	pBson = t_bson_malloc_object();

	t_bson_add_boolean(pBson, (char *)"bool_false", false);
	if(t_bson_inq_boolean(pBson, (char *)"bool_false", &bool_value) == false)
		TOOLSLOG("inq failed %d", bool_value);
	EXPECT_EQ(bool_value, false);
	t_bson_add_boolean(pBson, (char *)"bool_true", true);
	if(t_bson_inq_boolean(pBson, (char *)"bool_true", &bool_value) == false)
		TOOLSLOG("inq failed %d", bool_value);
	EXPECT_EQ(bool_value, true);

	t_bson_add_int(pBson, (char *)"int", 123);
	if(t_bson_inq_int(pBson, (char *)"int", &int_value) == false)
		TOOLSLOG("inq failed %d", int_value);
	EXPECT_EQ(int_value, 123);

	t_bson_add_double(pBson, (char *)"double", 123.7899);
	if(t_bson_inq_double(pBson, (char *)"double", &double_value) == false)
		TOOLSLOG("inq failed %d", double_value);
	EXPECT_EQ(double_value, 123.7899);

	t_bson_add_string(pBson, (char *)"string", (char *)"test_cases_2");
	if(t_bson_inq_string(pBson, (char *)"string", &string_value, &string_len) == false)
		TOOLSLOG("inq failed %s", string_value);
	EXPECT_STREQ(string_value, "test_cases_2");

	t_bson_add_bin(pBson, (char *)"bin", (char *)pBson, 12);
	if(t_bson_inq_bin(pBson, (char *)"bin", &bin_value, &bin_len) == false)
		TOOLSLOG("inq failed %d", bin_len);
	if((bin_len != 12) || (dave_memcmp(bin_value, pBson, bin_len) == dave_false))
		TOOLSLOG("inq failed %d", bin_len);

	t_bson_free_object(pBson);
}

static void
_t_bson_test_cases_3(void)
{
	void *pBson, *pArray, *pInqArray;

	pBson = t_bson_malloc_object();

	pArray = t_bson_malloc_array();

	t_bson_array_add_boolean(pArray, false);
	t_bson_array_add_double(pArray, 12.3456);
	t_bson_array_add_string(pArray, (char *)"string");

	t_bson_add_object(pBson, (char *)"array", pArray);
	pInqArray = t_bson_inq_object(pBson, (char *)"array");
	EXPECT_EQ(pInqArray, pArray);

	t_bson_free_object(pBson);
}

static void
_t_bson_test_cases_4(void)
{
	void *pBson, *pArray, *pInqArray, *pObject, *pInqObject;

	pBson = t_bson_malloc_object();

	pArray = t_bson_malloc_array();
	t_bson_array_add_boolean(pArray, false);
	t_bson_array_add_double(pArray, 12.3456);
	t_bson_array_add_string(pArray, (char *)"string");

	t_bson_add_object(pBson, (char *)"array", pArray);
	pInqArray = t_bson_inq_object(pBson, (char *)"array");
	EXPECT_EQ(pInqArray, pArray);

	pObject = t_bson_malloc_object();
	t_bson_add_boolean(pObject, (char *)"false", false);
	t_bson_add_double(pObject, (char *)"12", 12.3456);
	t_bson_add_string(pObject, (char *)"string", (char *)"string");

	t_bson_add_object(pBson, (char *)"object", pObject);
	pInqObject = t_bson_inq_object(pBson, (char *)"object");
	EXPECT_EQ(pInqObject, pObject);

	t_bson_free_object(pBson);
}

static void
_t_bson_test_cases_5(void)
{
	void *pBson;
	bool bool_value;

	pBson = t_bson_malloc_object();

	t_bson_add_boolean(pBson, (char *)"bool_value", true);
	if(t_bson_inq_boolean(pBson, (char *)"bool_value", &bool_value) == false)
		TOOLSLOG("inq failed %d", bool_value);
	EXPECT_EQ(bool_value, true);

	t_bson_add_boolean(pBson, (char *)"bool_value", false);
	if(t_bson_inq_boolean(pBson, (char *)"bool_value", &bool_value) == false)
		TOOLSLOG("inq failed %d", bool_value);
	EXPECT_EQ(bool_value, false);

	t_bson_add_boolean(pBson, (char *)"bool_value", true);
	if(t_bson_inq_boolean(pBson, (char *)"bool_value", &bool_value) == false)
		TOOLSLOG("inq failed %d", bool_value);
	EXPECT_EQ(bool_value, true);

	t_bson_add_double(pBson, (char *)"12", 12.3456);
	t_bson_add_double(pBson, (char *)"12", 11111);

	t_bson_free_object(pBson);
}

static void
_t_bson_test_cases_6(void)
{
	void *pBson, *pArray, *pObject, *pInqArray;

	pBson = t_bson_malloc_object();

	pArray = t_bson_malloc_array();
	t_bson_array_add_boolean(pArray, false);
	t_bson_array_add_double(pArray, 12.3456);
	t_bson_array_add_string(pArray, (char *)"string");

	pObject = t_bson_malloc_object();
	t_bson_add_boolean(pObject, (char *)"false", false);
	t_bson_add_double(pObject, (char *)"12", 12.3456);
	t_bson_add_string(pObject, (char *)"string", (char *)"string");

	t_bson_array_add_object(pArray, pObject);

	t_bson_add_object(pBson, (char *)"array", pArray);
	pInqArray = t_bson_inq_object(pBson, (char *)"array");
	EXPECT_EQ(pInqArray, pArray);

	t_bson_free_object(pBson);
}

static void
_t_bson_test_cases_7(void)
{
	void *pBson, *pSBson;
	char *serialize_ptr;
	size_t serialize_len;
	bool bool_value;
	char *string_value;
	size_t string_len;

	pBson = t_bson_malloc_object();

	t_bson_add_boolean(pBson, (char *)"false", false);
	t_bson_add_double(pBson, (char *)"12", 12.3456);
	t_bson_add_string(pBson, (char *)"string", (char *)"string");

	serialize_ptr = t_bson_to_serialize(pBson, &serialize_len);
	pSBson = t_serialize_to_bson(serialize_ptr, serialize_len);
	if(pSBson == NULL)
	{
		TOOLSLOG("pSBson is NULL!");
	}
	else
	{
		if(t_bson_inq_boolean(pSBson, (char *)"false", &bool_value) == false)
			TOOLSLOG("inq failed %d", bool_value);
		EXPECT_EQ(bool_value, false);
		if(t_bson_inq_string(pBson, (char *)"string", &string_value, &string_len) == false)
			TOOLSLOG("inq failed %s", string_value);
		EXPECT_STREQ(string_value, "string");

		t_bson_free_object(pSBson);
	}

	t_bson_free_object(pBson);
}

static void
_t_bson_test_cases_8(void)
{
	void *pBson, *pArray, *pSBson, *pSArray;
	char *serialize_ptr;
	size_t serialize_len;
	bool bool_value;
	double double_value;
	char *string_value;
	size_t string_len;

	pBson = t_bson_malloc_object();

	t_bson_add_boolean(pBson, (char *)"false", false);
	t_bson_add_double(pBson, (char *)"12", 12.3456);
	t_bson_add_string(pBson, (char *)"string", (char *)"string");

	pArray = t_bson_malloc_array();
	t_bson_array_add_boolean(pArray, true);
	t_bson_array_add_double(pArray, 12.3456);
	t_bson_array_add_string(pArray, (char *)"string");

	t_bson_add_object(pBson, (char *)"array", pArray);

	serialize_ptr = t_bson_to_serialize(pBson, &serialize_len);
	pSBson = t_serialize_to_bson(serialize_ptr, serialize_len);
	if(pSBson == NULL)
	{
		TOOLSLOG("pSBson is NULL!");
	}
	else
	{
		if(t_bson_inq_boolean(pSBson, (char *)"false", &bool_value) == false)
			TOOLSLOG("inq failed %d", bool_value);
		EXPECT_EQ(bool_value, false);
		if(t_bson_inq_string(pBson, (char *)"string", &string_value, &string_len) == false)
			TOOLSLOG("inq failed %s", string_value);
		EXPECT_STREQ(string_value, "string");

		pSArray = t_bson_inq_object(pBson, (char *)"array");
		if(pSArray == NULL)
		{
			TOOLSLOG("inq array failed!");
		}
		else
		{
			if(t_bson_array_inq_boolean(pSArray, 0, &bool_value) == false)
				TOOLSLOG("inq array bool value failed");
			EXPECT_EQ(bool_value, true);
			if(t_bson_array_inq_double(pSArray, 1, &double_value) == false)
				TOOLSLOG("inq array double value failed");
			EXPECT_EQ(double_value, 12.3456);
			if(t_bson_array_inq_string(pSArray, 2, &string_value, &string_len) == false)
				TOOLSLOG("inq array string value failed");
			EXPECT_STREQ(string_value, "string");
		}

		t_bson_free_object(pSBson);
	}

	t_bson_free_object(pBson);
}

static void
_t_bson_test_cases_9(void)
{
	void *pBson, *pArray, *pJson;

	pBson = t_bson_malloc_object();

	t_bson_add_boolean(pBson, (char *)"false", false);
	t_bson_add_double(pBson, (char *)"12", 12.3456);
	t_bson_add_string(pBson, (char *)"string", (char *)"string");
	t_bson_add_bin(pBson, (char *)"bin", (char *)pBson, sizeof(pBson));

	pArray = t_bson_malloc_array();
	t_bson_array_add_boolean(pArray, true);
	t_bson_array_add_double(pArray, 12.3456);
	t_bson_array_add_string(pArray, (char *)"string");

	t_bson_add_object(pBson, (char *)"array", pArray);

	pJson = t_bson_to_json(pBson);

	t_bson_free_object(pBson);

	json_object_put((struct json_object *)pJson);
}

static void
_t_bson_test_cases_10(void)
{
	void *pBson;
	MBUF *mbuf_data;

	pBson = t_bson_malloc_object();

	t_bson_add_boolean(pBson, (char *)"false", false);
	t_bson_add_double(pBson, (char *)"12", 12.3456);
	t_bson_add_string(pBson, (char *)"string", (char *)"string");
	t_bson_add_bin(pBson, (char *)"bin", (char *)pBson, sizeof(pBson));

	mbuf_data = t_bson_to_mbuf(pBson);
	t_bson_free_object(t_mbuf_to_bson(mbuf_data));
	dave_mfree(mbuf_data);

	t_bson_free_object((struct json_object *)pBson);
}

static void
_t_bson_test_cases_11(void)
{
	void *pBson, *pJson;

	pBson = t_bson_malloc_object();
	t_bson_add_boolean(pBson, (char *)"false", false);
	t_bson_add_double(pBson, (char *)"12", 12.3456);
	t_bson_add_string(pBson, (char *)"string", (char *)"string");
	t_bson_free_object(pBson);

	pJson = json_object_new_object();
	json_object_object_add((struct json_object *)pJson, (char *)"false", json_object_new_boolean((json_bool)false));
	json_object_object_add((struct json_object *)pJson, (char *)"12", json_object_new_double(12.3456));
	json_object_object_add((struct json_object *)pJson, (char *)"string", json_object_new_string("string"));

	json_object_put((struct json_object *)pJson);
}

static void
_t_bson_test_cases_12(void)
{
	void *pJson, *pArray, *pBson;
	dave_bool bool_value;
	double double_value;

	pJson = dave_json_malloc();
	dave_json_add_boolean(pJson, (char *)"boolean", dave_true);
	dave_json_add_str(pJson, (char *)"str", (s8 *)"bbbbbbb");
	dave_json_add_ub(pJson, (char *)"ub", 1234567);
	dave_json_add_double(pJson, (char *)"double", 1234567.909);

	pArray = dave_json_array_malloc();
	dave_json_array_add_int(pArray, 123456);
	dave_json_add_object(pJson, (char *)"array", pArray);

	pBson = t_json_to_bson(pJson);
	dave_json_free(pJson);

	pJson = t_bson_to_json(pBson);
	t_bson_free_object(pBson);

	dave_json_get_boolean(pJson, (char *)"boolean", &bool_value);
	EXPECT_EQ(bool_value, dave_true);
	dave_json_get_double(pJson, (char *)"double", &double_value);
	EXPECT_EQ(double_value, 1234567.909);

	dave_json_free(pJson);
}

// =====================================================================

TEST(bson_case, bson_case_1) { _t_bson_test_cases_1(); }
TEST(bson_case, bson_case_2) { _t_bson_test_cases_2(); }
TEST(bson_case, bson_case_3) { _t_bson_test_cases_3(); }
TEST(bson_case, bson_case_4) { _t_bson_test_cases_4(); }
TEST(bson_case, bson_case_5) { _t_bson_test_cases_5(); }
TEST(bson_case, bson_case_6) { _t_bson_test_cases_6(); }
TEST(bson_case, bson_case_7) { _t_bson_test_cases_7(); }
TEST(bson_case, bson_case_8) { _t_bson_test_cases_8(); }
TEST(bson_case, bson_case_9) { _t_bson_test_cases_9(); }
TEST(bson_case, bson_case_10) { _t_bson_test_cases_10(); }
TEST(bson_case, bson_case_11) { _t_bson_test_cases_11(); }
TEST(bson_case, bson_case_12) { _t_bson_test_cases_12(); }

#endif

