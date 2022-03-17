/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_JEMALLOC_H__
#define __DAVE_JEMALLOC_H__

void dave_jemalloc_init(void);

void dave_jemalloc_exit(void);

void * dave_jemalloc(ub length);

void dave_jefree(void *ptr);

ub dave_jelen(void *ptr);

#endif

