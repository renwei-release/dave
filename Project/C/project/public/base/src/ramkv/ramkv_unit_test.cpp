/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__) && defined(FORM_PRODUCT_BIN)
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

		if(base_ramkv_add_key_ptr(ramkv, key_1, ptr_1) == dave_false)
		{
			KVLOG("%s:%d failed! ******", __func__, __LINE__);
		}
		if(base_ramkv_add_key_ptr(ramkv, key_2, ptr_2) == dave_false)
		{
			KVLOG("%s:%d failed! ******", __func__, __LINE__);
		}

		inq_ptr = base_ramkv_inq_key_ptr(ramkv, key_1);
		if(inq_ptr != ptr_1)
		{
			KVLOG("%s:%d failed! ptr:%lx/%lx ******", __func__, __LINE__, inq_ptr, ptr_1);
		}
		inq_ptr = base_ramkv_inq_key_ptr(ramkv, key_2);
		if(inq_ptr != ptr_2)
		{
			KVLOG("%s:%d failed! ptr:%lx/%lx ******", __func__, __LINE__, inq_ptr, ptr_1);
		}

		if(base_ramkv_add_key_ptr(ramkv, key_1, ptr_2) == dave_false)
		{
			KVLOG("%s:%d failed! ******", __func__, __LINE__);
		}
		if(base_ramkv_inq_key_ptr(ramkv, key_1) != ptr_2)
		{
			KVLOG("%s:%d failed! ******", __func__, __LINE__);
		}
		if(base_ramkv_inq_key_ptr(ramkv, key_2) != ptr_2)
		{
			KVLOG("%s:%d failed! ******", __func__, __LINE__);
		}

		if(base_ramkv_inq_key_ptr(ramkv, (s8 *)"dddasdasdfdd") != NULL)
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

		if(base_ramkv_add_key_ptr(ramkv, key_1, ptr_1) == dave_false)
		{
			KVLOG("%s:%d failed! ******", __func__, __LINE__);
		}
		if(base_ramkv_add_key_ptr(ramkv, key_2, ptr_2) == dave_false)
		{
			KVLOG("%s:%d failed! ******", __func__, __LINE__);
		}

		if(base_ramkv_inq_key_ptr(ramkv, key_1) != ptr_1)
		{
			KVLOG("%s:%d failed! ******", __func__, __LINE__);
		}
		if(base_ramkv_inq_key_ptr(ramkv, key_2) != ptr_2)
		{
			KVLOG("%s:%d failed! ******", __func__, __LINE__);
		}

		if(base_ramkv_inq_key_ptr(ramkv, key_1) != NULL)
		{
			KVLOG("%s:%d failed! ******", __func__, __LINE__);
		}	
		if(base_ramkv_inq_key_ptr(ramkv, key_2) != NULL)
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

	ramkv = base_ramkv_malloc((s8 *)"testramkv", attrib, 0, NULL);

	test_time = _ramkv_test_add_and_inq(ramkv, test_counter);

	test_time += _ramkv_test_add_and_del(ramkv, test_counter);

	base_ramkv_free(ramkv, NULL);

	KVLOG("end ramkv database test! attrib:%d counter:%d time:%ldus times:%ldus",
		attrib, test_counter,  test_time, test_time/test_counter);
}

static void
_ramkv_test_add_del_not_free(KvAttrib attrib, s8 *test_counter_str)
{
	ub test_time;
	void *ramkv;
	ub test_counter = stringdigital(test_counter_str);

	if(test_counter == 0)
	{
		test_counter = 1;
	}

	KVLOG("start ramkv database test(attrib:%d loop:%d) ...", attrib, test_counter);

	ramkv = base_ramkv_malloc((s8 *)"testramkv", attrib, 0, NULL);

	test_time = _ramkv_test_add_and_del(ramkv, test_counter);

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
		ptr = base_ramkv_del_key_ptr(_timer_ramkv, key_ptr);
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
	void *ptr = base_ramkv_del_key_ptr(ramkv, key);

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

		_timer_ramkv = base_ramkv_malloc((s8 *)"testramkv", KvAttrib_list, out_second, _ramkv_test_timer_out);
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
		base_ramkv_free(_timer_ramkv, _ramkv_test_recycle);

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

	base_ramkv_add_key_ptr(_timer_ramkv, key, ptr);
}

static void
_ramkv_test_timer_thread_loop(ub thread_index, ub test_counter)
{
	static s8 static_str_key[64] = { '\0' };
	ub test_index;
	s8 str_key[64];
	void *str_ptr, *inq_ptr;
	ub ub_key;
	void *ub_ptr;

	for(test_index=0; test_index<test_counter; test_index++)
	{
		dave_snprintf(str_key, sizeof(str_key), "testramkv-%ld%ld%lx", thread_index, test_index, t_rand());
		str_ptr = dave_malloc(sizeof(str_key));
		dave_strcpy(str_ptr, str_key, sizeof(str_key));
		if(base_ramkv_add_key_ptr(_timer_ramkv, str_key, str_ptr) == dave_true)
		{
			inq_ptr = base_ramkv_inq_key_ptr(_timer_ramkv, str_key);
			if(inq_ptr != str_ptr)
			{
				base_restart("test failed on str_key:%s inq! %lx/%lx", str_key, inq_ptr, str_ptr);
			}
		}

		dave_strcpy(static_str_key, str_key, sizeof(static_str_key));

		ub_key = thread_index + test_index + t_rand();
		ub_ptr = dave_malloc(sizeof(ub_key));
		dave_strcpy(ub_ptr, &ub_key, sizeof(ub_key));
		if(base_ramkv_add_ub_ptr(_timer_ramkv, ub_key, ub_ptr) == dave_true)
		{
			if(base_ramkv_inq_ub_ptr(_timer_ramkv, ub_key) != ub_ptr)
			{
				base_restart("test failed on ub_key:%ld inq!", ub_key);
			}
		}
	}

	SAFECODEv1( _timer_pv, {
		if((t_rand() & 0x01) == 0x00)
		{
			if(static_str_key[0] != '\0')
			{
				str_ptr = base_ramkv_del_key_ptr(_timer_ramkv, static_str_key);
				if(str_ptr != NULL)
				{
					dave_free(str_ptr);
				}
			}
		}
	} );
}

static void *
_ramkv_test_timer_thread_(void *arg)
{
	ub thread_index = (ub)arg;
	ub sleep_time, test_counter;

	while(_timer_ramkv != NULL)
	{
		sleep_time = (t_rand() % 128);
		if(sleep_time == 0)
			sleep_time = 1;
		dave_os_sleep(sleep_time);

		test_counter = (t_rand() % 4);
		if(test_counter == 0)
			test_counter = 1;

		_ramkv_test_timer_thread_loop(thread_index, test_counter);
	}

	KVLOG("exit timer thread!!!");

	return NULL;
}

static void
_ramkv_test_timer_thread(void)
{
	ub thread_number = dave_os_cpu_process_number() * 2;
	ub thread_index;
	s8 thread_name[128];

	if(_timer_ramkv == NULL)
		return;

	KVLOG("start %d threads ...", thread_number);

	for(thread_index=0; thread_index<thread_number; thread_index++)
	{
		dave_snprintf(thread_name, sizeof(thread_name), "testramkvdatabase_%d", thread_index);
		dave_os_create_thread((char *)thread_name, _ramkv_test_timer_thread_, (void *)thread_index);
	}
}

static void
_ramkv_test_timer_del(s8 *key)
{
	void *ptr = base_ramkv_inq_key_ptr(_timer_ramkv, key);

	if(ptr == NULL)
	{
		KVLOG("can't find the key:%s", key);
		return;
	}

	KVLOG("del key:%s", key);

	base_ramkv_del_key_ptr(_timer_ramkv, key);

	dave_free(ptr);
}

static void
_ramkv_test_timer_inq(s8 *key)
{
	void *ptr = base_ramkv_inq_key_ptr(_timer_ramkv, key);

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

// =====================================================================

TEST(ramkv_case, ramkv_case_1) { _ramkv_test_loop(KvAttrib_ram, (s8 *)"1000"); }
TEST(ramkv_case, ramkv_case_2) { _ramkv_test_loop(KvAttrib_list, (s8 *)"1000"); }
TEST(ramkv_case, ramkv_case_3) { _ramkv_test_timer_start((s8 *)"60"); }
TEST(ramkv_case, ramkv_case_4) { _ramkv_test_timer_stop(); }
TEST(ramkv_case, ramkv_case_5) { _ramkv_test_timer_add(); }
TEST(ramkv_case, ramkv_case_6) { _ramkv_test_timer_del((s8 *)"aaaaa"); }
TEST(ramkv_case, ramkv_case_7) { _ramkv_test_timer_inq((s8 *)"bbbbb"); }
TEST(ramkv_case, ramkv_case_8) { _ramkv_test_timer_info(); }
TEST(ramkv_case, ramkv_case_9) { _ramkv_test_add_del_not_free(KvAttrib_list, (s8 *)"1000"); }
TEST(ramkv_case, ramkv_case_10) { _ramkv_test_timer_thread(); }

#endif

