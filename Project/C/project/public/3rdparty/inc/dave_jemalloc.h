/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_JEMALLOC_H__
#define __DAVE_JEMALLOC_H__

void * dave_jemalloc(size_t size);
void dave_jefree(void *ptr);
size_t dave_jelen(void *ptr);

#endif

