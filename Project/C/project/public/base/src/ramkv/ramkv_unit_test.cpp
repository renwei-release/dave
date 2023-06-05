/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "dave_3rdparty.h"
#if defined(__DAVE_BASE__) && defined(GTEST_3RDPARTY)
#include <iostream>
using namespace std;
#include "gtest.h"
#include "dave_os.h"
#include "dave_tools.h"
#include "dave_base.h"
#include "ramkv_log.h"

#define MAX_KV_TEST_TIMES 50000

static void *_timer_ramkv = NULL;
static TLock _timer_pv;

static ub
_ramkv_test_add_and_inq(void *ramkv, ub test_counter)
{
	ub test_start_time, test_stop_time, test_time, while_counter = test_counter;
	s8 key_1[128];
	s8 *ptr_1 = key_1;
	s8 key_2[128];
	s8 *ptr_2 = key_2;
	void *inq_ptr;

	KVLOG("start %s test ...", __func__);

	test_start_time = dave_os_time_us();

	while((while_counter --) > 0)
	{
		dave_snprintf(key_1, sizeof(key_1), "add_inq_1_%d", while_counter);
		dave_snprintf(key_2, sizeof(key_2), "add_inq_2_%d", while_counter);

		if(kv_add_key_ptr(ramkv, key_1, ptr_1) == dave_false)
		{
			KVLOG("%s:%d failed! ******", __func__, __LINE__);
		}
		if(kv_add_key_ptr(ramkv, key_2, ptr_2) == dave_false)
		{
			KVLOG("%s:%d failed! ******", __func__, __LINE__);
		}

		inq_ptr = kv_inq_key_ptr(ramkv, key_1);
		if(inq_ptr != ptr_1)
		{
			KVLOG("%s:%d failed! ptr:%lx/%lx ******", __func__, __LINE__, inq_ptr, ptr_1);
		}
		inq_ptr = kv_inq_key_ptr(ramkv, key_2);
		if(inq_ptr != ptr_2)
		{
			KVLOG("%s:%d failed! ptr:%lx/%lx ******", __func__, __LINE__, inq_ptr, ptr_1);
		}

		if(kv_add_key_ptr(ramkv, key_1, ptr_2) == dave_false)
		{
			KVLOG("%s:%d failed! ******", __func__, __LINE__);
		}
		if(kv_inq_key_ptr(ramkv, key_1) != ptr_2)
		{
			KVLOG("%s:%d failed! ******", __func__, __LINE__);
		}
		if(kv_inq_key_ptr(ramkv, key_2) != ptr_2)
		{
			KVLOG("%s:%d failed! ******", __func__, __LINE__);
		}

		if(kv_inq_key_ptr(ramkv, (s8 *)"dddasdasdfdd") != NULL)
		{
			KVLOG("%s:%d failed! ******", __func__, __LINE__);
		}
	}

	test_stop_time = dave_os_time_us();

	test_time = test_stop_time - test_start_time;

	KVLOG("end %s test! time:%ldus times:%ldus",
		__func__, test_time, test_time / test_counter);

	return test_time;
}

static ub
_ramkv_test_add_and_del(void *ramkv, ub test_counter)
{
	ub test_start_time, test_stop_time, test_time, while_counter = test_counter;
	s8 key_1[128];
	void *ptr_1 = key_1;
	s8 key_2[128];
	void *ptr_2 = key_2;

	KVLOG("start %s test ...", __func__);

	test_start_time = dave_os_time_us();

	while((while_counter --) > 0)
	{
		dave_snprintf(key_1, sizeof(key_1), "add_del_1_%d", while_counter);
		dave_snprintf(key_2, sizeof(key_2), "add_del_2_%d", while_counter);

		if(kv_add_key_ptr(ramkv, key_1, ptr_1) == dave_false)
		{
			KVLOG("%s:%d failed! ******", __func__, __LINE__);
		}
		if(kv_add_key_ptr(ramkv, key_2, ptr_2) == dave_false)
		{
			KVLOG("%s:%d failed! ******", __func__, __LINE__);
		}

		if(kv_inq_key_ptr(ramkv, key_1) != ptr_1)
		{
			KVLOG("%s:%d failed! ******", __func__, __LINE__);
		}
		if(kv_inq_key_ptr(ramkv, key_2) != ptr_2)
		{
			KVLOG("%s:%d failed! ******", __func__, __LINE__);
		}

		if(kv_inq_key_ptr(ramkv, key_1) == NULL)
		{
			KVLOG("%s:%d failed! ******", __func__, __LINE__);
		}	
		if(kv_inq_key_ptr(ramkv, key_2) == NULL)
		{
			KVLOG("%s:%d failed! ******", __func__, __LINE__);
		}
	}

	test_stop_time = dave_os_time_us();

	test_time = test_stop_time - test_start_time;

	KVLOG("end %s test! time:%ldus times:%ldus",
		__func__, test_time, test_time / test_counter);

	return test_time;
}

static void
_ramkv_test_loop(KvAttrib attrib, s8 *test_counter_str)
{
	ub test_time;
	void *ramkv;
	ub test_counter = stringdigital(test_counter_str);

	if(test_counter == 0)
	{
		test_counter = 1;
	}
	if(test_counter > MAX_KV_TEST_TIMES)
	{
		test_counter = MAX_KV_TEST_TIMES;
	}

	KVLOG("start ramkv database test(attrib:%d loop:%d) ...", attrib, test_counter);

	ramkv = kv_malloc((s8 *)"testramkv", 0, NULL);

	test_time = _ramkv_test_add_and_inq(ramkv, test_counter);

	test_time += _ramkv_test_add_and_del(ramkv, test_counter);

	kv_free(ramkv, NULL);

	KVLOG("end ramkv database test! attrib:%d counter:%d time:%ldus times:%ldus",
		attrib, test_counter,  test_time, test_time/test_counter);
}

static void
_ramkv_test_add_del_free(KvAttrib attrib, s8 *test_counter_str)
{
	ub test_time;
	void *ramkv;
	ub test_counter = stringdigital(test_counter_str);

	if(test_counter == 0)
	{
		test_counter = 1;
	}

	KVLOG("start ramkv database test(attrib:%d loop:%d) ...", attrib, test_counter);

	ramkv = kv_malloc((s8 *)"testramkv", 0, NULL);

	test_time = _ramkv_test_add_and_del(ramkv, test_counter);

	kv_free(ramkv, NULL);

	KVLOG("end ramkv database test! attrib:%d counter:%d time:%ldus times:%ldus",
		attrib, test_counter,  test_time, test_time/test_counter);
}

static void
_ramkv_test_timer_out(void *ramkv, s8 *key_ptr)
{
	void *ptr;
	dave_bool del = ((t_rand() & 0xff) != 0x00) ? dave_true : dave_false;

	if(ramkv != _timer_ramkv)
	{
		KVLOG("ptr error:%x/%x!", ramkv, _timer_ramkv);
	}

	if(del == dave_true)
	{
		ptr = kv_del_key_ptr(_timer_ramkv, key_ptr);
		if(ptr != NULL)
		{
			dave_free(ptr);
		}
	}
}

static void
_ramkv_test_check_timer(TIMERID timer_id, ub thread_index)
{
	ub index;

	for(index=0; index<1024; index++)
	{
		if(_timer_ramkv != NULL)
		{
			base_ramkv_inq_index_ptr(_timer_ramkv, index);
		}
	}

	if(_timer_ramkv == NULL)
	{
		base_timer_die(timer_id);
	}
}

static RetCode
_ramkv_test_recycle(void *ramkv, s8 *key)
{
	void *ptr = kv_del_key_ptr(ramkv, key);

	if(ptr == NULL)
	{
		return RetCode_empty_data;
	}

	if(dave_free(ptr) == dave_false)
	{
		return RetCode_Arithmetic_error;
	}

	return RetCode_OK;
}

static void
_ramkv_test_timer_start(s8 *out_second_str)
{
	ub out_second;

	if(_timer_ramkv == NULL)
	{
		out_second = stringdigital(out_second_str);
		if(out_second == 0)
		{
			out_second = 30;
		}

		_timer_ramkv = kv_malloc((s8 *)"testramkv", out_second, _ramkv_test_timer_out);
		t_lock_reset(&_timer_pv);

		base_timer_creat((char *)"testramkv_check_timer", _ramkv_test_check_timer, 1000);

		KVLOG("ramkv database timer start! out_second:%d", out_second);
	}
	else
	{
		KVLOG("ramkv database timer already started!");
	}
}

static void
_ramkv_test_timer_stop(void)
{
	if(_timer_ramkv != NULL)
	{
		kv_free(_timer_ramkv, _ramkv_test_recycle);

		_timer_ramkv = NULL;
		t_lock_destroy(&_timer_pv);

		KVLOG("ramkv database timer stop!");
	}
}

static void
_ramkv_test_timer_add(void)
{
	s8 key[64];
	void *ptr;

	dave_snprintf(key, sizeof(key), "testramkv-%lx", t_rand());

	ptr = dave_malloc(sizeof(key));
	dave_strcpy(ptr, key, sizeof(key));

	KVLOG("add key:%s", key);

	kv_add_key_ptr(_timer_ramkv, key, ptr);
}

static void
_ramkv_test_timer_del(s8 *key)
{
	void *ptr = kv_inq_key_ptr(_timer_ramkv, key);

	if(ptr == NULL)
	{
		KVLOG("can't find the key:%s", key);
		return;
	}

	KVLOG("del key:%s", key);

	kv_del_key_ptr(_timer_ramkv, key);

	dave_free(ptr);
}

static void
_ramkv_test_timer_inq(s8 *key)
{
	void *ptr = kv_inq_key_ptr(_timer_ramkv, key);

	if(ptr == NULL)
	{
		KVLOG("can't find the key:%s", key);
		return;
	}

	KVLOG("inq key:%s", key);
}

static void
_ramkv_test_timer_info(void)
{
	s8 info_ptr[4096];

	base_ramkv_info(_timer_ramkv, info_ptr, sizeof(info_ptr));

	KVLOG("%s", info_ptr);
}

static void
_ramkv_test_add_short_add_long_add_short(void)
{
	void *kv;

	kv = kv_malloc((s8 *)"shortlong", 0, NULL);

	kv_add_key_value(kv, (s8 *)"aaa", (s8 *)"12");
	kv_add_key_value(kv, (s8 *)"aaa", (s8 *)"12dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd");
	kv_add_key_value(kv, (s8 *)"aaa", (s8 *)"12");
	kv_add_key_value(kv, (s8 *)"aaa", (s8 *)"12ddddddddddddddddddddddddddddddddddddddddddddddddddddddd");
	kv_add_key_value(kv, (s8 *)"aaa", (s8 *)"12");

	kv_free(kv, NULL);
}

// =====================================================================

TEST(ramkv_case, ramkv_case_1) { _ramkv_test_loop(KvAttrib_ram, (s8 *)"1000"); }
TEST(ramkv_case, ramkv_case_2) { _ramkv_test_loop(KvAttrib_list, (s8 *)"1000"); }
TEST(ramkv_case, ramkv_case_3) { _ramkv_test_timer_start((s8 *)"60"); }
TEST(ramkv_case, ramkv_case_4) { _ramkv_test_timer_stop(); }
TEST(ramkv_case, ramkv_case_5) { _ramkv_test_timer_add(); }
TEST(ramkv_case, ramkv_case_6) { _ramkv_test_timer_del((s8 *)"aaaaa"); }
TEST(ramkv_case, ramkv_case_7) { _ramkv_test_timer_inq((s8 *)"bbbbb"); }
TEST(ramkv_case, ramkv_case_8) { _ramkv_test_timer_info(); }
TEST(ramkv_case, ramkv_case_9) { _ramkv_test_add_del_free(KvAttrib_list, (s8 *)"1000"); }
TEST(ramkv_case, ramkv_case_10) { _ramkv_test_add_short_add_long_add_short(); }

#endif
