/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __MEM_EXTER_H__
#define __MEM_EXTER_H__
#include "base_define.h"

void exter_mem_init(void);

void exter_mem_exit(void);

void * __exter_malloc__(ub len, s8 *file, ub line);

dave_bool __exter_free__(void *ptr, s8 *file, ub line);

ub __exter_len__(void *ptr, s8 *file, ub line);

dave_bool __exter_memory__(void *ptr, s8 *file, ub line);

ub __exter_len__(void *ptr, s8 *file, ub line);

ub __exter_memory_info__(s8 *info, ub info_len, dave_bool base_flag);

#endif

