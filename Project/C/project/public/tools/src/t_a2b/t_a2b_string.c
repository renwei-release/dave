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
		number = bin_ptr[bin_index];

		buf_ptr[buf_index ++] = digits[number >> 4];
		buf_ptr[buf_index ++] = digits[number & 0x0f];
	}

	if((buf_index + 1) <= buf_len)
	{
		buf_ptr[buf_index] = '\0';
	}

	return buf_index;
}

static ub
_t_a2b_string_to_digital(s8 *str_ptr, ub str_len, ub base)
{
	ub digital, str_index;

	digital = 0;
	str_index = 0;

	while(str_index < 64)
	{
		if((str_ptr[str_index] == ' ')
			|| (str_ptr[str_index] == '\0')
			|| ((str_ptr[str_index] == '0') && (str_ptr[str_index + 1] == 'x')))
		{
			str_index ++;
		}
		else
		{
			break;
		}
	}

	while(str_index < 64)
	{
		if((str_ptr[str_index] >= '0') && (str_ptr[str_index] <= '9'))
		{
			digital = digital * base;
			digital += (str_ptr[str_index] - '0');
		}
		else if((str_ptr[str_index] >= 'a') && (str_ptr[str_index] <= 'f'))
		{
			digital = digital * base;
			digital += (10 + (str_ptr[str_index] - 'a'));
		}	
		else if((str_ptr[str_index] >= 'A') && (str_ptr[str_index] <= 'F'))
		{
			digital = digital * base;
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

static ub
_t_a2b_double_to_str(s8 *str_ptr, ub str_len, double double_data)
{
	ub str_index;

	str_index = dave_snprintf(str_ptr, str_len, "%lf", double_data) + 1;
	if((dave_strcmp(str_ptr, "-nan") == dave_true)
		|| (dave_strcmp(str_ptr, "nan") == dave_true)
		|| (dave_strcmp(str_ptr, "-inf") == dave_true)
		|| (dave_strcmp(str_ptr, "inf") == dave_true))
	{
		TOOLSDEBUG("double_data:%lf is:%s why?", double_data, str_ptr);
		str_index = dave_snprintf(str_ptr, str_len, "%lf", 0.0) + 1;
	}

	return str_index;
}

static ub
_t_a2b_str_to_double(double *double_data, s8 *str_ptr, ub str_len, dave_bool *error)
{
	s8 digits[256], decimal[256];
	ub str_index, digits_index, digits_len, decimal_index, decimal_len;
	dave_bool negative_number;
	double decimal_except;
	dave_bool find_end_flag, find_error_flag;

	if(str_ptr == NULL)
	{
		return 0;
	}

	digits[0] = decimal[0] = '\0';
	str_index = digits_index = decimal_index = 0;
	digits_len = sizeof(digits) - 1;
	decimal_len = sizeof(decimal) - 1;
	negative_number = dave_false;
	decimal_except = 1;
	find_end_flag = find_error_flag = dave_false;

	for(digits_index=0; (str_index<str_len)&&(digits_index<digits_len); str_index++)
	{
		if(str_ptr[str_index] == '\0')
		{
			str_index += 1;
			find_end_flag = dave_true;
			break;
		}

		if(t_is_digit((u8)str_ptr[str_index]) == dave_true)
		{
			digits[digits_index ++] = str_ptr[str_index];
		}
		else if(str_ptr[str_index] == '.')
		{
			break;
		}
		else if(str_ptr[str_index] == '-')
		{
			negative_number = dave_true;
		}
		else
		{
			TOOLSABNOR("double has invalid data:%c<%x><%d/%d>|%s",
				str_ptr[str_index], str_ptr[str_index], str_index, str_len,
				&str_ptr[str_index]);

			find_error_flag = dave_true;
		}
	}

	digits[digits_index] = '\0';

	if(str_ptr[str_index] == '.')
	{
		str_index ++;

		for(decimal_index=0; (str_index<str_len)&&(decimal_index<decimal_len); str_index++)
		{
			if(str_ptr[str_index] == '\0')
			{
				str_index += 1;
				find_end_flag = dave_true;
				break;
			}

			if(t_is_digit((u8)str_ptr[str_index]) == dave_true)
			{
				decimal[decimal_index ++] = str_ptr[str_index];

				decimal_except *= 10;
			}
			else
			{
				TOOLSABNOR("double has invalid data:%c<%x><%d/%d>|%s",
					str_ptr[str_index], str_ptr[str_index], str_index, str_len,
					&str_ptr[str_index]);

				find_error_flag = dave_true;
			}
		}
	}

	decimal[decimal_index] = '\0';

	*double_data = (double)(stringdouble(digits)) + ((double)(stringdouble(decimal))) / decimal_except;
	if(negative_number == dave_true)
	{
		*double_data = - *double_data;
	}

	if(find_end_flag == dave_false)
	{
		TOOLSABNOR("can't find end flag: str:%d/%d digits:%d/%d decimal:%d/%d",
			str_index, str_len, digits_index, digits_len, decimal_index, decimal_len);
	}

	if(error != NULL)
	{
		if(find_error_flag == dave_true)
		{
			*error = dave_true;
		}
	}

	return str_index;
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
	ub str_len = dave_strlen(str_ptr);

	if(t_is_decimal_str(str_ptr, str_len) == dave_true)
		return _t_a2b_string_to_digital(str_ptr, str_len, 10);
	else
		return _t_a2b_string_to_digital(str_ptr, str_len, 16);
}

ub
t_a2b_stringhex_to_digital(s8 *str_ptr)
{
	return _t_a2b_string_to_digital(str_ptr, dave_strlen(str_ptr), 16);
}

ub
t_a2b_bin_to_hex_string(s8 *buf_ptr, ub buf_len, u8 *bin_ptr, ub bin_len)
{
	return _t_a2b_bin_to_hex_str(buf_ptr, buf_len, bin_ptr, bin_len);
}

double
t_a2b_string_to_double(s8 *string)
{
	double double_data;

	if(string == NULL)
	{
		return 0;
	}

	_t_a2b_str_to_double(&double_data, string, dave_strlen(string) + 1, NULL);

	return double_data;
}

ub
t_a2b_double_to_string(s8 *string_ptr, ub string_len, double double_data)
{
	return _t_a2b_double_to_str(string_ptr, string_len, double_data);
}

