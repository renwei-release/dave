/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <string.h>
#include "dave_base.h"
#include "dave_tools.h"
#include "tools_log.h"

static inline unsigned char
__tolower(unsigned char c)
{
	if((c >= 'A') && (c <= 'Z'))
	{
		c += 32;
	}
	return c;
}

static inline unsigned char
__toupper(unsigned char c)
{
	if((c >= 'a') && (c <= 'z'))
	{
		c -= 32;
	}
	return c;
}

// =====================================================================

ub
t_stdio_sprintf(s8 *buf, const char *fmt, ...)
{
	va_list args;
	ub printf_len;

	va_start(args, fmt);
	printf_len = (ub)vsprintf((char *)buf, fmt, args);
	va_end(args);

	return printf_len;
}

ub
t_stdio_sscanf(const s8 *buf, const char *fmt, ...)
{
	va_list args;
	ub scanf_len;

	va_start(args, fmt);
	scanf_len = (ub)vsscanf((char *)buf, fmt, args);
	va_end(args);

	return scanf_len;
}

ub
t_stdio_snscanf(const s8 *buf_ptr, ub buf_len, const char *fmt, ...)
{
	va_list args;
	ub printf_len;

	if((buf_len == 0) || (buf_len > 102400))
	{
		return 0;
	}

	va_start(args, fmt);
	printf_len = (ub)vsscanf((char *)buf_ptr, fmt, args);
	va_end(args);

	return printf_len;
}

s8 *
t_stdio_strdup(s8 *str)
{
	ub len;
	char *copy;

	len = strlen(str);

	copy = dave_malloc(len + 1);
	memcpy(copy, str, len);
	copy[len] = '\0';

	return copy;
}

s8 *
t_stdio_strfind(s8 *str, s8 end_char, s8 *find_ptr, ub find_len)
{
	ub find_index;

	find_ptr[0] = '\0';

	if(str == NULL)
	{
		return NULL;
	}

	find_index = 0;

	if(find_len <= 1)
	{
		return str;
	}

	find_len -= 1;

	while(find_index < find_len)
	{
		if(*str == '\0')
		{
			str = NULL;
			break;
		}

		if(*str == end_char)
		{
			str ++;
			break;
		}

		find_ptr[find_index ++] = *(str ++);
	}

	find_ptr[find_index] = '\0';

	return str;
}

s8 *
t_stdio_strfindfrist(s8 *str, s8 find_char)
{
	ub safe_counter;

	if(str == NULL)
	{
		return NULL;
	}

	safe_counter = 0;

	while((safe_counter ++) < 102400)
	{
		if(*str == '\0')
		{
			break;
		}

		if(*str == find_char)
		{
			str ++;

			if(*str == '\0')
			{
				return NULL;
			}

			return str;
		}

		str ++;
	}

	return NULL;
}

s8 *
t_stdio_strfindlast(s8 *str, s8 find_char)
{
	ub safe_counter;
	s8 *last_char_position;

	if(str == NULL)
	{
		return NULL;
	}

	safe_counter = 0;
	last_char_position = NULL;

	while((safe_counter ++) < 102400)
	{
		if(*str == '\0')
		{
			break;
		}

		if(*str == find_char)
		{
			last_char_position = (str + 1);
		}

		str ++;
	}

	return last_char_position;
}

s8 *
t_stdio_tolowers(s8 *str)
{
	s8 *opt_str = str;
	ub safe_counter = 0;

	if(opt_str != NULL)
	{
		safe_counter = 0;

		while((*opt_str != '\0') && ((safe_counter ++) < 10240))
		{
			*opt_str = __tolower(*opt_str); opt_str ++;
		}
	}

	if(safe_counter >= 10240)
	{
		TOOLSLOG("safe_counter:%d overflow!", safe_counter);
	}

	return str;
}

s8 *
t_stdio_touppers(s8 *str)
{
	s8 *opt_str = str;
	ub safe_counter = 0;

	if(opt_str != NULL)
	{
		safe_counter = 0;

		while((*opt_str != '\0') && ((safe_counter ++) < 10240))
		{
			*opt_str = __toupper(*opt_str); opt_str ++;
		}
	}

	if(safe_counter >= 10240)
	{
		TOOLSLOG("safe_counter:%d overflow!", safe_counter);
	}

	return str;
}

s8 *
t_stdio_strstr(s8 *buf, s8 *sub)
{
	s8 *bp;
	s8 *sp;

	if(!*sub)
		return buf;

	while(*buf)
	{
		bp = buf;
		sp = sub;
		do {
			if(!*sp)
				return buf;
		} while (*bp++ == *sp++);
		buf += 1;
	}
	return NULL;
}

s8 *
t_stdio_remove_the_char_on_frist(s8 *data_ptr, s8 remove_char)
{
	ub data_len, data_index;

	if(data_ptr == NULL)
		return NULL;

	data_len = t_stdio_strlen(data_ptr);

	for(data_index=0; data_index<data_len; data_index++)
	{
		if(data_ptr[data_index] != remove_char)
			break;
	}

	if(data_index > 0)
	{
		memmove(data_ptr, &data_ptr[data_index], data_len - data_index);
		data_ptr[data_len - data_index] = '\0';
	}

	return data_ptr;
}

