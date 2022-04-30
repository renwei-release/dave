/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __BASE_MEM_H__
#define __BASE_MEM_H__

void *__base_malloc__(ub len, dave_bool reset, u8 reset_data, s8 *file, ub line);
void *__base_calloc__(ub num, ub len, s8 *file, ub line);
dave_bool __base_free__(void *ptr, s8 *file, ub line);
MBUF *__base_mmalloc__(ub length, s8 *file, ub line);
sb __base_mheader__(MBUF *m, sb header_size_increment, s8 *file, ub line);
ub __base_mfree__(MBUF *m, s8 *file, ub line);
void __base_mref__(MBUF *m, s8 *file, ub line);
MBUF * __base_mchain__(MBUF *cur_point, MBUF *cat_point, s8 *file, ub line);
MBUF * __base_mdechain__(MBUF *m, s8 *file, ub line);
MBUF * __base_mclone__(MBUF *m, s8 *file, ub line);
void * base_mptr(MBUF *data);
ub base_mlnumber(MBUF *data);
void base_mem_init(void);
void base_mem_exit(void);
ub base_mem_info(s8 *info, ub info_len, dave_bool base_flag);

#define base_ralloc(a) __base_malloc__((ub)(a), dave_true, 0x00, (s8 *)__func__, (ub)__LINE__)
#define base_malloc(a) __base_malloc__((ub)(a), dave_false, 0x00, (s8 *)__func__, (ub)__LINE__)
#define base_falloc(a, b) __base_malloc__((ub)(a), dave_true, (u8)b, (s8 *)__func__, (ub)__LINE__)
#define base_calloc(a,b) __base_calloc__((ub)(a), (ub)(b), (s8 *)__func__, (ub)__LINE__)
#define base_free(a) __base_free__(a, (s8 *)__func__, (ub)__LINE__)
#define base_mmalloc(a) __base_mmalloc__((ub)(a), (s8 *)__func__, (ub)__LINE__)
#define base_mheader(a, b) __base_mheader__(a, (sb)b, (s8 *)__func__, (ub)__LINE__)
#define base_mfree(a) __base_mfree__(a, (s8 *)__func__, (ub)__LINE__)
#define base_mref(a) __base_mref__(a, (s8 *)__func__, (ub)__LINE__)
#define base_mchain(a, b) __base_mchain__(a, (b), (s8 *)__func__, (ub)__LINE__)
#define base_mdechain(a) __base_mdechain__(a, (s8 *)__func__, (ub)__LINE__)
#define base_mclone(a) __base_mclone__(a, (s8 *)__func__, (ub)__LINE__)

#define dave_ralloc base_ralloc
#define dave_malloc base_malloc
#define dave_falloc base_falloc
#define dave_calloc base_calloc
#define dave_free base_free
#define dave_mmalloc base_mmalloc
#define dave_mheader base_mheader
#define dave_mfree base_mfree
#define dave_mref base_mref
#define dave_mchain base_mchain
#define dave_mdechain base_mdechain
#define dave_mclone base_mclone
#define dave_mptr base_mptr
#define dave_mlnumber base_mlnumber

#define ms8ptr(mbuf_ptr) (s8 *)base_mptr(mbuf_ptr)

#endif

