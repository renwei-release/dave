/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __T_STDIO_H__
#define __T_STDIO_H__
#include <string.h>
#include "tools_log.h"

static inline ub
t_stdio_memcpy(u8 *dst, u8 *src, ub len)
{
	if((dst == NULL) || (src == NULL) || (len == 0))
	{
		return 0;
	}

	memcpy((void *)dst, (void *)src, (unsigned)len);

	return len;
}

static inline ub
t_stdio_memmove(u8 *dst, u8 *src, ub len)
{
	if((dst == NULL) || (src == NULL) || (len == 0))
	{
		return 0;
	}

	memmove((void *)dst, (void *)src, (size_t)len);

	return len;
}

static inline dave_bool
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

static inline void
t_stdio_memset(u8 *mem, u8 data, ub len)
{
	if((mem == NULL) || (len == 0))
	{
		return;
	}

	memset(mem, data, len);
}

static inline ub
t_stdio_strlen(s8 *str)
{
    if(str == NULL)
    {
        return 0;
    }

    return (ub)strlen((const char *)str);
}

ub t_stdio_sprintf(s8 *buf, const char *fmt, ...);
ub t_stdio_sscanf(const s8 *buf, const char *fmt, ...);

static inline ub
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

ub t_stdio_snscanf(const s8 *buf_ptr, ub buf_len, const char *fmt, ...);

static inline ub
__t_stdio_strcpy__(s8 *dst, const s8 *src, ub max_length, s8 *file, ub line)
{
	s8 *original_dst;
	ub copy_index;

	if((dst == NULL) || (max_length <= 0))
	{
		TOOLSLOG("copy dest is NULL! <%s:%d>", file, line);
		return 0;
	}

	if(src == NULL)
	{
		dst[0] = '\0';
		TOOLSLOG("copy src is NULL! <%s:%d>", file, line);
		return 0;
	}

	// To insert '\0'
	max_length -= 1;
	original_dst = dst;

	for(copy_index=0; copy_index<max_length; copy_index++)
	{
		if((*dst++ = *src++) == '\0')
			break;
	}

	if(copy_index >= max_length)
	{
		original_dst[copy_index] = '\0';
	}

	return copy_index;
}

static inline dave_bool
__t_stdio_strcmp__(s8 *cmp1, s8 *cmp2, s8 *file, ub line)
{
	sb safe_counter, max_counter = 10485760;
	s8 ret;

	if((cmp1 == NULL) && (cmp2 == NULL))
		return dave_true;

	if((cmp1 == NULL) || (cmp2 == NULL))
		return dave_false;

	safe_counter = 0;

	ret = (*cmp1 - *cmp2);

	while(((++ safe_counter) < max_counter) && (!ret) && (*cmp1) && (*cmp2))
	{ 
		cmp1 ++;
		cmp2 ++;
		ret = (*cmp1 - *cmp2);
	}

	if(safe_counter >= max_counter)
	{
		TOOLSABNOR("Why do so long(%ld/%ld) strings need to be compared? <%s:%d>",
			safe_counter, max_counter, file, line);
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

s8 * t_stdio_strdup(s8 *str);
s8 * t_stdio_strfind(s8 *str, s8 end_char, s8 *find_ptr, ub find_len);
s8 * t_stdio_strfindfrist(s8 *str, s8 find_char);
s8 * t_stdio_strfindlast(s8 *str, s8 find_char);
s8 * t_stdio_tolowers(s8 *str);
s8 * t_stdio_touppers(s8 *str);
s8 * t_stdio_strstr(s8 *buf, s8 *sub);
s8 * t_stdio_remove_the_char_on_frist(s8 *data_ptr, s8 remove_char);

void t_stdio_print_hex(const char *msg, u8 *hex, ub hex_len);
void t_stdio_print_char(const char *msg, u8 *char_data, ub char_len);

#define t_stdio_strcpy(dst, src, max_length) __t_stdio_strcpy__((s8 *)(dst), (const s8 *)(src), (ub)(max_length), (s8 *)__func__, (ub)__LINE__)
#define t_stdio_strcmp(cmp1, cmp2) __t_stdio_strcmp__((s8 *)(cmp1), (s8 *)(cmp2), (s8 *)__func__, (ub)__LINE__)

#define dave_sprintf t_stdio_sprintf
#define dave_sscanf t_stdio_sscanf
#define dave_snprintf t_stdio_snprintf
#define dave_snscanf t_stdio_snscanf
#define dave_strcpy t_stdio_strcpy
#define dave_strcmp t_stdio_strcmp
#define dave_strdup t_stdio_strdup
#define dave_memcpy(a, b, c) t_stdio_memcpy((u8 *)(a), (u8 *)(b), (ub)(c))
#define dave_memmove(a, b, c) t_stdio_memmove((u8 *)(a), (u8 *)(b), (ub)(c))
#define dave_memcmp(a, b, c) t_stdio_memcmp((u8 *)(a), (u8 *)(b), (ub)(c))
#define dave_memset(a, b, c) t_stdio_memset((u8 *)(a), (u8)(b), (ub)(c))
#define dave_strlen(a) t_stdio_strlen((s8 *)(a))
#define dave_strfind t_stdio_strfind
#define dave_strfindfrist t_stdio_strfindfrist
#define dave_strfindlast t_stdio_strfindlast
#define dave_tolowers t_stdio_tolowers
#define dave_touppers t_stdio_touppers
#define dave_strstr t_stdio_strstr

#define upper t_stdio_touppers
#define lower t_stdio_tolowers

#endif

