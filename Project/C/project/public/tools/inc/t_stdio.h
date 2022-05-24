/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __T_STDIO_H__
#define __T_STDIO_H__

ub t_stdio_memcpy(u8 *dst, u8 *src, ub len);
ub t_stdio_memmove(u8 *dst, u8 *src, ub len);
dave_bool t_stdio_memcmp(u8 *cmp1, u8 *cmp2, ub cmp_len);
void t_stdio_memset(u8 *mem, u8 data, ub len);
ub t_stdio_strlen(s8 *str);
ub t_stdio_sprintf(s8 *buf, const char *fmt, ...);
ub t_stdio_sscanf(const s8 *buf, const char *fmt, ...);
ub t_stdio_snprintf(s8 *buf_ptr, ub buf_len, const char *fmt, ...);
ub t_stdio_snscanf(const s8 *buf_ptr, ub buf_len, const char *fmt, ...);
ub __t_stdio_strcpy__(s8 *dst, const s8 *src, ub max_length, s8 *file, ub line);
#define t_stdio_strcpy(dst, src, max_length) __t_stdio_strcpy__((s8 *)(dst), (const s8 *)(src), (ub)(max_length), (s8 *)__func__, (ub)__LINE__)
dave_bool __t_stdio_strcmp__(s8 *cmp1, s8 *cmp2, s8 *file, ub line);
#define t_stdio_strcmp(cmp1, cmp2) __t_stdio_strcmp__((s8 *)(cmp1), (s8 *)(cmp2), (s8 *)__func__, (ub)__LINE__)
s8 * t_stdio_strfind(s8 *str, s8 end_char, s8 *find_ptr, ub find_len);
s8 * t_stdio_tolowers(s8 *str);
s8 * t_stdio_touppers(s8 *str);
s8 * t_stdio_strstr(s8 *buf, s8 *sub);
s8 * t_stdio_remove_the_char_on_frist(s8 *data_ptr, s8 remove_char);

void t_stdio_print_hex(const char *msg, u8 *hex, ub hex_len);
void t_stdio_print_char(const char *msg, u8 *char_data, ub char_len);

#define dave_sprintf t_stdio_sprintf
#define dave_sscanf t_stdio_sscanf
#define dave_snprintf t_stdio_snprintf
#define dave_snscanf t_stdio_snscanf
#define dave_strcpy t_stdio_strcpy
#define dave_strcmp t_stdio_strcmp
#define dave_memcpy(a, b, c) t_stdio_memcpy((u8 *)(a), (u8 *)(b), (ub)(c))
#define dave_memmove(a, b, c) t_stdio_memmove((u8 *)(a), (u8 *)(b), (ub)(c))
#define dave_memcmp(a, b, c) t_stdio_memcmp((u8 *)(a), (u8 *)(b), (ub)(c))
#define dave_memset(a, b, c) t_stdio_memset((u8 *)(a), (u8)(b), (ub)(c))
#define dave_strlen(a) t_stdio_strlen((s8 *)(a))
#define dave_strfind t_stdio_strfind
#define dave_tolowers t_stdio_tolowers
#define dave_strstr t_stdio_strstr

#endif

