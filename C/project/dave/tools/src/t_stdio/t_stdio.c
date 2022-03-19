/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <string.h>
#include "dave_base.h"
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
t_stdio_memcpy(u8 *dst, u8 *src, ub len)
{
	if((dst == NULL) || (src == NULL) || (len == 0))
	{
		return 0;
	}

	memcpy((void *)dst, (void *)src, (unsigned)len);

	return len;
}

ub
t_stdio_memmove(u8 *dst, u8 *src, ub len)
{
	if((dst == NULL) || (src == NULL) || (len == 0))
	{
		return 0;
	}

	memmove((void *)dst, (void *)src, (size_t)len);

	return len;
}

dave_bool
t_stdio_memcmp(u8 *cmp1, u8 *cmp2, ub cmp_len)
{
    if((cmp1 == NULL) || (cmp2 == NULL) || (cmp_len == 0))
    {
        return dave_false;
    }
 
    if(memcmp(cmp1, cmp2, cmp_len) == 0)
    {
        return dave_true;
    }
    else
    {
		return dave_false;
    }
}

void
t_stdio_memset(u8 *mem, u8 data, ub len)
{
	if((mem == NULL) || (len == 0))
	{
		return;
	}

	memset(mem, data, len);
}

ub
t_stdio_strlen(s8 *str)
{
    if(str == NULL)
    {
        return 0;
    }

    return (ub)strlen((char *)str);
}

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
t_stdio_snprintf(s8 *buf_ptr, ub buf_len, const char *fmt, ...)
{
	va_list args;
	ub printf_len;

	if((buf_len == 0) || (buf_len > 0xffffffff))
	{
		return 0;
	}

	va_start(args, fmt);
	printf_len = (ub)vsnprintf((char *)buf_ptr, buf_len, fmt, args);
	va_end(args);

	if (printf_len >= buf_len)
	{
		return buf_len - 1;
	}

	return printf_len;
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

ub
__t_stdio_strcpy__(s8 *dst, const s8 *src, ub max_length, s8 *file, ub line)
{
	ub copy_length;

	if(dst == NULL)
	{
		TOOLSLOG("copy dest is NULL! <%s:%d>", file, line);
		return 0;
	}

	if(src == NULL)
	{
		dst[0] = '\0';
		TOOLSLOG("copy src is NULL! <%s:%d>\r\n", file, line);
		return 0;
	}

	copy_length = 0;

	if(max_length > 0)
	{
		max_length -= 1;	// To insert '\0'

		while((copy_length < max_length) && ((*dst++ = *src++) != '\0')) { copy_length ++; }

		if(copy_length >= max_length)
		{
			*dst = '\0';
		}
	}
	else
	{
		*dst = '\0';
	}

	return copy_length;
}

dave_bool
__t_stdio_strcmp__(s8 *cmp1, s8 *cmp2, s8 *file, ub line)
{
	sb safe_counter;
	s8 ret;

	if((cmp1 == NULL) && (cmp2 == NULL))
		return dave_true;

	if((cmp1 == NULL) || (cmp2 == NULL))
		return dave_false;

	safe_counter = 0;

	ret = (*cmp1 - *cmp2);

	while(((++ safe_counter) < 40960) && (!ret) && (*cmp1) && (*cmp2))
	{ 
		cmp1 ++;
		cmp2 ++;
		ret = (*cmp1 - *cmp2);
	}

	if(safe_counter >= 40960)
	{
		TOOLSABNOR("Why do so long strings need to be compared?");
	}

	if((ret == 0) && (*cmp1 == *cmp2))
	{
		return dave_true;
	}
	else
	{
		return dave_false;
	}
}

s8 *
t_stdio_strfind(s8 *str, s8 end_char, s8 *find_ptr, ub find_len)
{
	ub find_index;

	find_ptr[0] = '\0';

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
t_stdio_tolowers(s8 *str)
{
	s8 *opt_str = str;
	ub safe_counter;

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
	ub safe_counter;

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

