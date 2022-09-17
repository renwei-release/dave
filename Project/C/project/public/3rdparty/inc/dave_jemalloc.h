/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_JEMALLOC_H__
#define __DAVE_JEMALLOC_H__
#include "3rdparty_macro.h"

extern void *malloc(size_t size);
extern void free(void *);
extern size_t malloc_usable_size(void *);

#define dave_jemalloc(size) malloc((size_t)size)
#define dave_jefree(ptr) free(ptr)
#define dave_jelen(ptr) malloc_usable_size(ptr)

#endif

