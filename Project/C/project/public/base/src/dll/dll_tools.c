/*
 * Copyright (c) 2023 Renwei
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
_dll_remove_some_data(s8 *data_ptr, ub data_len)
{
	ub temp_len = 4096;
	s8 *temp_buffer = dave_malloc(temp_len);
	ub temp_index;
	ub data_index;
	ub EXTRA_len;
	dave_bool find_b_start;

	temp_index = 0;
	data_index = 0;
	find_b_start = dave_true;

	/*
	 * 此处是为了过滤python里面byte类型的b'xxxxxxx'表示符号b'与‘
	 */
	while((data_index < data_len) && (temp_index < (temp_len - 1)))
	{
		if(find_b_start == dave_true)
		{
			if((data_ptr[data_index] == 'b')
				&& (data_ptr[data_index + 1] == '\''))
			{
				data_index += 2;
				find_b_start = dave_false;
			}
			else
			{
				temp_buffer[temp_index ++] = data_ptr[data_index ++];
			}
		}
		else
		{
			if(data_ptr[data_index] == '\'')
			{
				data_index += 1;
				find_b_start = dave_true;
			}
			else
			{
				temp_buffer[temp_index ++] = data_ptr[data_index ++];
			}
		}
	}

	temp_buffer[temp_index] = '\0';

	/*
	 * 此处是为了过滤Go里面，如果在打印里面没有%d类似处理时，会在字符串最后
	 * 携带%!(EXTRA []interface {}=[])
	 * 这似乎是Go的BUG。Go版本：1.17.1
	 */
	EXTRA_len = 27;
	if((temp_index >= EXTRA_len)
		&& (dave_strcmp(&temp_buffer[temp_index-EXTRA_len], "%!(EXTRA []interface {}=[])") == dave_true))
	{
		temp_buffer[temp_index-EXTRA_len] = '\0';
	}

	dave_strcpy(data_ptr, temp_buffer, data_len + 1);

	dave_free(temp_buffer);
}

// =====================================================================

void
dll_remove_some_data(s8 *data_ptr, ub data_len)
{
	_dll_remove_some_data(data_ptr, data_len);
}

#endif

