/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "tools_log.h"

// =====================================================================

dave_bool
t_is_alpha(u8 c)
{
	return ((((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')))? dave_true : dave_false);
}

dave_bool
t_is_digit(u8 c)
{
	return (((c >= '0') && (c <= '9'))? dave_true : dave_false);
}

dave_bool 
t_is_digit_or_alpha(u8 c)
{
	return ((((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9')))? dave_true : dave_false);
}

dave_bool
t_is_all_digit(u8 *data_ptr, ub data_len)
{
	ub data_index;

	for(data_index=0; data_index<data_len; data_index++)
	{
		if(t_is_digit(data_ptr[data_index]) == dave_false)
		{
			return dave_false;
		}
	}

	return dave_true;
}

dave_bool 
t_is_all_digit_or_alpha(u8 *data_ptr, ub data_len)
{
	ub data_index;

	for(data_index=0; data_index<data_len; data_index++)
	{
		if(t_is_digit_or_alpha(data_ptr[data_index]) == dave_false)
		{
			return dave_false;
		}
	}

	return dave_true;
}

dave_bool
t_is_separator(u8 c)
{
	return (((c == ':') || (c == ' ')||(c == '.'))? dave_true : dave_false);
}

dave_bool
t_is_show_char(u8 c)
{
	return (((c >= 0x20) && (c <= 0x7E)) || (c >= 0x80)) ? dave_true : dave_false;
}

dave_bool
t_is_all_show_char(u8 *data_ptr, ub data_len)
{
	ub data_index;

	if(data_len == 0)
	{
		return dave_false;
	}

	for(data_index=0; data_index<data_len; data_index++)
	{
		if(data_ptr[data_index] == '\0')
		{
			if(data_index == 0)
			{
				return dave_false;
			}
			else
			{
				return dave_true;
			}
		}

		if(t_is_show_char(data_ptr[data_index]) == dave_false)
		{
			return dave_false;
		}
	}

	return dave_true;
}

dave_bool
t_is_all_show_char_or_rn(u8 *data_ptr, ub data_len)
{
	ub data_index;

	for(data_index=0; data_index<data_len; data_index++)
	{
		if((data_len - data_index) > 2)
		{
			if(t_is_show_char(data_ptr[data_index]) == dave_false)
			{
				TOOLSDEBUG("%x, %s", data_ptr[data_index], &data_ptr[data_index]);
				return dave_false;
			}
		}

		if((data_len - data_index) <= 2)
		{
			if(t_is_show_char(data_ptr[data_index]) == dave_false)
			{
				if((data_ptr[data_index] != '\r') && (data_ptr[data_index] != '\n'))
				{
					return dave_false;
				}
			}
		}
	}

	return dave_true;
}

dave_bool
t_is_legal_char(u8 c)
{
	return ((c >= 0x20) && (c <= 0x7E)) ? dave_true : dave_false;
}

dave_bool
t_is_empty_str(s8 *str)
{
	if(str == NULL)
		return dave_true;

	if(str[0] == '\0')
		return dave_true;

	return dave_false;
}

dave_bool
t_is_ipv4(s8 *ip_str)
{
	u8 ip[8];
	ub index, ip_str_len;

	ip_str_len = dave_strlen(ip_str);
	
	dave_memset(ip, 0x00, sizeof(ip));

	if ((15 < ip_str_len) || (7 > ip_str_len))
	{
		return dave_false;
	}

	for (index=0; index<ip_str_len; ++index)
	{
		if (t_is_digit((u8)ip_str[index]) == dave_false)
		{
			if ('.' != (u8)ip_str[index])
			{
				return dave_false;
			}
		}
	}

	dave_sscanf(ip_str, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);

	for(index=0; index<4; ++index)
	{
		if ((0 >ip[index]) || (255 < ip[index]))
		{
			return dave_false;
		}
	}

	return dave_true;
}

dave_bool
t_is_decimal_str(s8 *str_ptr, ub str_len)
{
	ub str_index;
	dave_bool ret;

	if(str_len == 0)
		str_len = dave_strlen(str_ptr);

	if(str_len > 2)
	{
		if((str_ptr[0] == '0')
			&& ((str_ptr[1] == 'x') || (str_ptr[1] == 'X')))
		{
			return dave_false;
		}
	}

	ret = dave_true;

	for(str_index=0; str_index<str_len; str_index++)
	{
		if(str_ptr[str_index] == '\0')
			break;

		if(t_is_digit((u8)str_ptr[str_index]) == dave_false)
		{
			switch(str_ptr[str_index])
			{
				case 'a':
				case 'A':
				case 'b':
				case 'B':
				case 'c':
				case 'C':
				case 'd':
				case 'D':
				case 'e':
				case 'E':
				case 'f':
				case 'F':
						ret = dave_false;
					break;
				default:
					break;
			}

			break;
		}
	}

	return ret;
}

