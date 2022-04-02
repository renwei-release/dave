/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_tools.h"
#include "dave_os.h"
#include "json.h"
#include "tools_log.h"

static void
_t_bson_test_cases_1(void)
{
	ub start_test_time = dave_os_time_us();
	void *pBson;

	pBson = t_bson_malloc_object();

	t_bson_add_boolean(pBson, "bool_false", false);
	t_bson_add_boolean(pBson, "bool_true", true);
	t_bson_add_int(pBson, "int", 123);
	t_bson_add_double(pBson, "double", 123.7899);
	t_bson_add_string(pBson, "string", "test_cases_1");
	t_bson_add_bin(pBson, "bin", (char *)pBson, 12);
	t_bson_free_object(pBson);

	TOOLSLOG("Time-consuming:%dus", dave_os_time_us() - start_test_time);
}

static void
_t_bson_test_cases_2(void)
{
	ub start_test_time = dave_os_time_us();
	void *pBson;
	bool bool_value;
	int int_value;
	double double_value;
	size_t string_len;
	char *string_value;
	size_t bin_len;
	char *bin_value;

	pBson = t_bson_malloc_object();

	t_bson_add_boolean(pBson, "bool_false", false);
	if(t_bson_inq_boolean(pBson, "bool_false", &bool_value) == false)
		TOOLSLOG("inq failed %d", bool_value);
	if(bool_value != false)
		TOOLSLOG("inq failed %d", bool_value);
	t_bson_add_boolean(pBson, "bool_true", true);
	if(t_bson_inq_boolean(pBson, "bool_true", &bool_value) == false)
		TOOLSLOG("inq failed %d", bool_value);
	if(bool_value != true)
		TOOLSLOG("inq failed %d", bool_value);

	t_bson_add_int(pBson, "int", 123);
	if(t_bson_inq_int(pBson, "int", &int_value) == false)
		TOOLSLOG("inq failed %d", int_value);
	if(int_value != 123)
		TOOLSLOG("inq failed %d", int_value);

	t_bson_add_double(pBson, "double", 123.7899);
	if(t_bson_inq_double(pBson, "double", &double_value) == false)
		TOOLSLOG("inq failed %d", double_value);
	if(double_value != 123.7899)
		TOOLSLOG("inq failed %f", double_value);

	t_bson_add_string(pBson, "string", "test_cases_2");
	if(t_bson_inq_string(pBson, "string", &string_value, &string_len) == false)
		TOOLSLOG("inq failed %s", string_value);
	if(dave_strcmp(string_value, "test_cases_2") == dave_false)
		TOOLSLOG("inq failed %s", string_value);

	t_bson_add_bin(pBson, "bin", (char *)pBson, 12);
	if(t_bson_inq_bin(pBson, "bin", &bin_value, &bin_len) == false)
		TOOLSLOG("inq failed %d", bin_len);
	if((bin_len != 12) || (dave_memcmp(bin_value, pBson, bin_len) == dave_false))
		TOOLSLOG("inq failed %d", bin_len);

	t_bson_free_object(pBson);

	TOOLSLOG("Time-consuming:%dus", dave_os_time_us() - start_test_time);
}

static void
_t_bson_test_cases_3(void)
{
	ub start_test_time = dave_os_time_us();
	void *pBson, *pArray, *pInqArray;

	pBson = t_bson_malloc_object();

	pArray = t_bson_malloc_array();

	t_bson_array_add_boolean(pArray, false);
	t_bson_array_add_double(pArray, 12.3456);
	t_bson_array_add_string(pArray, "string");

	t_bson_add_object(pBson, "array", pArray);
	pInqArray = t_bson_inq_object(pBson, "array");
	if(pInqArray == NULL)
		TOOLSLOG("inq failed %x", pInqArray);
	if(pInqArray != pArray)
		TOOLSLOG("inq failed %x/%x", pInqArray, pArray);

	t_bson_free_object(pBson);

	TOOLSLOG("Time-consuming:%dus", dave_os_time_us() - start_test_time);
}

static void
_t_bson_test_cases_4(void)
{
	ub start_test_time = dave_os_time_us();
	void *pBson, *pArray, *pInqArray, *pObject, *pInqObject;

	pBson = t_bson_malloc_object();

	pArray = t_bson_malloc_array();
	t_bson_array_add_boolean(pArray, false);
	t_bson_array_add_double(pArray, 12.3456);
	t_bson_array_add_string(pArray, "string");

	t_bson_add_object(pBson, "array", pArray);
	pInqArray = t_bson_inq_object(pBson, "array");
	if(pInqArray == NULL)
		TOOLSLOG("inq failed %x", pInqArray);
	if(pInqArray != pArray)
		TOOLSLOG("inq failed %x/%x", pInqArray, pArray);

	pObject = t_bson_malloc_object();
	t_bson_add_boolean(pObject, "false", false);
	t_bson_add_double(pObject, "12", 12.3456);
	t_bson_add_string(pObject, "string", "string");

	t_bson_add_object(pBson, "object", pObject);
	pInqObject = t_bson_inq_object(pBson, "object");
	if(pInqObject == NULL)
		TOOLSLOG("inq failed %x", pInqObject);
	if(pInqObject != pObject)
		TOOLSLOG("inq failed %x/%x", pInqObject, pObject);

	t_bson_free_object(pBson);

	TOOLSLOG("Time-consuming:%dus", dave_os_time_us() - start_test_time);
}

static void
_t_bson_test_cases_5(void)
{
	ub start_test_time = dave_os_time_us();
	void *pBson;
	bool bool_value;

	pBson = t_bson_malloc_object();

	t_bson_add_boolean(pBson, "bool_value", true);
	if(t_bson_inq_boolean(pBson, "bool_value", &bool_value) == false)
		TOOLSLOG("inq failed %d", bool_value);
	if(bool_value != true)
		TOOLSLOG("inq failed %d", bool_value);

	t_bson_add_boolean(pBson, "bool_value", false);
	if(t_bson_inq_boolean(pBson, "bool_value", &bool_value) == false)
		TOOLSLOG("inq failed %d", bool_value);
	if(bool_value != false)
		TOOLSLOG("inq failed %d", bool_value);	

	t_bson_add_boolean(pBson, "bool_value", true);
	if(t_bson_inq_boolean(pBson, "bool_value", &bool_value) == false)
		TOOLSLOG("inq failed %d", bool_value);
	if(bool_value != true)
		TOOLSLOG("inq failed %d", bool_value);

	t_bson_add_double(pBson, "12", 12.3456);
	t_bson_add_double(pBson, "12", 11111);

	t_bson_free_object(pBson);

	TOOLSLOG("Time-consuming:%dus", dave_os_time_us() - start_test_time);	
}

static void
_t_bson_test_cases_6(void)
{
	ub start_test_time = dave_os_time_us();
	void *pBson, *pArray, *pObject, *pInqArray;

	pBson = t_bson_malloc_object();

	pArray = t_bson_malloc_array();
	t_bson_array_add_boolean(pArray, false);
	t_bson_array_add_double(pArray, 12.3456);
	t_bson_array_add_string(pArray, "string");

	pObject = t_bson_malloc_object();
	t_bson_add_boolean(pObject, "false", false);
	t_bson_add_double(pObject, "12", 12.3456);
	t_bson_add_string(pObject, "string", "string");

	t_bson_array_add_object(pArray, pObject);

	t_bson_add_object(pBson, "array", pArray);
	pInqArray = t_bson_inq_object(pBson, "array");
	if(pInqArray == NULL)
		TOOLSLOG("inq failed %x", pInqArray);
	if(pInqArray != pArray)
		TOOLSLOG("inq failed %x/%x", pInqArray, pArray);

	t_bson_free_object(pBson);

	TOOLSLOG("Time-consuming:%dus", dave_os_time_us() - start_test_time);
}

static void
_t_bson_test_cases_7(void)
{
	ub start_test_time = dave_os_time_us();
	void *pBson, *pSBson;
	char *serialize_ptr;
	size_t serialize_len;
	bool bool_value;
	char *string_value;
	size_t string_len;

	pBson = t_bson_malloc_object();

	t_bson_add_boolean(pBson, "false", false);
	t_bson_add_double(pBson, "12", 12.3456);
	t_bson_add_string(pBson, "string", "string");

	serialize_ptr = t_bson_to_serialize(pBson, &serialize_len);
	pSBson = t_serialize_to_bson(serialize_ptr, serialize_len);
	if(pSBson == NULL)
	{
		TOOLSLOG("pSBson is NULL!");
	}
	else
	{
		if(t_bson_inq_boolean(pSBson, "false", &bool_value) == false)
			TOOLSLOG("inq failed %d", bool_value);
		if(bool_value != false)
			TOOLSLOG("inq failed %d", bool_value);
		if(t_bson_inq_string(pBson, "string", &string_value, &string_len) == false)
			TOOLSLOG("inq failed %s", string_value);
		if(dave_strcmp(string_value, "string") == dave_false)
			TOOLSLOG("inq failed %d/%s", string_len, string_value);

		t_bson_free_object(pSBson);
	}

	t_bson_free_object(pBson);

	TOOLSLOG("Time-consuming:%dus", dave_os_time_us() - start_test_time);
}

static void
_t_bson_test_cases_8(void)
{
	ub start_test_time = dave_os_time_us();
	void *pBson, *pArray, *pSBson, *pSArray;
	char *serialize_ptr;
	size_t serialize_len;
	bool bool_value;
	double double_value;
	char *string_value;
	size_t string_len;

	pBson = t_bson_malloc_object();

	t_bson_add_boolean(pBson, "false", false);
	t_bson_add_double(pBson, "12", 12.3456);
	t_bson_add_string(pBson, "string", "string");

	pArray = t_bson_malloc_array();
	t_bson_array_add_boolean(pArray, true);
	t_bson_array_add_double(pArray, 12.3456);
	t_bson_array_add_string(pArray, "string");

	t_bson_add_object(pBson, "array", pArray);

	serialize_ptr = t_bson_to_serialize(pBson, &serialize_len);
	pSBson = t_serialize_to_bson(serialize_ptr, serialize_len);
	if(pSBson == NULL)
	{
		TOOLSLOG("pSBson is NULL!");
	}
	else
	{
		if(t_bson_inq_boolean(pSBson, "false", &bool_value) == false)
			TOOLSLOG("inq failed %d", bool_value);
		if(bool_value != false)
			TOOLSLOG("inq failed %d", bool_value);
		if(t_bson_inq_string(pBson, "string", &string_value, &string_len) == false)
			TOOLSLOG("inq failed %s", string_value);
		if(dave_strcmp(string_value, "string") == dave_false)
			TOOLSLOG("inq failed %d/%s", string_len, string_value);

		pSArray = t_bson_inq_object(pBson, "array");
		if(pSArray == NULL)
		{
			TOOLSLOG("inq array failed!");
		}
		else
		{
			if(t_bson_array_inq_boolean(pSArray, 0, &bool_value) == false)
				TOOLSLOG("inq array bool value failed");
			if(bool_value != true)
				TOOLSLOG("inq array bool value:%d failed", bool_value);
			if(t_bson_array_inq_double(pSArray, 1, &double_value) == false)
				TOOLSLOG("inq array double value failed");
			if(double_value != 12.3456)
				TOOLSLOG("inq array double value:%f failed", double_value);
			if(t_bson_array_inq_string(pSArray, 2, &string_value, &string_len) == false)
				TOOLSLOG("inq array string value failed");
			if(dave_strcmp(string_value, "string") == dave_false)
				TOOLSLOG("inq array string value:%s failed", string_value);			
		}

		t_bson_free_object(pSBson);
	}

	t_bson_free_object(pBson);

	TOOLSLOG("Time-consuming:%dus", dave_os_time_us() - start_test_time);
}

static void
_t_bson_test_cases_9(void)
{
	ub start_test_time = dave_os_time_us();
	void *pBson, *pArray, *pJson;
	const char *json_string;

	pBson = t_bson_malloc_object();

	t_bson_add_boolean(pBson, "false", false);
	t_bson_add_double(pBson, "12", 12.3456);
	t_bson_add_string(pBson, "string", "string");
	t_bson_add_bin(pBson, "bin", (char *)pBson, sizeof(pBson));

	pArray = t_bson_malloc_array();
	t_bson_array_add_boolean(pArray, true);
	t_bson_array_add_double(pArray, 12.3456);
	t_bson_array_add_string(pArray, "string");

	t_bson_add_object(pBson, "array", pArray);

	pJson = t_bson_to_json(pBson);

	t_bson_free_object(pBson);

	TOOLSLOG("Time-consuming:%dus", dave_os_time_us() - start_test_time);

	json_string = json_object_to_json_string_ext(pJson, JSON_C_TO_STRING_PRETTY);

	TOOLSLOG("%s", json_string);

	json_object_put(pJson);
}

static void
_t_bson_test_cases_10(void)
{
	ub start_test_time = dave_os_time_us();
	void *pBson;
	MBUF *mbuf_data;

	pBson = t_bson_malloc_object();

	t_bson_add_boolean(pBson, "false", false);
	t_bson_add_double(pBson, "12", 12.3456);
	t_bson_add_string(pBson, "string", "string");
	t_bson_add_bin(pBson, "bin", (char *)pBson, sizeof(pBson));

	mbuf_data = t_bson_to_mbuf(pBson);
	t_bson_free_object(t_mbuf_to_bson(mbuf_data));
	dave_mfree(mbuf_data);

	t_bson_free_object(pBson);

	TOOLSLOG("Time-consuming:%dus", dave_os_time_us() - start_test_time);
}

static void
_t_bson_test_cases_11(void)
{
	ub start_test_time;
	void *pBson, *pJson;

	start_test_time = dave_os_time_us();
	pBson = t_bson_malloc_object();
	t_bson_add_boolean(pBson, "false", false);
	t_bson_add_double(pBson, "12", 12.3456);
	t_bson_add_string(pBson, "string", "string");
	t_bson_free_object(pBson);
	TOOLSLOG("BSON Time-consuming:%dus", dave_os_time_us() - start_test_time);

	start_test_time = dave_os_time_us();
	pJson = json_object_new_object();
	json_object_object_add((struct json_object *)pJson, "false", json_object_new_boolean((json_bool)false));
	json_object_object_add((struct json_object *)pJson, "12", json_object_new_double(12.3456));
	json_object_object_add((struct json_object *)pJson, "string", json_object_new_string("string"));
	json_object_put(pJson);
	TOOLSLOG("JSON Time-consuming:%dus", dave_os_time_us() - start_test_time);	
}

// =====================================================================

void
t_bson_test(void)
{
	TOOLSLOG("bson test start ...");

	_t_bson_test_cases_1();
	_t_bson_test_cases_2();
	_t_bson_test_cases_3();
	_t_bson_test_cases_4();
	_t_bson_test_cases_5();
	_t_bson_test_cases_6();
	_t_bson_test_cases_7();
	_t_bson_test_cases_8();
	_t_bson_test_cases_9();
	_t_bson_test_cases_10();
	_t_bson_test_cases_11();
}

