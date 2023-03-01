/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __MEM_MBUF_H__
#define __MEM_MBUF_H__
#include "base_macro.h"
#include "base_define.h"

MBUF * __mbuf_mmalloc__(ub length, s8 *file, ub line);

sb __mbuf_mheader__(MBUF *m, sb header_size_increment, s8 *file, ub line);

ub __mbuf_mfree__(MBUF *m, s8 *file, ub line);

ub __mbuf_mclean__(MBUF *m, s8 *file, ub line);

void __mbuf_mref__(MBUF *m, s8 *file, ub line);

MBUF * __mbuf_mchain__(MBUF *cur_point, MBUF *cat_point, s8 *file, ub line);

MBUF * __mbuf_mdechain__(MBUF *m, s8 *file, ub line);

MBUF * __mbuf_clone__(MBUF *me, s8 *file, ub line);

ub __mbuf_list_number__(MBUF *m);

#endif

