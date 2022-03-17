/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "tools_log.h"

static ub
_t_a2b_bin_to_hex_str(s8 *buf_ptr, ub buf_len, u8 *bin_ptr, ub bin_len)
{
	static const char digits[16] = "0123456789ABCDEF";
	ub buf_index, bin_index;
	u8 number;

	for(buf_index=0,bin_index=0; ((buf_index+2)<buf_len)&&(bin_index<bin_len); bin_index++)
	{
		number = bin_ptr[buf_index];

		buf_ptr[buf_index ++] = digits[number >> 4];
		buf_ptr[buf_index ++] = digits[number & 0x0f];
	}

	if((buf_index + 1) <= buf_len)
		buf_ptr[buf_index] = '\0';

	return buf_index;
}

// =====================================================================

ub
t_a2b_digital_to_string(s8 *str_ptr, ub str_len, ub digital)
{
	return t_stdio_snprintf(str_ptr, str_len, "%ld", digital);
}

ub
t_a2b_string_to_digital(s8 *str_ptr)
{
	ub digital, str_index;

	if(str_ptr == NULL)
	{
		TOOLSLOG("str is NULL!");
		return 0;
	}

	digital = 0;
	str_index = 0;

	while(str_index < 64)
	{
		if(str_ptr[str_index] != ' ')
		{
			break;
		}

		str_index ++;
	}

	while(str_index < 64)
	{
		if((str_ptr[str_index] >= '0') && (str_ptr[str_index] <= '9'))
		{
			digital = digital * 10;
			digital += (str_ptr[str_index] - '0');
		}
		else if((str_ptr[str_index] >= 'a') && (str_ptr[str_index] <= 'f'))
		{
			digital = digital * 10;
			digital += (10 + (str_ptr[str_index] - 'a'));
		}	
		else if((str_ptr[str_index] >= 'A') && (str_ptr[str_index] <= 'F'))
		{
			digital = digital * 10;
			digital += (10 + (str_ptr[str_index] - 'A'));
		}	
		else
		{
			break;
		}

		str_index ++;
	}

	return digital;
}

ub
t_a2b_bin_to_hex_string(s8 *buf_ptr, ub buf_len, u8 *bin_ptr, ub bin_len)
{
	return _t_a2b_bin_to_hex_str(buf_ptr, buf_len, bin_ptr, bin_len);
}

