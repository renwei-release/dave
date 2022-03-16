/*
 * ================================================================================
 * (c) Copyright 2016 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
 */

#ifndef __MEM_MBUF_H__
#define __MEM_MBUF_H__
#include "base_macro.h"
#include "base_define.h"

MBUF * __mbuf_mmalloc__(ub length, s8 *file, ub line);

sb __mbuf_mheader__(MBUF *m, sb header_size_increment, s8 *file, ub line);

ub __mbuf_mfree__(MBUF *m, s8 *file, ub line);

void __mbuf_mref__(MBUF *m, s8 *file, ub line);

MBUF * __mbuf_mchain__(MBUF *cur_point, MBUF *cat_point, s8 *file, ub line);

MBUF * __mbuf_mdechain__(MBUF *m, s8 *file, ub line);

MBUF * __mbuf_clone__(MBUF *me, s8 *file, ub line);

ub __mbuf_list_number__(MBUF *m);

#endif

