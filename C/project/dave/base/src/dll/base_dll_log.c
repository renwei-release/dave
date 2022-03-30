/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_base.h"
#include "dave_verno.h"
#include "dave_tools.h"
#include "dll_log.h"

static void
_dll_log_remove_some_data(s8 *log_buffer, ub log_len)
{
	s8 temp_buffer[2048];
	ub temp_index;
	ub log_index;
	ub EXTRA_len;
	dave_bool find_b_start;

	temp_index = 0;
	log_index = 0;
	find_b_start = dave_true;

	/*
	 * 此处是为了过滤python里面byte类型的b'xxxxxxx'表示符号b'与‘
	 */
	while((log_index < log_len) && (temp_index < (sizeof(temp_buffer) - 1)))
	{
		if(find_b_start == dave_true)
		{
			if((log_buffer[log_index] == 'b')
				&& (log_buffer[log_index + 1] == '\''))
			{
				log_index += 2;
				find_b_start = dave_false;
			}
			else
			{
				temp_buffer[temp_index ++] = log_buffer[log_index ++];
			}
		}
		else
		{
			if(log_buffer[log_index] == '\'')
			{
				log_index += 1;
				find_b_start = dave_true;
			}
			else
			{
				temp_buffer[temp_index ++] = log_buffer[log_index ++];
			}
		}
	}

	temp_buffer[temp_index] = '\0';

	/*
	 * 此处是为了过滤Go里面，如果在打印里面没有%d类似处理时，会在字符串最后
	 * 携带%!(EXTRA []interface {}=[])
	 * 这似乎时Go的BUG。Go版本：1.17.1
	 */
	EXTRA_len = dave_strlen("%!(EXTRA []interface {}=[])");
	if((temp_index >= EXTRA_len)
		&& (dave_strcmp(&temp_buffer[temp_index-EXTRA_len], "%!(EXTRA []interface {}=[])") == dave_true))
	{
		temp_buffer[temp_index-EXTRA_len] = '\0';
	}

	dave_strcpy(log_buffer, temp_buffer, log_len + 1);
}

// =====================================================================

void
dave_dll_log(char *func, int line, char *log_msg)
{
	s8 log_buffer[2048];
	ub log_len;

	log_len = dave_strcpy(log_buffer, log_msg, sizeof(log_buffer));

	_dll_log_remove_some_data(log_buffer, log_len);

	DAVELOG("[%s]<%s:%d>%s\n", dave_verno_my_product(), func, line, log_buffer);
}

#endif

